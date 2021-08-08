///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/textentrycmn.cpp
// Purpose:     wxTextEntryBase implementation
// Author:      Vadim Zeitlin
// Created:     2007-09-26
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_TEXTCTRL || wxUSE_COMBOBOX

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/dataobj.h"
    #include "wx/textctrl.h"            // Only needed for wxTE_PASSWORD.
#endif //WX_PRECOMP

#include "wx/textentry.h"
#include "wx/textcompleter.h"
#include "wx/clipbrd.h"

// ----------------------------------------------------------------------------
// wxTextEntryHintData
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxTextEntryHintData : public wxEvtHandler
{
public:
    wxTextEntryHintData(wxTextEntryBase *entry, wxWindow *win)
        : m_entry(entry),
          m_win(win),
          m_text(m_entry->GetValue())
    {
        // We push ourselves as the event handler because this allows us to
        // handle events before the user-defined handlers and notably process
        // wxEVT_TEXT even if the user code already handles it, which is vital
        // as if we don't get this event, we would always set the control text
        // to the hint when losing focus, instead of preserving the text
        // entered by user. Of course, the same problem could still happen if
        // the user code pushed their own event handler before this one and
        // didn't skip wxEVT_TEXT in it, but there doesn't seem anything we can
        // do about this anyhow and this at least takes care of the much more
        // common case.
        m_win->PushEventHandler(this);

        Bind(wxEVT_SET_FOCUS, &wxTextEntryHintData::OnSetFocus, this);
        Bind(wxEVT_KILL_FOCUS, &wxTextEntryHintData::OnKillFocus, this);
        Bind(wxEVT_TEXT, &wxTextEntryHintData::OnTextChanged, this);
    }

    ~wxTextEntryHintData() override
    {
        m_win->PopEventHandler();
    }

    wxTextEntryHintData(const wxTextEntryHintData&) = delete;
	wxTextEntryHintData& operator=(const wxTextEntryHintData&) = delete;

    // Get the real text of the control such as it was before we replaced it
    // with the hint.
    const std::string& GetText() const { return m_text; }

    // Set the hint to show, shouldn't be empty normally.
    //
    // This should be called after creating a new wxTextEntryHintData object
    // and may be called more times in the future.
    void SetHintString(const std::string& hint)
    {
        m_hint = hint;

        if ( !m_win->HasFocus() )
            ShowHintIfAppropriate();
        //else: The new hint will be shown later when we lose focus.
    }

    const std::string& GetHintString() const { return m_hint; }

    // This is called whenever the text control contents changes.
    //
    // We call it ourselves when this change generates an event but it's also
    // necessary to call it explicitly from wxTextEntry::ChangeValue() as it,
    // by design, does not generate any events.
    void HandleTextUpdate(const std::string& text)
    {
        m_text = text;

        // If we're called because of a call to Set or ChangeValue(), the
        // control may still have the hint text colour, reset it in this case.
        RestoreTextColourIfNecessary();
    }

private:
    // Show the hint in the window if we should do it, i.e. if the window
    // doesn't have any text of its own.
    void ShowHintIfAppropriate()
    {
        // Never overwrite existing window text.
        if ( !m_text.empty() )
            return;

        // Save the old text colour and set a more inconspicuous one for the
        // hint.
        if (!m_colFg.IsOk())
        {
            m_colFg = m_win->GetForegroundColour();
            m_win->SetForegroundColour(*wxLIGHT_GREY);
        }

        m_entry->DoSetValue(m_hint, wxTextEntryBase::SetValue_NoEvent);
    }

    // Restore the original text colour if we had changed it to show the hint
    // and not restored it yet.
    void RestoreTextColourIfNecessary()
    {
        if ( m_colFg.IsOk() )
        {
            m_win->SetForegroundColour(m_colFg);
            m_colFg = wxColour();
        }
    }

    void OnSetFocus(wxFocusEvent& event)
    {
        // If we had been showing the hint before, remove it now and restore
        // the normal colour.
        if ( m_text.empty() )
        {
            RestoreTextColourIfNecessary();

            m_entry->DoSetValue("", wxTextEntryBase::SetValue_NoEvent);
        }

        event.Skip();
    }

    void OnKillFocus(wxFocusEvent& event)
    {
        // Restore the hint if the user didn't enter anything.
        ShowHintIfAppropriate();

        event.Skip();
    }

    void OnTextChanged(wxCommandEvent& event)
    {
        // Update the stored window text.
        //
        // Notice that we can't use GetValue() nor wxCommandEvent::GetString()
        // which uses it internally because this would just forward back to us
        // so go directly to the private method which returns the real control
        // contents.
        HandleTextUpdate(m_entry->DoGetValue());

        event.Skip();
    }


    // the text control we're associated with (as its interface and its window)
    wxTextEntryBase * const m_entry;
    wxWindow * const m_win;

    // the original foreground colour of m_win before we changed it
    wxColour m_colFg;

    // The hint passed to wxTextEntry::SetHint(), never empty.
    std::string m_hint;

    // The real text of the window.
    std::string m_text;

};

