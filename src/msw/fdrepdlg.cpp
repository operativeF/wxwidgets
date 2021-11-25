/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/fdrepdlg.cpp
// Purpose:     wxFindReplaceDialog class
// Author:      Markus Greither and Vadim Zeitlin
// Modified by:
// Created:     23/03/2001
// Copyright:   (c) Markus Greither
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FINDREPLDLG

#include "wx/msw/wrapcdlg.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/fdrepdlg.h"

#include <boost/nowide/convert.hpp>

// ----------------------------------------------------------------------------
// functions prototypes
// ----------------------------------------------------------------------------

UINT_PTR CALLBACK wxFindReplaceDialogHookProc(WXHWND hwnd,
                                              WXUINT uiMsg,
                                              WXWPARAM wParam,
                                              WXLPARAM lParam);

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxFindReplaceDialog, wxDialog);

// ----------------------------------------------------------------------------
// wxFindReplaceDialogImpl: the internals of wxFindReplaceDialog
// ----------------------------------------------------------------------------

class wxFindReplaceDialogImpl
{
public:
    wxFindReplaceDialogImpl(wxFindReplaceDialog *dialog, FindReplaceFlags flagsWX);
    ~wxFindReplaceDialogImpl();

    wxFindReplaceDialogImpl(const wxFindReplaceDialogImpl&) = delete;
	wxFindReplaceDialogImpl& operator=(const wxFindReplaceDialogImpl&) = delete;

    void InitFindWhat(const std::string& str);
    void InitReplaceWith(const std::string& str);

    // only for passing to ::FindText or ::ReplaceText
    FINDREPLACE *GetPtrFindReplace() { return &m_findReplace; }

    // set/query the "closed by user" flag
    void SetClosedByUser() { m_wasClosedByUser = true; }
    bool WasClosedByUser() const { return m_wasClosedByUser; }

private:
    // called from window procedure for ms_msgFindDialog
    static bool FindMessageHandler(wxWindow *win,
                                   WXUINT nMsg,
                                   WXWPARAM wParam,
                                   WXLPARAM lParam);

    // copy string str contents to ppStr and fill pLen with its length
    void InitString(const std::string& str, LPTSTR *ppStr, WXWORD *pLen);


    // the find replace data used by the dialog
    FINDREPLACE m_findReplace;

    // true if the user closed us, false otherwise
    bool m_wasClosedByUser{false};

