/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/helpdlg.h
// Purpose:     wxHtmlHelpDialog
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden, Vaclav Slavik, Julian Smart
// Copyright:   (c) Harm van der Heijden, Vaclav Slavik, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPDLG_H_
#define _WX_HELPDLG_H_

#include "wx/defs.h"

#if wxUSE_WXHTML_HELP

#include "wx/dialog.h"
#include "wx/string.h"

#include "wx/html/helpdata.h"
#include "wx/html/htmlwin.h"
#include "wx/html/helpwnd.h"
#include "wx/html/htmprint.h"

class wxHtmlHelpController;
class wxHtmlHelpWindow;

class wxWindow;

class WXDLLIMPEXP_HTML wxHtmlHelpDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlHelpDialog);

public:
    wxHtmlHelpDialog(wxHtmlHelpData* data = nullptr) { Init(data); }
    wxHtmlHelpDialog(wxWindow* parent, wxWindowID wxWindowID,
                    const wxString& title = {},
                    unsigned int style = wxHF_DEFAULT_STYLE, wxHtmlHelpData* data = nullptr);

    wxHtmlHelpDialog(const wxHtmlHelpDialog&) = delete;
	wxHtmlHelpDialog& operator=(const wxHtmlHelpDialog&) = delete;

    bool Create(wxWindow* parent, wxWindowID id, const wxString& title = {},
                unsigned int style = wxHF_DEFAULT_STYLE);

    /// Returns the data associated with this dialog.
    wxHtmlHelpData* GetData() { return m_Data; }

    /// Returns the controller that created this dialog.
    wxHtmlHelpController* GetController() const { return m_helpController; }

    /// Sets the controller associated with this dialog.
    void SetController(wxHtmlHelpController* controller) { m_helpController = controller; }

    /// Returns the help window.
    wxHtmlHelpWindow* GetHelpWindow() const { return m_HtmlHelpWin; }

    // Sets format of title of the frame. Must contain exactly one "%s"
    // (for title of displayed HTML page)
    void SetTitleFormat(const wxString& format);

    // Override to add custom buttons to the toolbar
    virtual void AddToolbarButtons(wxToolBar* WXUNUSED(toolBar), int WXUNUSED(style)) {}

protected:
    void Init(wxHtmlHelpData* data = nullptr);

    void OnCloseWindow(wxCloseEvent& event);

protected:
    // Temporary pointer to pass to window
    wxHtmlHelpData* m_Data;
    wxString m_TitleFormat;  // title of the help frame
    wxHtmlHelpWindow *m_HtmlHelpWin;
    wxHtmlHelpController* m_helpController;

    wxDECLARE_EVENT_TABLE();
};

#endif
    // wxUSE_WXHTML_HELP

#endif