// ============================================================================
// wxTextEntryBase implementation
// ============================================================================

wxTextEntryBase::~wxTextEntryBase()
{
    delete m_hintData;
}

// ----------------------------------------------------------------------------
// text accessors
// ----------------------------------------------------------------------------

std::string wxTextEntryBase::GetValue() const
{
    return m_hintData ? m_hintData->GetText() : DoGetValue();
}

std::string wxTextEntryBase::GetRange(long from, long to) const
{
    std::string sel;
    std::string value = GetValue();

    if ( from < to && (long)value.length() >= to )
    {
        sel = value.substr(from, to - from);
    }

    return sel;
}

// ----------------------------------------------------------------------------
// text operations
// ----------------------------------------------------------------------------

void wxTextEntryBase::ChangeValue(const std::string& value)
{
    DoSetValue(value, SetValue_NoEvent);

    // As we didn't generate any events for wxTextEntryHintData to catch,
    // notify it explicitly about our changed contents.
    if ( m_hintData )
        m_hintData->HandleTextUpdate(value);
}

void wxTextEntryBase::AppendText(const std::string& text)
{
    SetInsertionPointEnd();
    WriteText(text);
}

void wxTextEntryBase::DoSetValue(const std::string& value, int flags)
{
    if ( value != DoGetValue() )
    {
        EventsSuppressor noeventsIf(this, !(flags & SetValue_SendEvent));

        SelectAll();
        WriteText(value);

        SetInsertionPoint(0);
    }
    else // Same value, no need to do anything.
    {
        // Except that we still need to generate the event for consistency with
        // the normal case when the text does change.
        if ( flags & SetValue_SendEvent )
            SendTextUpdatedEvent(GetEditableWindow());
    }
}

void wxTextEntryBase::Replace(long from, long to, const std::string& value)
{
    {
        EventsSuppressor noevents(this);
        Remove(from, to);
    }

    SetInsertionPoint(from);
    WriteText(value);
}

// ----------------------------------------------------------------------------
// selection
// ----------------------------------------------------------------------------

bool wxTextEntryBase::HasSelection() const
{
    // TODO: Return pair
    long from, to;
    GetSelection(&from, &to);

    return from < to;
}

void wxTextEntryBase::RemoveSelection()
{
    long from, to;
    GetSelection(& from, & to);
    if (from != -1 && to != -1)
        Remove(from, to);
}

std::string wxTextEntryBase::GetStringSelection() const
{
    long from, to;
    GetSelection(&from, &to);

    return GetRange(from, to);
}

// ----------------------------------------------------------------------------
// clipboard
// ----------------------------------------------------------------------------

bool wxTextEntryBase::CanCopy() const
{
    return HasSelection();
}

bool wxTextEntryBase::CanCut() const
{
    return CanCopy() && IsEditable();
}

bool wxTextEntryBase::CanPaste() const
{
    if ( IsEditable() )
    {
#if wxUSE_CLIPBOARD
        // check if there is any text on the clipboard
        if ( wxTheClipboard->IsSupported(wxDF_TEXT)
                || wxTheClipboard->IsSupported(wxDF_UNICODETEXT)
           )
        {
            return true;
        }
#endif // wxUSE_CLIPBOARD
    }

    return false;
}

// ----------------------------------------------------------------------------
// input restrictions
// ----------------------------------------------------------------------------

#ifndef wxHAS_NATIVE_TEXT_FORCEUPPER

