///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/textentry.cpp
// Purpose:     wxTextEntry implementation for wxMSW
// Author:      Vadim Zeitlin
// Created:     2007-09-26
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/event.h"
#include "wx/textctrl.h"    // Only for wxTE_PROCESS_XXX constants

#if wxUSE_TEXTCTRL || wxUSE_COMBOBOX

#include "wx/textentry.h"
#include "wx/textcompleter.h"
#include "wx/dynlib.h"

#include "wx/msw/private/winstyle.h"
#include "wx/msw/private/cotaskmemptr.h"

#if wxUSE_UXTHEME
    #include "wx/msw/uxtheme.h"
#endif

#include <boost/nowide/convert.hpp>

import WX.WinDef;

#define GetEditHwnd() ((WXHWND)(GetEditHWND()))

// ----------------------------------------------------------------------------
// Classes used by auto-completion implementation.
// ----------------------------------------------------------------------------

#if wxUSE_OLE
    #define HAS_AUTOCOMPLETE
#endif

#ifdef HAS_AUTOCOMPLETE

#include "wx/msw/ole/oleutils.h"
#include <shldisp.h>

#if defined(__MINGW32__) || defined(__CYGWIN__)
    // needed for IID_IAutoComplete, IID_IAutoComplete2 and ACO_AUTOSUGGEST
    #include <shlguid.h>

    #ifndef ACO_AUTOAPPEND
        #define ACO_AUTOAPPEND 0x02
    #endif
#endif

#ifndef SHACF_FILESYS_ONLY
    #define SHACF_FILESYS_ONLY 0x00000010
#endif

#ifndef SHACF_FILESYS_DIRS
    #define SHACF_FILESYS_DIRS 0x00000020
#endif

#ifdef __CYGWIN__
// Normally this interface and its IID are defined in shobjidl.h header file
// included in the platform SDK but MinGW and Cygwin don't have it so redefine
// the interface ourselves and, as long as we do it all, do it for all
// compilers to ensure we have the same behaviour for all of them and to avoid
// the need to check for concrete compilers and maybe even their versions.
class IAutoCompleteDropDown : public IUnknown
{
public:
    virtual HRESULT wxSTDCALL GetDropDownStatus(WXDWORD *, LPWSTR *) = 0;
    virtual HRESULT wxSTDCALL ResetEnumerator() = 0;
};

#endif // __CYGWIN__

namespace
{

DEFINE_GUID(wxIID_IAutoCompleteDropDown,
    0x3cd141f4, 0x3c6a, 0x11d2, 0xbc, 0xaa, 0x00, 0xc0, 0x4f, 0xd9, 0x29, 0xdb);

DEFINE_GUID(wxCLSID_AutoComplete,
    0x00bb2763, 0x6a77, 0x11d0, 0xa5, 0x35, 0x00, 0xc0, 0x4f, 0xd7, 0xd0, 0x62);

#ifndef ACDD_VISIBLE
constexpr unsigned int ACDD_VISIBLE = 0x0001;
#endif

// Small helper class which can be used to ensure thread safety even when
// wxUSE_THREADS==0 (and hence wxCriticalSection does nothing).
class CSLock
{
public:
    explicit CSLock(CRITICAL_SECTION& cs) : m_cs(&cs)
    {
        ::EnterCriticalSection(m_cs);
    }

    ~CSLock()
    {
        ::LeaveCriticalSection(m_cs);
    }

    CSLock(const CSLock&) = delete;
	CSLock& operator=(const CSLock&) = delete;

private:
    CRITICAL_SECTION * const m_cs;
};

} // anonymity namespace

// Implementation of string enumerator used by wxTextAutoCompleteData. This
// class simply forwards to wxTextCompleter associated with it.
//
// Notice that Next() method of this class is called by IAutoComplete
// background thread and so we must care about thread safety here.
class wxIEnumString : public IEnumString
{
public:
    wxIEnumString()
    {   
        ::InitializeCriticalSection(&m_csCompleter);
        ::InitializeCriticalSection(&m_csRestart);
    }

