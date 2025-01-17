/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/helpfrm.h
// Purpose:     wxHtmlHelpFrame
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden and Vaclav Slavik
// Copyright:   (c) Harm van der Heijden and Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPFRM_H_
#define _WX_HELPFRM_H_

#if wxUSE_WXHTML_HELP

#include "wx/helpbase.h"
#include "wx/html/helpdata.h"
#include "wx/html/htmlwin.h"
#include "wx/html/helpwnd.h"
#include "wx/html/htmprint.h"

class wxButton;
class wxTextCtrl;
class wxTreeEvent;
class wxTreeCtrl;


// style flags for the Help Frame
#define wxHF_TOOLBAR                0x0001
#define wxHF_CONTENTS               0x0002
#define wxHF_INDEX                  0x0004
#define wxHF_SEARCH                 0x0008
#define wxHF_BOOKMARKS              0x0010
#define wxHF_OPEN_FILES             0x0020
#define wxHF_PRINT                  0x0040
#define wxHF_FLAT_TOOLBAR           0x0080
#define wxHF_MERGE_BOOKS            0x0100
#define wxHF_ICONS_BOOK             0x0200
#define wxHF_ICONS_BOOK_CHAPTER     0x0400
#define wxHF_ICONS_FOLDER           0x0000 // this is 0 since it is default
#define wxHF_DEFAULT_STYLE          (wxHF_TOOLBAR | wxHF_CONTENTS | \
                                     wxHF_INDEX | wxHF_SEARCH | \
                                     wxHF_BOOKMARKS | wxHF_PRINT)

struct wxHtmlHelpMergedIndexItem;
class wxHtmlHelpMergedIndex;

class wxHelpControllerBase;
class wxHtmlHelpController;
class wxHtmlHelpWindow;

class wxHtmlHelpFrame : public wxFrame
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlHelpFrame);

public:
    wxHtmlHelpFrame(wxHtmlHelpData* data = nullptr) { Init(data); }
    wxHtmlHelpFrame(wxWindow* parent, wxWindowID id,
                    const std::string& title = {},
                    unsigned int style = wxHF_DEFAULT_STYLE, wxHtmlHelpData* data = nullptr
#if wxUSE_CONFIG
                    , wxConfigBase *config=nullptr, const std::string& rootpath = {}
#endif // wxUSE_CONFIG
                    );
    bool Create(wxWindow* parent, wxWindowID id, const std::string& title = {},
                unsigned int style = wxHF_DEFAULT_STYLE
#if wxUSE_CONFIG
                , wxConfigBase *config=nullptr, const std::string& rootpath = {}
#endif // wxUSE_CONFIG
                );

    wxHtmlHelpFrame(const wxHtmlHelpFrame&) = delete;
	wxHtmlHelpFrame& operator=(const wxHtmlHelpFrame&) = delete;

    /// Returns the data associated with the window.
    wxHtmlHelpData* GetData() { return m_Data; }

    /// Returns the help controller associated with the window.
    wxHtmlHelpController* GetController() const { return m_helpController; }

    /// Sets the help controller associated with the window.
    void SetController(wxHtmlHelpController* controller);

    /// Returns the help window.
    wxHtmlHelpWindow* GetHelpWindow() const { return m_HtmlHelpWin; }

    // Sets format of title of the frame. Must contain exactly one "%s"
    // (for title of displayed HTML page)
    void SetTitleFormat(const std::string& format);

#if wxUSE_CONFIG
    // For compatibility
    void UseConfig(wxConfigBase *config, const std::string& rootpath = {});
#endif // wxUSE_CONFIG

    // Make the help controller's frame 'modal' if
    // needed
    void AddGrabIfNeeded();

    // Override to add custom buttons to the toolbar
    virtual void AddToolbarButtons([[maybe_unused]] wxToolBar* toolBar, [[maybe_unused]] unsigned int style) {}

    void SetShouldPreventAppExit(bool enable);

    // we don't want to prevent the app from closing just because a help window
    // remains opened
    bool ShouldPreventAppExit() const override { return m_shouldPreventAppExit; }

protected:
    void Init(wxHtmlHelpData* data = nullptr);

    void OnCloseWindow(wxCloseEvent& event);
    void OnActivate(wxActivateEvent& event);

    // Images:
    enum {
        IMG_Book = 0,
        IMG_Folder,
        IMG_Page
    };

protected:
    wxHtmlHelpData* m_Data;
    bool m_DataCreated;  // m_Data created by frame, or supplied?
    std::string m_TitleFormat;  // title of the help frame
    wxHtmlHelpWindow *m_HtmlHelpWin;
    wxHtmlHelpController* m_helpController;
    bool m_shouldPreventAppExit;

private:
    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_WXHTML_HELP

#endif