namespace
{

// Poor man's lambda: helper for binding ConvertToUpperCase() to the event
struct ForceUpperFunctor
{
    // This class must have default ctor in wxNO_RTTI case, so allow creating
    // it with null entry even if this never actually happens in practice.
    explicit ForceUpperFunctor(wxTextEntryBase* entry = NULL)
        : m_entry(entry)
    {
    }

    void operator()(wxCommandEvent& event)
    {
        event.Skip();
        m_entry->ConvertToUpperCase();
    }

    wxTextEntryBase* const m_entry;
};

} // anonymous namespace

#endif // !wxHAS_NATIVE_TEXT_FORCEUPPER

void wxTextEntryBase::ConvertToUpperCase()
{
    std::string valueOld = GetValue();
    std::string valueNew{wx::utils::ToUpperCopy(valueOld)};

    if ( valueNew != valueOld )
    {
        long from, to;
        GetSelection(&from, &to);
        ChangeValue(valueNew);
        SetSelection(from, to);
    }
}

void wxTextEntryBase::ForceUpper()
{
    // Do nothing if this method is never called because a native override is
    // provided: this is just a tiny size-saving optimization, nothing else.
#ifndef wxHAS_NATIVE_TEXT_FORCEUPPER
    wxWindow* const win = GetEditableWindow();
    wxCHECK_RET( win, wxS("can't be called before creating the window") );

    // Convert the current control contents to upper case
    ConvertToUpperCase();

    // And ensure that any text entered in the future is converted too
    win->Bind(wxEVT_TEXT, ForceUpperFunctor(this));
#endif // !wxHAS_NATIVE_TEXT_FORCEUPPER
}

// ----------------------------------------------------------------------------
// hints support
// ----------------------------------------------------------------------------

bool wxTextEntryBase::SetHint(const std::string& hint)
{
    // Hint contents would be shown hidden in a password text entry anyhow, so
    // we just can't support hints in this case.
    if ( GetEditableWindow()->HasFlag(wxTE_PASSWORD) )
        return false;

    if ( !hint.empty() )
    {
        if ( !m_hintData )
            m_hintData = new wxTextEntryHintData(this, GetEditableWindow());

        m_hintData->SetHintString(hint);
    }
    else if ( m_hintData )
    {
        // Setting empty hint removes any currently set one.
        delete m_hintData;
        m_hintData = nullptr;
    }
    //else: Setting empty hint when we don't have any doesn't do anything.

    return true;
}

std::string wxTextEntryBase::GetHint() const
{
    return m_hintData ? m_hintData->GetHintString() : "";
}

// ----------------------------------------------------------------------------
// margins support
// ----------------------------------------------------------------------------

bool wxTextEntryBase::DoSetMargins(const wxPoint& WXUNUSED(pt))
{
    return false;
}

wxPoint wxTextEntryBase::DoGetMargins() const
{
    return wxPoint(-1, -1);
}

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

/* static */
bool wxTextEntryBase::SendTextUpdatedEvent(wxWindow *win)
{
    wxCHECK_MSG( win, false, "can't send an event without a window" );

    wxCommandEvent event(wxEVT_TEXT, win->GetId());

    // do not do this as it could be very inefficient if the text control
    // contains a lot of text and we're not using ref-counted std::string
    // implementation -- instead, event.GetString() will query the control for
    // its current text if needed
    //event.SetString(win->GetValue());

    event.SetEventObject(win);
    return win->HandleWindowEvent(event);
}

// ----------------------------------------------------------------------------
// auto-completion stubs
// ----------------------------------------------------------------------------

bool wxTextCompleterSimple::Start(const std::string& prefix)
{
    m_index = 0;
    m_completions.clear();
    m_completions = GetCompletions(prefix);

    return !m_completions.empty();
}

std::string wxTextCompleterSimple::GetNext()
{
    if ( m_index == m_completions.size() )
        return {};

    return m_completions[m_index++];
}

bool wxTextEntryBase::DoAutoCompleteCustom(wxTextCompleter *completer)
{
    // We don't do anything here but we still need to delete the completer for
    // consistency with the ports that do implement this method and take
    // ownership of the pointer.
    delete completer;

    return false;
}

#endif // wxUSE_TEXTCTRL || wxUSE_COMBOBOX