    wxIEnumString(const wxIEnumString&) = delete;
	wxIEnumString& operator=(const wxIEnumString&) = delete;    

    void ChangeCompleter(wxTextCompleter *completer)
    {
        // Indicate to Next() that it should bail out as soon as possible.
        {
            CSLock lock(m_csRestart);

            m_restart = TRUE;
        }

        // Now try to enter this critical section to ensure that Next() doesn't
        // use the old pointer any more before changing it (this is vital as
        // the old pointer will be destroyed after we return).
        CSLock lock(m_csCompleter);

        m_completer = completer;
    }

    bool UpdatePrefix(const std::string& prefix)
    {
        CSLock lock(m_csRestart);

        if ( prefix == m_prefix )
            return false;

        // We simply store the prefix here and will really update during the
        // next call to our Next() method as we want to call Start() from the
        // worker thread to prevent the main UI thread from blocking while the
        // completions are generated.
        m_prefix = prefix;
        m_restart = TRUE;

        return true;
    }

    HRESULT STDMETHODCALLTYPE Next(ULONG celt,
                                   LPOLESTR *rgelt,
                                   ULONG *pceltFetched) override
    {
        if ( !rgelt || (!pceltFetched && celt > 1) )
            return E_POINTER;

        ULONG pceltFetchedDummy;
        if ( !pceltFetched )
            pceltFetched = &pceltFetchedDummy;

        *pceltFetched = 0;

        CSLock lock(m_csCompleter);

        if ( !RestartIfNeeded() )
            return S_FALSE;

        while ( celt-- )
        {
            // Stop iterating if we need to update completions anyhow.
            if ( m_restart )
                return S_FALSE;

            const std::string s = m_completer->GetNext();
            if ( s.empty() )
                return S_FALSE;

            boost::nowide::wstackstring stackBuf{s.c_str()};
            const size_t size = (stackBuf.buffer_size + 1) * sizeof(wchar_t);
            wxCoTaskMemPtr<wchar_t> olestr(size);

            if ( !olestr )
                return E_OUTOFMEMORY;

            std::memcpy(olestr, stackBuf.get(), size);
            *rgelt++ = olestr.release();

            ++(*pceltFetched);
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override
    {
        if ( !celt )
            return E_INVALIDARG;

        CSLock lock(m_csCompleter);

        if ( !RestartIfNeeded() )
            return S_FALSE;

        while ( celt-- )
        {
            if ( m_restart )
                return S_FALSE;

            if ( m_completer->GetNext().empty() )
                return S_FALSE;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Reset() override
    {
        CSLock lock(m_csRestart);

        m_restart = TRUE;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(IEnumString **ppEnum) override
    {
        if ( !ppEnum )
            return E_POINTER;

        CSLock lock(m_csCompleter);

        wxIEnumString * const e = new wxIEnumString;
        e->AddRef();

        e->ChangeCompleter(m_completer);

        *ppEnum = e;

        return S_OK;
    }

    DECLARE_IUNKNOWN_METHODS;

private:
    // dtor doesn't have to be virtual as we're only ever deleted from our own
    // Release() and are not meant to be derived form anyhow, but making it
    // virtual silences gcc warnings; making it private makes it impossible to
    // (mistakenly) delete us directly instead of calling Release()
    virtual ~wxIEnumString()
    {
        ::DeleteCriticalSection(&m_csRestart);
        ::DeleteCriticalSection(&m_csCompleter);
    }

    // Restart completions generation if needed. Should be only called from
    // inside m_csCompleter.
    //
    // If false is returned, it means that there are no completions and that
    // wxTextCompleter::GetNext() shouldn't be called at all.
    bool RestartIfNeeded()
    {
        bool rc = true;
        for ( ;; )
        {
            std::string prefix;
            LONG restart;
            {
                CSLock lock(m_csRestart);

                prefix = m_prefix;
                restart = m_restart;

                m_restart = FALSE;
            } // Release m_csRestart before calling Start() to avoid blocking
              // the main thread in UpdatePrefix() during its execution.

            if ( !restart )
                break;

            rc = m_completer->Start(prefix);
        }

        return rc;
    }


    // Critical section protecting m_completer itself. It must be entered when
    // using the pointer to ensure that we don't continue using a dangling one
    // after it is destroyed.
    CRITICAL_SECTION m_csCompleter;

    // The completer we delegate to for the completions generation. It is never
    // NULL after the initial ChangeCompleter() call.
    wxTextCompleter *m_completer{nullptr};


    // Critical section m_prefix and m_restart. It should be only entered for
    // short periods of time, i.e. we shouldn't call any wxTextCompleter
    // methods from inside, to prevent the main thread from blocking on it in
    // UpdatePrefix().
    CRITICAL_SECTION m_csRestart;

    // If m_restart is true, we need to call wxTextCompleter::Start() with the
    // given prefix to restart generating the completions.
    std::string m_prefix;

    // Notice that we use LONG and not bool here to ensure that reading this
    // value is atomic (32 bit reads are atomic operations under all Windows
    // versions but reading bool isn't necessarily).
    LONG m_restart{FALSE};
};

BEGIN_IID_TABLE(wxIEnumString)
    ADD_IID(Unknown)
    ADD_IID(EnumString)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIEnumString)


// This class gathers the all auto-complete-related stuff we use. It is
// allocated on demand by wxTextEntry when AutoComplete() is called.
class wxTextAutoCompleteData
{
public:
    // The constructor associates us with the given text entry.
    explicit wxTextAutoCompleteData(wxTextEntry *entry)
        : m_entry(entry),
          m_win(entry->GetEditableWindow())
    {
        m_autoComplete = nullptr;
        m_autoCompleteDropDown = nullptr;
        m_enumStrings = nullptr;

        m_fixedCompleter = nullptr;
        m_customCompleter = nullptr;

        m_connectedCharEvent = false;

        // Create an object exposing IAutoComplete interface which we'll later
        // use to get IAutoComplete2 as the latter can't be created directly,
        // apparently.
        HRESULT hr = CoCreateInstance
                     (
                        wxCLSID_AutoComplete,
                        nullptr,
                        CLSCTX_INPROC_SERVER,
                        IID_IAutoComplete,
                        reinterpret_cast<void **>(&m_autoComplete)
                     );
        if ( FAILED(hr) )
        {
            wxLogApiError("CoCreateInstance(CLSID_AutoComplete)", hr);
            return;
        }

        // Create a string enumerator and initialize the completer with it.
        m_enumStrings = new wxIEnumString;
        m_enumStrings->AddRef();
        hr = m_autoComplete->Init(m_entry->GetEditHWND(), m_enumStrings,
                                  nullptr, nullptr);
        if ( FAILED(hr) )
        {
            wxLogApiError("IAutoComplete::Init", hr);

            m_enumStrings->Release();
            m_enumStrings = nullptr;

            return;
        }

        // As explained in DoRefresh(), we need to call IAutoCompleteDropDown::
        // ResetEnumerator() if we want to be able to change the completions on
        // the fly. In principle we could live without it, i.e. return true
        // from IsOk() even if this QueryInterface() fails, but it doesn't look
        // like this is ever going to have in practice anyhow as the shell-
        // provided IAutoComplete always implements IAutoCompleteDropDown too.
        hr = m_autoComplete->QueryInterface
                             (
                               wxIID_IAutoCompleteDropDown,
                               reinterpret_cast<void **>(&m_autoCompleteDropDown)
                             );
        if ( FAILED(hr) )
        {
            wxLogApiError("IAutoComplete::QI(IAutoCompleteDropDown)", hr);
            return;
        }

        // Finally set the completion options using IAutoComplete2.
        IAutoComplete2 *pAutoComplete2 = nullptr;
        hr = m_autoComplete->QueryInterface
                             (
                               IID_IAutoComplete2,
                               reinterpret_cast<void **>(&pAutoComplete2)
                             );
        if ( SUCCEEDED(hr) )
        {
            pAutoComplete2->SetOptions(ACO_AUTOSUGGEST |
                                       ACO_AUTOAPPEND |
                                       ACO_UPDOWNKEYDROPSLIST);
            pAutoComplete2->Release();
        }

        m_win->Bind(wxEVT_CHAR_HOOK, &wxTextAutoCompleteData::OnCharHook, this);
    }

    ~wxTextAutoCompleteData()
    {
        delete m_customCompleter;
        delete m_fixedCompleter;

        if ( m_enumStrings )
            m_enumStrings->Release();
        if ( m_autoCompleteDropDown )
            m_autoCompleteDropDown->Release();
        if ( m_autoComplete )
            m_autoComplete->Release();
    }

    wxTextAutoCompleteData(const wxTextAutoCompleteData&) = delete;
	wxTextAutoCompleteData& operator=(const wxTextAutoCompleteData&) = delete;

    // Must be called after creating this object to verify if initializing it
    // succeeded.
    bool IsOk() const
    {
        return m_autoComplete && m_autoCompleteDropDown && m_enumStrings;
    }

    void ChangeStrings(const std::vector<std::string>& strings)
    {
        if ( !m_fixedCompleter )
            m_fixedCompleter = new wxTextCompleterFixed;

        m_fixedCompleter->SetCompletions(strings);

        m_enumStrings->ChangeCompleter(m_fixedCompleter);

        DoRefresh();
    }

    // Takes ownership of the pointer if it is non-NULL.
    bool ChangeCustomCompleter(wxTextCompleter *completer)
    {
        // Ensure that the old completer is not used any more before deleting
        // it.
        m_enumStrings->ChangeCompleter(completer);

        delete m_customCompleter;
        m_customCompleter = completer;

        if ( m_customCompleter )
        {
            // We postpone connecting to this event until we really need to do
            // it (however we don't disconnect from it when we don't need it
            // any more because we don't have wxUNBIND_OR_DISCONNECT_HACK...).
            if ( !m_connectedCharEvent )
            {
                m_connectedCharEvent = true;

                // Use the special wxEVT_AFTER_CHAR and not the usual
                // wxEVT_CHAR here because we need to have the updated value of
                // the text control in this handler in order to provide
                // completions for the correct prefix and unfortunately we
                // don't have any way to let DefWindowProc() run from our
                // wxEVT_CHAR handler (as we must also let the other handlers
                // defined at wx level run first).
                //
                // Notice that we can't use wxEVT_TEXT here
                // neither as, due to our use of ACO_AUTOAPPEND, we get
                // EN_CHANGE notifications from the control every time
                // IAutoComplete auto-appends something to it.
                m_win->Bind(wxEVT_AFTER_CHAR,
                            &wxTextAutoCompleteData::OnAfterChar, this);
            }

            UpdateStringsFromCustomCompleter();
        }

        return true;
    }

    void DisableCompletion()
    {
        // We currently simply reset the list of possible strings as this seems
        // to effectively disable auto-completion just fine. We could (and
        // probably should) use IAutoComplete::Enable(FALSE) for this too but
        // then we'd need to call Enable(TRUE) to turn it on back again later.
        ChangeStrings({});
    }

private:
    // Must be called after changing the values to be returned by wxIEnumString
    // to really make the changes stick.
    void DoRefresh()
    {
        m_enumStrings->Reset();

        // This is completely and utterly not documented and in fact the
        // current MSDN seems to try to discourage us from using it by saying
        // that "there is no reason to use this method unless the drop-down
        // list is currently visible" but actually we absolutely must call it
        // to force the auto-completer (and not just its drop-down!) to refresh
        // the list of completions which could have changed now. Without this
        // call the new choices returned by GetCompletions() that hadn't been
        // returned by it before are simply silently ignored.
        m_autoCompleteDropDown->ResetEnumerator();
    }

    // Update the strings returned by our string enumerator to correspond to
    // the currently valid choices according to the custom completer.
    void UpdateStringsFromCustomCompleter()
    {
        // As we use ACO_AUTOAPPEND, the selected part of the text is usually
        // the one appended by us so don't consider it as part of the
        // user-entered prefix.
        long from, to;
        m_entry->GetSelection(&from, &to);

        if ( to == from )
            from = m_entry->GetLastPosition(); // Take all if no selection.

        const std::string prefix = m_entry->GetRange(0, from);

        if ( m_enumStrings->UpdatePrefix(prefix) )
            DoRefresh();
    }

    void OnAfterChar(wxKeyEvent& event)
    {
        if ( m_customCompleter )
        {
            UpdateStringsFromCustomCompleter();
        }

        event.Skip();
    }

    void OnCharHook(wxKeyEvent& event)
    {
        // We need to override the default handling of some keys here.
        bool specialKey = false;
        switch ( event.GetKeyCode() )
        {
            case WXK_RETURN:
            case WXK_NUMPAD_ENTER:
                if ( m_win->HasFlag(wxTE_PROCESS_ENTER) )
                    specialKey = true;
                break;

            case WXK_TAB:
                if ( m_win->HasFlag(wxTE_PROCESS_TAB) )
                    specialKey = true;
                break;

            case WXK_ESCAPE:
                specialKey = true;
                break;
        }

        if ( specialKey )
        {
            // Check if the drop down is currently open.
            WXDWORD dwFlags = 0;
            if ( SUCCEEDED(m_autoCompleteDropDown->GetDropDownStatus(&dwFlags,
                                                                     nullptr))
                    && dwFlags == ACDD_VISIBLE )
            {
                if ( event.GetKeyCode() == WXK_ESCAPE )
                {
                    // We need to dismiss the drop-down manually as Escape
                    // could be eaten by something else (e.g. EVT_CHAR_HOOK in
                    // the dialog that this control is found in) otherwise.
                    ::SendMessageW(GetHwndOf(m_win), WM_KEYDOWN, WXK_ESCAPE, 0);

                    // Do not skip the event in this case, we've already handled it.
                    return;
                }
            }
            else // Drop down is not open.
            {
                // In this case we need to handle Return and Tab as both of
                // them are simply eaten by the auto completer and never reach
                // us at all otherwise.
                if ( event.GetKeyCode() != WXK_ESCAPE )
                {
                    m_entry->MSWProcessSpecialKey(event);
                    return;
                }
            }
        }

        event.Skip();
    }

    // The text entry we're associated with.
    wxTextEntry * const m_entry;

    // The window of this text entry.
    wxWindow * const m_win;

    // The auto-completer object itself.
    IAutoComplete *m_autoComplete;

    // Its IAutoCompleteDropDown interface needed for ResetEnumerator() call.
    IAutoCompleteDropDown *m_autoCompleteDropDown;

    // Enumerator for strings currently used for auto-completion.
    wxIEnumString *m_enumStrings;

    // Fixed string completer or NULL if none.
    wxTextCompleterFixed *m_fixedCompleter;

    // Custom completer or NULL if none.
    wxTextCompleter *m_customCompleter;

    // Initially false, set to true after connecting OnTextChanged() handler.
    bool m_connectedCharEvent;
};

// Special pointer value which indicates that we're using SHAutoComplete().
static wxTextAutoCompleteData* const wxDUMMY_SHAUTOCOMPLETE_DATA =
    reinterpret_cast<wxTextAutoCompleteData*>(-1);

#endif // HAS_AUTOCOMPLETE

// ============================================================================
// wxTextEntry implementation
// ============================================================================

// ----------------------------------------------------------------------------
// initialization and destruction
// ----------------------------------------------------------------------------

wxTextEntry::wxTextEntry()
{
#ifdef HAS_AUTOCOMPLETE
    m_autoCompleteData = nullptr;
#endif // HAS_AUTOCOMPLETE
}

wxTextEntry::~wxTextEntry()
{
#ifdef HAS_AUTOCOMPLETE
    if ( MSWHasAutoCompleteData() )
        delete m_autoCompleteData;
#endif // HAS_AUTOCOMPLETE
}

// ----------------------------------------------------------------------------
// operations on text
// ----------------------------------------------------------------------------

void wxTextEntry::WriteText(std::string_view text)
{
    ::SendMessageW(GetEditHwnd(), EM_REPLACESEL, 0, reinterpret_cast<WXLPARAM>(boost::nowide::widen(text).c_str()));
}

std::string wxTextEntry::DoGetValue() const
{
    return wxGetWindowText(GetEditHWND());
}

void wxTextEntry::Remove(long from, long to)
{
    DoSetSelection(from, to, SetSel_NoScroll);
    WriteText("");
}

// ----------------------------------------------------------------------------
// clipboard operations
// ----------------------------------------------------------------------------

void wxTextEntry::Copy()
{
    ::SendMessageW(GetEditHwnd(), WM_COPY, 0, 0);
}

void wxTextEntry::Cut()
{
    ::SendMessageW(GetEditHwnd(), WM_CUT, 0, 0);
}

void wxTextEntry::Paste()
{
    ::SendMessageW(GetEditHwnd(), WM_PASTE, 0, 0);
}

// ----------------------------------------------------------------------------
// undo/redo
// ----------------------------------------------------------------------------

void wxTextEntry::Undo()
{
    ::SendMessageW(GetEditHwnd(), EM_UNDO, 0, 0);
}

void wxTextEntry::Redo()
{
    // same as Undo, since Undo undoes the undo
    Undo();
    return;
}

bool wxTextEntry::CanUndo() const
{
    return ::SendMessageW(GetEditHwnd(), EM_CANUNDO, 0, 0) != 0;
}

bool wxTextEntry::CanRedo() const
{
    // see comment in Redo()
    return CanUndo();
}

// ----------------------------------------------------------------------------
// insertion point and selection
// ----------------------------------------------------------------------------

void wxTextEntry::SetInsertionPoint(long pos)
{
    // calling DoSetSelection(-1, -1) would select everything which is not what
    // we want here
    if ( pos == -1 )
        pos = GetLastPosition();

    // be careful to call DoSetSelection() which is overridden in wxTextCtrl
    // and not just SetSelection() here
    DoSetSelection(pos, pos);
}

long wxTextEntry::GetInsertionPoint() const
{
    long from;
    GetSelection(&from, nullptr);
    return from;
}

long wxTextEntry::GetLastPosition() const
{
    return ::SendMessageW(GetEditHwnd(), EM_LINELENGTH, 0, 0);
}

void wxTextEntry::DoSetSelection(long from, long to, [[maybe_unused]] unsigned int flags)
{
    // if from and to are both -1, it means (in wxWidgets) that all text should
    // be selected, translate this into Windows convention
    if ( (from == -1) && (to == -1) )
    {
        from = 0;
    }

    ::SendMessageW(GetEditHwnd(), EM_SETSEL, from, to);
}

void wxTextEntry::GetSelection(long *from, long *to) const
{
    WXDWORD dwStart, dwEnd;
    ::SendMessageW(GetEditHwnd(), EM_GETSEL, (WXWPARAM)&dwStart, (WXLPARAM)&dwEnd);

    if ( from )
        *from = dwStart;
    if ( to )
        *to = dwEnd;
}

// ----------------------------------------------------------------------------
// auto-completion
// ----------------------------------------------------------------------------

#if wxUSE_OLE

#ifdef HAS_AUTOCOMPLETE

#if wxUSE_DYNLIB_CLASS

bool wxTextEntry::DoAutoCompleteFileNames(unsigned int flags)
{
    WXDWORD dwFlags = 0;
    if ( flags & wxFILE )
        dwFlags |= SHACF_FILESYS_ONLY;
    else if ( flags & wxDIR )
        dwFlags |= SHACF_FILESYS_DIRS;
    else
    {
        wxFAIL_MSG("No flags for file name auto completion?");
        return false;
    }

    HRESULT hr = ::SHAutoComplete(GetEditHwnd(), dwFlags);
    if ( FAILED(hr) )
    {
        wxLogApiError("SHAutoComplete()", hr);

        return false;
    }

    // Disable the other kinds of completion now that we use the built-in file
    // names completion.
    if ( MSWHasAutoCompleteData() )
        delete m_autoCompleteData;

    // Set it to the special value indicating that we're using SHAutoComplete().
    m_autoCompleteData = wxDUMMY_SHAUTOCOMPLETE_DATA;

    return true;
}

#endif // wxUSE_DYNLIB_CLASS

void wxTextEntry::MSWProcessSpecialKey([[maybe_unused]] wxKeyEvent& event)
{
    wxFAIL_MSG("Must be overridden if can be called");
}

bool wxTextEntry::MSWUsesStandardAutoComplete() const
{
    return m_autoCompleteData == wxDUMMY_SHAUTOCOMPLETE_DATA;
}

bool wxTextEntry::MSWHasAutoCompleteData() const
{
    // We use special wxDUMMY_SHAUTOCOMPLETE_DATA for the pointer to indicate
    // that we're using SHAutoComplete(), so we need to check for it too, and
    // not just whether the pointer is non-NULL.
    return m_autoCompleteData != nullptr
            && m_autoCompleteData != wxDUMMY_SHAUTOCOMPLETE_DATA;
}

bool wxTextEntry::MSWEnsureHasAutoCompleteData()
{
    if ( !MSWHasAutoCompleteData() )
    {
        wxTextAutoCompleteData * const ac = new wxTextAutoCompleteData(this);
        if ( !ac->IsOk() )
        {
            delete ac;
            return false;
        }

        m_autoCompleteData = ac;
    }

    return true;
}

bool wxTextEntry::DoAutoCompleteStrings(const std::vector<std::string>& choices)
{
    if ( !MSWEnsureHasAutoCompleteData() )
        return false;

    m_autoCompleteData->ChangeStrings(choices);

    return true;
}

bool wxTextEntry::DoAutoCompleteCustom(wxTextCompleter *completer)
{
    // First deal with the case when we just want to disable auto-completion.
    if ( !completer )
    {
        if ( MSWHasAutoCompleteData() )
            m_autoCompleteData->DisableCompletion();
        //else: Nothing to do, we hadn't used auto-completion even before.
    }
    else // Have a valid completer.
    {
        if ( !MSWEnsureHasAutoCompleteData() )
        {
            // Delete the custom completer for consistency with the case when
            // we succeed to avoid memory leaks in user code.
            delete completer;
            return false;
        }

        // This gives ownership of the custom completer to m_autoCompleteData.
        if ( !m_autoCompleteData->ChangeCustomCompleter(completer) )
            return false;
    }

    return true;
}

#else // !HAS_AUTOCOMPLETE

// We still need to define stubs as we declared these overrides in the header.

bool wxTextEntry::DoAutoCompleteFileNames(unsigned int flags)
{
    return wxTextEntryBase::DoAutoCompleteFileNames(flags);
}

bool wxTextEntry::DoAutoCompleteStrings(const std::vector<std::string>& choices)
{
    return wxTextEntryBase::DoAutoCompleteStrings(choices);
}

bool wxTextEntry::DoAutoCompleteCustom(wxTextCompleter *completer)
{
    return wxTextEntryBase::DoAutoCompleteCustom(completer);
}

#endif // HAS_AUTOCOMPLETE/!HAS_AUTOCOMPLETE

#endif // wxUSE_OLE

// ----------------------------------------------------------------------------
// editable state
// ----------------------------------------------------------------------------

bool wxTextEntry::IsEditable() const
{
    return !(::GetWindowLongPtrW(GetEditHwnd(), GWL_STYLE) & ES_READONLY);
}

void wxTextEntry::SetEditable(bool editable)
{
    ::SendMessageW(GetEditHwnd(), EM_SETREADONLY, !editable, 0);
}

// ----------------------------------------------------------------------------
// input restrictions
// ----------------------------------------------------------------------------

void wxTextEntry::SetMaxLength(unsigned long len)
{
    if ( len >= 0xffff )
    {
        // this will set it to a platform-dependent maximum (much more
        // than 64Kb under NT)
        len = 0;
    }

    ::SendMessageW(GetEditHwnd(), EM_LIMITTEXT, len, 0);
}

void wxTextEntry::ForceUpper()
{
    ConvertToUpperCase();

    wxMSWWinStyleUpdater(GetEditHwnd()).TurnOn(ES_UPPERCASE);
}

// ----------------------------------------------------------------------------
// hints
// ----------------------------------------------------------------------------

#if wxUSE_UXTHEME

#ifndef EM_SETCUEBANNER
    #define EM_SETCUEBANNER 0x1501
    #define EM_GETCUEBANNER 0x1502
#endif

bool wxTextEntry::SetHint(const std::string& hint)
{
    if ( wxGetWinVersion() >= wxWinVersion_Vista && wxUxThemeIsActive() )
    {
        // notice that this message always works with Unicode strings
        //
        // we always use TRUE for wParam to show the hint even when the window
        // has focus, otherwise there would be no way to show the hint for the
        // initially focused window
        if ( ::SendMessageW(GetEditHwnd(), EM_SETCUEBANNER,
                             TRUE, (WXLPARAM)boost::nowide::widen(hint).c_str()) )
            return true;
    }

    return wxTextEntryBase::SetHint(hint);
}

std::string wxTextEntry::GetHint() const
{
    if ( wxUxThemeIsActive() )
    {
        std::wstring buf;

        buf.resize(256);

        if ( ::SendMessageW(GetEditHwnd(), EM_GETCUEBANNER,
                            (WXWPARAM)&buf[0], buf.size()) )
            return boost::nowide::narrow(buf);
    }

    return wxTextEntryBase::GetHint();
}


#endif // wxUSE_UXTHEME

// ----------------------------------------------------------------------------
// margins support
// ----------------------------------------------------------------------------

bool wxTextEntry::DoSetMargins(const wxPoint& margins)
{
    bool res = true;

    if ( margins.x != -1 )
    {
        // Set both horizontal margins to the given value, we don't distinguish
        // between left and right margin at wx API level and it seems to be
        // better to change both of them than only left one.
        ::SendMessageW(GetEditHwnd(), EM_SETMARGINS,
                      EC_LEFTMARGIN | EC_RIGHTMARGIN,
                      MAKELONG(margins.x, margins.x));
    }

    if ( margins.y != -1 )
    {
        res = false;
    }

    return res;
}

wxPoint wxTextEntry::DoGetMargins() const
{
    LRESULT lResult = ::SendMessageW(GetEditHwnd(), EM_GETMARGINS, 0, 0);
    
    return {LOWORD(lResult), -1};
}

// ----------------------------------------------------------------------------
// input handling
// ----------------------------------------------------------------------------

bool wxTextEntry::ClickDefaultButtonIfPossible()
{
    return !wxIsAnyModifierDown() &&
                wxWindow::MSWClickButtonIfPossible(
                    wxWindow::MSWGetDefaultButtonFor(GetEditableWindow()));
}

#endif // wxUSE_TEXTCTRL || wxUSE_COMBOBOX
