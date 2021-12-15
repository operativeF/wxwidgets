/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/helpctrl.h
// Purpose:     wxHtmlHelpController
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden and Vaclav Slavik
// Copyright:   (c) Harm van der Heijden and Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPCTRL_H_
#define _WX_HELPCTRL_H_

#if wxUSE_WXHTML_HELP

#include "wx/helpbase.h"
#include "wx/html/helpfrm.h"

#define wxID_HTML_HELPFRAME   (wxID_HIGHEST + 1)

// This style indicates that the window is
// embedded in the application and must not be
// destroyed by the help controller.
#define wxHF_EMBEDDED                0x00008000

// Create a dialog for the help window.
#define wxHF_DIALOG                  0x00010000

// Create a frame for the help window.
#define wxHF_FRAME                   0x00020000

// Make the dialog modal when displaying help.
#define wxHF_MODAL                   0x00040000

class wxHtmlHelpDialog;
class wxHtmlHelpWindow;
class wxHtmlHelpFrame;
class wxHtmlHelpDialog;

class wxHtmlHelpController : public wxHelpControllerBase // wxEvtHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlHelpController);

public:
    wxHtmlHelpController(unsigned int style = wxHF_DEFAULT_STYLE, wxWindow* parentWindow = nullptr);
    wxHtmlHelpController(wxWindow* parentWindow, unsigned int style = wxHF_DEFAULT_STYLE);

    ~wxHtmlHelpController();

    wxHtmlHelpController(const wxHtmlHelpController&) = delete;
	wxHtmlHelpController& operator=(const wxHtmlHelpController&) = delete;

    void SetShouldPreventAppExit(bool enable);

    void SetTitleFormat(const std::string& format);
    void SetTempDir(const std::string& path) { m_helpData.SetTempDir(path); }
    bool AddBook(const std::string& book_url, bool show_wait_msg = false);
    bool AddBook(const wxFileName& book_file, bool show_wait_msg = false);

    bool Display(const std::string& x);
    bool Display(int id);
    bool DisplayContents() override;
    bool DisplayIndex();
    bool KeywordSearch(const std::string& keyword,
                       wxHelpSearchMode mode = wxHelpSearchMode::All) override;

    wxHtmlHelpWindow* GetHelpWindow() { return m_helpWindow; }
    void SetHelpWindow(wxHtmlHelpWindow* helpWindow);

    wxHtmlHelpFrame* GetFrame() { return m_helpFrame; }
    wxHtmlHelpDialog* GetDialog() { return m_helpDialog; }

#if wxUSE_CONFIG
    void UseConfig(wxConfigBase *config, const std::string& rootpath = {});

    // Assigns config object to the Ctrl. This config is then
    // used in subsequent calls to Read/WriteCustomization of both help
    // Ctrl and it's wxHtmlWindow
    virtual void ReadCustomization(wxConfigBase *cfg, const std::string& path = {});
    virtual void WriteCustomization(wxConfigBase *cfg, const std::string& path = {});
#endif // wxUSE_CONFIG

    //// Backward compatibility with wxHelpController API

    bool Initialize(const std::string& file, [[maybe_unused]] int server ) override { return Initialize(file); }
    bool Initialize(const std::string& file) override;
    void SetViewer([[maybe_unused]] const std::string& viewer, [[maybe_unused]] unsigned int flags = 0) override {}
    bool LoadFile(const std::string& file = {}) override;
    bool DisplaySection(int sectionNo) override;
    bool DisplaySection(const std::string& section) override { return Display(section); }
    bool DisplayBlock(long blockNo) override { return DisplaySection(blockNo); }
    bool DisplayTextPopup(const std::string& text, const wxPoint& pos) override;

    void SetFrameParameters(const std::string& titleFormat,
                               const wxSize& size,
                               const wxPoint& pos = wxDefaultPosition,
                               bool newFrameEachTime = false) override;
    /// Obtains the latest settings used by the help frame and the help
    /// frame.
    wxFrame *GetFrameParameters(wxSize *size = nullptr,
                               wxPoint *pos = nullptr,
                               bool *newFrameEachTime = nullptr) override;

    // Get direct access to help data:
    wxHtmlHelpData *GetHelpData() { return &m_helpData; }

    bool Quit() override;
    void OnQuit() override {}

    void OnCloseFrame(wxCloseEvent& evt);

    // Make the help controller's frame 'modal' if
    // needed
    void MakeModalIfNeeded();

    // Find the top-most parent window
    wxWindow* FindTopLevelWindow();

protected:
    void Init(unsigned int style);

    virtual wxWindow* CreateHelpWindow();
    virtual wxHtmlHelpFrame* CreateHelpFrame(wxHtmlHelpData *data);
    virtual wxHtmlHelpDialog* CreateHelpDialog(wxHtmlHelpData *data);
    virtual void DestroyHelpWindow();

    wxHtmlHelpData      m_helpData;
    wxHtmlHelpWindow*   m_helpWindow;
#if wxUSE_CONFIG
    wxConfigBase *      m_Config;
    std::string            m_ConfigRoot;
#endif // wxUSE_CONFIG
    std::string            m_titleFormat;
    unsigned int        m_FrameStyle;
    wxHtmlHelpFrame*    m_helpFrame;
    wxHtmlHelpDialog*   m_helpDialog;

    bool                m_shouldPreventAppExit;
};

/*
 * wxHtmlModalHelp
 * A convenience class particularly for use on wxMac,
 * where you can only show modal dialogs from a modal
 * dialog.
 *
 * Use like this:
 *
 * wxHtmlModalHelp help(parent, filename, topic);
 *
 * If topic is empty, the help contents is displayed.
 */

class wxHtmlModalHelp
{
public:
    wxHtmlModalHelp(wxWindow* parent, const std::string& helpFile, const std::string& topic = {},
        unsigned int style = wxHF_DEFAULT_STYLE | wxHF_DIALOG | wxHF_MODAL);
};

#endif // wxUSE_WXHTML_HELP

#endif // _WX_HELPCTRL_H_