    // registered Message for Dialog
    inline static WXUINT ms_msgFindDialog{0};
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxFindReplaceDialogImpl
// ----------------------------------------------------------------------------

wxFindReplaceDialogImpl::wxFindReplaceDialogImpl(wxFindReplaceDialog *dialog,
                                                 FindReplaceFlags flagsWX)
{
    // get the identifier for the find dialog message if we don't have it yet
    if ( !ms_msgFindDialog )
    {
        ms_msgFindDialog = ::RegisterWindowMessageW(FINDMSGSTRING);

        if ( !ms_msgFindDialog )
        {
            wxLogLastError("RegisterWindowMessage(FINDMSGSTRING)");
        }

        wxWindow::MSWRegisterMessageHandler
                  (
                    ms_msgFindDialog,
                    &wxFindReplaceDialogImpl::FindMessageHandler
                  );
    }

    wxZeroMemory(m_findReplace);

    // translate the flags: first the dialog creation flags

    // always set this to be able to set the title
    unsigned int flags = FR_ENABLEHOOK;

    unsigned int flagsDialog = dialog->wxGetWindowStyle();
    if ( flagsDialog & wxFR_NOMATCHCASE)
        flags |= FR_NOMATCHCASE;
    if ( flagsDialog & wxFR_NOWHOLEWORD)
        flags |= FR_NOWHOLEWORD;
    if ( flagsDialog & wxFR_NOUPDOWN)
        flags |= FR_NOUPDOWN;

    // and now the flags governing the initial values of the dialogs controls
    if ( flagsWX & wxFindReplaceFlags::Down)
        flags |= FR_DOWN;
    if ( flagsWX & wxFindReplaceFlags::MatchCase)
        flags |= FR_MATCHCASE;
    if ( flagsWX & wxFindReplaceFlags::WholeWord )
        flags |= FR_WHOLEWORD;

    m_findReplace.lStructSize = sizeof(FINDREPLACE);
    m_findReplace.hwndOwner = GetHwndOf(dialog->GetParent());
    m_findReplace.Flags = flags;

    m_findReplace.lCustData = (WXLPARAM)dialog;
    m_findReplace.lpfnHook = wxFindReplaceDialogHookProc;
}

void wxFindReplaceDialogImpl::InitString(const std::string& str,
                                         LPTSTR *ppStr, WXWORD *pLen)
{
    size_t len = str.length() + 1;
    if ( len < 80 )
    {
        // MSDN docs say that the buffer must be at least 80 chars
        len = 80;
    }

    *ppStr = new wxChar[len];
    wxStrcpy(*ppStr, str);
    *pLen = (WXWORD)len;
}

void wxFindReplaceDialogImpl::InitFindWhat(const std::string& str)
{
    InitString(str, &m_findReplace.lpstrFindWhat, &m_findReplace.wFindWhatLen);
}

void wxFindReplaceDialogImpl::InitReplaceWith(const std::string& str)
{
    InitString(str,
               &m_findReplace.lpstrReplaceWith,
               &m_findReplace.wReplaceWithLen);
}

wxFindReplaceDialogImpl::~wxFindReplaceDialogImpl()
{
    delete [] m_findReplace.lpstrFindWhat;
    delete [] m_findReplace.lpstrReplaceWith;
}

// ----------------------------------------------------------------------------
// handler for FINDMSGSTRING message
// ----------------------------------------------------------------------------

bool
wxFindReplaceDialogImpl::FindMessageHandler([[maybe_unused]] wxWindow * win,
                                            WXUINT WXUNUSED_UNLESS_DEBUG(nMsg),
                                            [[maybe_unused]] WXWPARAM wParam,
                                            WXLPARAM lParam)
{
    wxASSERT_MSG( nMsg == ms_msgFindDialog, "unexpected message received" );

    FINDREPLACE *pFR = (FINDREPLACE *)lParam;

    wxFindReplaceDialog *dialog = (wxFindReplaceDialog *)pFR->lCustData;

    // map flags from Windows
    wxEventType evtType;

    bool replace = false;

    if ( pFR->Flags & FR_DIALOGTERM )
    {
        // we have to notify the dialog that it's being closed by user and
        // not deleted programmatically as it behaves differently in these
        // 2 cases
        dialog->GetImpl()->SetClosedByUser();

        evtType = wxEVT_FIND_CLOSE;
    }
    else if ( pFR->Flags & FR_FINDNEXT )
    {
        evtType = wxEVT_FIND_NEXT;
    }
    else if ( pFR->Flags & FR_REPLACE )
    {
        evtType = wxEVT_FIND_REPLACE;

        replace = true;
    }
    else if ( pFR->Flags & FR_REPLACEALL )
    {
        evtType = wxEVT_FIND_REPLACE_ALL;

        replace = true;
    }
    else
    {
        wxFAIL_MSG( "unknown find dialog event" );

        return false;
    }

    FindReplaceFlags flags{};

    if ( pFR->Flags & FR_DOWN )
        flags |= wxFindReplaceFlags::Down;
    if ( pFR->Flags & FR_WHOLEWORD )
        flags |= wxFindReplaceFlags::WholeWord;
    if ( pFR->Flags & FR_MATCHCASE )
        flags |= wxFindReplaceFlags::MatchCase;

    wxFindDialogEvent event(evtType, dialog->GetId());

    event.SetEventObject(dialog);
    event.SetFlags(flags);
    event.SetFindString(boost::nowide::narrow(pFR->lpstrFindWhat));
    if ( replace )
    {
        event.SetReplaceString(boost::nowide::narrow(pFR->lpstrReplaceWith));
    }

    dialog->Send(event);

    return true;
}

// ----------------------------------------------------------------------------
// Find/replace dialog hook proc
// ----------------------------------------------------------------------------

UINT_PTR CALLBACK
wxFindReplaceDialogHookProc(WXHWND hwnd,
                            WXUINT uiMsg,
                            [[maybe_unused]] WXWPARAM wParam,
                            WXLPARAM lParam)
{
    if ( uiMsg == WM_INITDIALOG )
    {
        FINDREPLACE *pFR = (FINDREPLACE *)lParam;
        wxFindReplaceDialog *dialog = (wxFindReplaceDialog *)pFR->lCustData;

        ::SetWindowTextW(hwnd, boost::nowide::widen(dialog->GetTitle()).c_str());

        // don't return FALSE from here or the dialog won't be shown
        return TRUE;
    }

    return 0;
}

// ----------------------------------------------------------------------------
// wxFindReplaceDialog ctors/dtor
// ----------------------------------------------------------------------------

wxFindReplaceDialog::wxFindReplaceDialog(wxWindow *parent,
                                         wxFindReplaceData *data,
                                         const std::string &title,
                                         unsigned int flags)
                   : wxFindReplaceDialogBase(parent, data, title, flags)
{
    (void)Create(parent, data, title, flags);
}

wxFindReplaceDialog::~wxFindReplaceDialog()
{
    if ( m_impl )
    {
        // the dialog might have been already deleted if the user closed it
        // manually but in this case we should have got a notification about it
        // and the flag must have been set
        if ( !m_impl->WasClosedByUser() )
        {
            // if it wasn't, delete the dialog ourselves
            if ( !::DestroyWindow(GetHwnd()) )
            {
                wxLogLastError("DestroyWindow(find dialog)");
            }
        }

        // unsubclass the parent
        delete m_impl;
    }

    // prevent the base class dtor from trying to hide us!
    m_isShown = false;

    // and from destroying our window [again]
    m_hWnd = (WXHWND)nullptr;
}

bool wxFindReplaceDialog::Create(wxWindow *parent,
                                 wxFindReplaceData *data,
                                 const std::string &title,
                                 unsigned int flags)
{
    m_windowStyle = flags;
    m_FindReplaceData = data;

    if ( parent )
        parent->AddChild(this);

    SetTitle(title);

    // we must have a parent as it will get the messages from us
    return parent != nullptr;
}

// ----------------------------------------------------------------------------
// wxFindReplaceData show/hide
// ----------------------------------------------------------------------------

bool wxFindReplaceDialog::Show(bool show)
{
    if ( !wxWindowBase::Show(show) )
    {
        // visibility status didn't change
        return false;
    }

    // do we already have the dialog window?
    if ( m_hWnd )
    {
        // yes, just use it
        (void)::ShowWindow(GetHwnd(), show ? SW_SHOW : SW_HIDE);

        return true;
    }

    if ( !show )
    {
        // well, it doesn't exist which is as good as being hidden
        return true;
    }

    wxCHECK_MSG( m_FindReplaceData, false, "call Create() first!" );

    wxASSERT_MSG( !m_impl, "why don't we have the window then?" );

    m_impl = new wxFindReplaceDialogImpl(this, m_FindReplaceData->GetFlags());

    m_impl->InitFindWhat(m_FindReplaceData->GetFindString());

    bool replace = HasFlag(wxFR_REPLACEDIALOG);
    if ( replace )
    {
        m_impl->InitReplaceWith(m_FindReplaceData->GetReplaceString());
    }

    // call the right function to show the dialog which does what we want
    FINDREPLACE *pFR = m_impl->GetPtrFindReplace();
    WXHWND hwnd;
    if ( replace )
        hwnd = ::ReplaceTextW(pFR);
    else
        hwnd = ::FindTextW(pFR);

    if ( !hwnd )
    {
        wxLogError(_("Failed to create the standard find/replace dialog (error code %d)"),
                   ::CommDlgExtendedError());

        wxDELETE(m_impl);

        return false;
    }

    if ( !::ShowWindow(hwnd, SW_SHOW) )
    {
        wxLogLastError("ShowWindow(find dialog)");
    }

    m_hWnd = (WXHWND)hwnd;

    return true;
}

// ----------------------------------------------------------------------------
// wxFindReplaceDialog title handling
// ----------------------------------------------------------------------------

// we set the title of this dialog in our jook proc but for now don't crash in
// the base class version because of m_hWnd == 0

void wxFindReplaceDialog::SetTitle( const std::string& title)
{
    m_title = title;
}

std::string wxFindReplaceDialog::GetTitle() const
{
    return m_title;
}

// ----------------------------------------------------------------------------
// wxFindReplaceDialog position/size
// ----------------------------------------------------------------------------

void wxFindReplaceDialog::DoSetSize([[maybe_unused]] wxRect boundary, [[maybe_unused]] unsigned int sizeFlags)
{
    // ignore - we can't change the size of this standard dialog
    return;
}

// NB: of course, both of these functions are completely bogus, but it's better
//     than nothing
wxSize wxFindReplaceDialog::DoGetSize() const
{
    // FIXME: Magic values.
    // the standard dialog size
    return { 225, 324 };
}

wxSize wxFindReplaceDialog::DoGetClientSize() const
{
    // FIXME: Magic values.
    return { 219, 299 };
}

#endif // wxUSE_FINDREPLDLG
