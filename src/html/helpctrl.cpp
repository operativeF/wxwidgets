/////////////////////////////////////////////////////////////////////////////
// Name:        src/html/helpctrl.cpp
// Purpose:     wxHtmlHelpController
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden and Vaclav Slavik
// Copyright:   (c) Harm van der Heijden and Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#if wxUSE_WXHTML_HELP

#include "wx/app.h"
#include "wx/intl.h"
#include "wx/busyinfo.h"
#include "wx/html/helpctrl.h"
#include "wx/html/helpwnd.h"
#include "wx/html/helpfrm.h"
#include "wx/html/helpdlg.h"

#if wxUSE_HELP
import WX.Core.TipWin;
#endif

#if wxUSE_LIBMSPACK
#include "wx/html/forcelnk.h"
FORCE_LINK(wxhtml_chm_support)
#endif

wxHtmlHelpController::wxHtmlHelpController(unsigned int style, wxWindow* parentWindow):
    wxHelpControllerBase(parentWindow)
{
    Init(style);
}

wxHtmlHelpController::wxHtmlHelpController(wxWindow* parentWindow, unsigned int style):
    wxHelpControllerBase(parentWindow)
{
    Init(style);
}

void wxHtmlHelpController::Init(unsigned int style)
{
    m_helpWindow = nullptr;
    m_helpFrame = nullptr;
    m_helpDialog = nullptr;
#if wxUSE_CONFIG
    m_Config = nullptr;
    m_ConfigRoot.clear();
#endif // wxUSE_CONFIG
    m_titleFormat = _("Help: %s");
    m_FrameStyle = style;
    m_shouldPreventAppExit = false;
}


wxHtmlHelpController::~wxHtmlHelpController()
{
#if wxUSE_CONFIG
    if (m_Config)
        WriteCustomization(m_Config, m_ConfigRoot);
#endif // wxUSE_CONFIG
    if (m_helpWindow)
        DestroyHelpWindow();
}


void wxHtmlHelpController::DestroyHelpWindow()
{
    if (m_FrameStyle & wxHF_EMBEDDED)
        return;

    // Find top-most parent window
    // If a modal dialog
    wxWindow* parent = FindTopLevelWindow();
    if (parent)
    {
        wxDialog* dialog = dynamic_cast<wxDialog*>(parent);
        if (dialog && dialog->IsModal())
        {
            dialog->EndModal(wxID_OK);
        }
        parent->Destroy();
        m_helpWindow = nullptr;
    }
    m_helpDialog = nullptr;
    m_helpFrame = nullptr;
}

void wxHtmlHelpController::OnCloseFrame(wxCloseEvent& evt)
{
#if wxUSE_CONFIG
    if (m_Config)
        WriteCustomization(m_Config, m_ConfigRoot);
#endif // wxUSE_CONFIG

    evt.Skip();

    OnQuit();

    if ( m_helpWindow )
        m_helpWindow->SetController(nullptr);
    m_helpWindow = nullptr;
    m_helpDialog = nullptr;
    m_helpFrame = nullptr;
}

void wxHtmlHelpController::SetShouldPreventAppExit(bool enable)
{
    m_shouldPreventAppExit = enable;
    if ( m_helpFrame )
        m_helpFrame->SetShouldPreventAppExit(enable);
}

void wxHtmlHelpController::SetTitleFormat(const std::string& title)
{
    m_titleFormat = title;
    wxHtmlHelpFrame* frame = dynamic_cast<wxHtmlHelpFrame*>(FindTopLevelWindow());
    wxHtmlHelpDialog* dialog = dynamic_cast<wxHtmlHelpDialog*>(FindTopLevelWindow());
    if (frame)
    {
        frame->SetTitleFormat(title);
    }
    else if (dialog)
        dialog->SetTitleFormat(title);
}

// Find the top-most parent window
wxWindow* wxHtmlHelpController::FindTopLevelWindow()
{
    return wxGetTopLevelParent(m_helpWindow);
}

bool wxHtmlHelpController::AddBook(const wxFileName& book_file, bool show_wait_msg)
{
    return AddBook(wxFileSystem::FileNameToURL(book_file), show_wait_msg);
}

bool wxHtmlHelpController::AddBook(const std::string& book, bool show_wait_msg)
{
    wxBusyCursor cur;
#if wxUSE_BUSYINFO
    wxBusyInfo* busy = nullptr;
    wxString info;
    if (show_wait_msg)
    {
        info.Printf(_("Adding book %s"), book.c_str());
        busy = new wxBusyInfo(info);
    }
#endif
    bool retval = m_helpData.AddBook(book);
#if wxUSE_BUSYINFO
    if (show_wait_msg)
        delete busy;
#else
    wxUnusedVar(show_wait_msg);
#endif
    if (m_helpWindow)
        m_helpWindow->RefreshLists();
    return retval;
}

wxHtmlHelpFrame* wxHtmlHelpController::CreateHelpFrame(wxHtmlHelpData *data)
{
    wxHtmlHelpFrame* frame = new wxHtmlHelpFrame(data);
    frame->SetController(this);
    frame->SetTitleFormat(m_titleFormat);
    frame->Create(m_parentWindow, -1, {}, m_FrameStyle
#if wxUSE_CONFIG
        , m_Config, m_ConfigRoot
#endif // wxUSE_CONFIG
        );
    frame->SetShouldPreventAppExit(m_shouldPreventAppExit);
    m_helpFrame = frame;
    return frame;
}

wxHtmlHelpDialog* wxHtmlHelpController::CreateHelpDialog(wxHtmlHelpData *data)
{
    wxHtmlHelpDialog* dialog = new wxHtmlHelpDialog(data);
    dialog->SetController(this);
    dialog->SetTitleFormat(m_titleFormat);
    dialog->Create(m_parentWindow, -1, {}, m_FrameStyle);
    m_helpDialog = dialog;
    return dialog;
}

wxWindow* wxHtmlHelpController::CreateHelpWindow()
{
    if (m_helpWindow)
    {
        if (m_FrameStyle & wxHF_EMBEDDED)
            return m_helpWindow;

        wxWindow* topLevelWindow = FindTopLevelWindow();
        if (topLevelWindow)
            topLevelWindow->Raise();
        return m_helpWindow;
    }

#if wxUSE_CONFIG
    if (m_Config == nullptr)
    {
        m_Config = wxConfigBase::Get(false);
        if (m_Config != nullptr)
            m_ConfigRoot = "wxWindows/wxHtmlHelpController";
    }
#endif // wxUSE_CONFIG

    if (m_FrameStyle & wxHF_DIALOG)
    {
        wxHtmlHelpDialog* dialog = CreateHelpDialog(&m_helpData);
        m_helpWindow = dialog->GetHelpWindow();
    }
    else if ((m_FrameStyle & wxHF_EMBEDDED) && m_parentWindow)
    {
        m_helpWindow = new wxHtmlHelpWindow(m_parentWindow, -1, wxDefaultPosition, wxDefaultSize,
            wxTAB_TRAVERSAL|wxNO_BORDER, m_FrameStyle, &m_helpData);
    }
    else // wxHF_FRAME
    {
        wxHtmlHelpFrame* frame = CreateHelpFrame(&m_helpData);
        m_helpWindow = frame->GetHelpWindow();
        frame->Show(true);
    }

    return m_helpWindow;
}

#if wxUSE_CONFIG
void wxHtmlHelpController::ReadCustomization(wxConfigBase* cfg, const std::string& path)
{
    /* should not be called by the user; call UseConfig, and the controller
     * will do the rest */
    if (m_helpWindow && cfg)
        m_helpWindow->ReadCustomization(cfg, path);
}

void wxHtmlHelpController::WriteCustomization(wxConfigBase* cfg, const std::string& path)
{
    /* typically called by the controllers OnCloseFrame handler */
    if (m_helpWindow && cfg)
        m_helpWindow->WriteCustomization(cfg, path);
}

void wxHtmlHelpController::UseConfig(wxConfigBase *config, const std::string& rootpath)
{
    m_Config = config;
    m_ConfigRoot = rootpath;
    if (m_helpWindow) m_helpWindow->UseConfig(config, rootpath);
    ReadCustomization(config, rootpath);
}
#endif // wxUSE_CONFIG

//// Backward compatibility with wxHelpController API

bool wxHtmlHelpController::Initialize(const std::string& file)
{
    wxString dir, filename, ext;
    wxFileName::SplitPath(file, & dir, & filename, & ext);

    if (!dir.empty())
        dir = dir + wxFILE_SEP_PATH;

    // Try to find a suitable file
    std::string actualFilename = dir + filename + wxString(".zip");
    if (!wxFileExists(actualFilename))
    {
        actualFilename = dir + filename + wxString(".htb");
        if (!wxFileExists(actualFilename))
        {
            actualFilename = dir + filename + wxString(".hhp");
            if (!wxFileExists(actualFilename))
            {
#if wxUSE_LIBMSPACK
                actualFilename = dir + filename + wxString(".chm");
                if (!wxFileExists(actualFilename))
#endif
                    return false;
            }
        }
    }
    return AddBook(wxFileName(actualFilename));
}

bool wxHtmlHelpController::LoadFile([[maybe_unused]] const std::string& file)
{
    // Don't reload the file or we'll have it appear again, presumably.
    return true;
}

bool wxHtmlHelpController::DisplaySection(int sectionNo)
{
    return Display(sectionNo);
}

bool wxHtmlHelpController::DisplayTextPopup(const std::string& text, [[maybe_unused]] const wxPoint& pos)
{
#if wxUSE_TIPWINDOW
    static wxTipWindow* s_tipWindow = nullptr;

    if (s_tipWindow)
    {
        // Prevent s_tipWindow being nulled in OnIdle,
        // thereby removing the chance for the window to be closed by ShowHelp
        s_tipWindow->SetTipWindowPtr(nullptr);
        s_tipWindow->Close();
    }
    s_tipWindow = nullptr;

    if ( !text.empty() )
    {
        s_tipWindow = new wxTipWindow(wxTheApp->GetTopWindow(), text, 100, & s_tipWindow);

        return true;
    }
#else
    wxUnusedVar(text);
#endif // wxUSE_TIPWINDOW

    return false;
}

void wxHtmlHelpController::SetHelpWindow(wxHtmlHelpWindow* helpWindow)
{
    m_helpWindow = helpWindow;
    if (helpWindow)
        helpWindow->SetController(this);
}

void wxHtmlHelpController::SetFrameParameters(const std::string& titleFormat,
                                   const wxSize& size,
                                   const wxPoint& pos,
                                   [[maybe_unused]] bool newFrameEachTime)
{
    SetTitleFormat(titleFormat);
    wxHtmlHelpFrame* frame = dynamic_cast<wxHtmlHelpFrame*>(FindTopLevelWindow());
    wxHtmlHelpDialog* dialog = dynamic_cast<wxHtmlHelpDialog*>(FindTopLevelWindow());
    if (frame)
        frame->SetSize(wxRect{pos, size});
    else if (dialog)
        dialog->SetSize(wxRect{pos, size});
}

wxFrame* wxHtmlHelpController::GetFrameParameters(wxSize *size,
                                   wxPoint *pos,
                                   bool *newFrameEachTime)
{
    if (newFrameEachTime)
        (* newFrameEachTime) = false;

    wxHtmlHelpFrame* frame = dynamic_cast<wxHtmlHelpFrame*>(FindTopLevelWindow());
    wxHtmlHelpDialog* dialog = dynamic_cast<wxHtmlHelpDialog*>(FindTopLevelWindow());
    if (frame)
    {
        if (size)
            (* size) = frame->GetSize();
        if (pos)
            (* pos) = frame->GetPosition();
        return frame;
    }
    else if (dialog)
    {
        if (size)
            (* size) = dialog->GetSize();
        if (pos)
            (* pos) = dialog->GetPosition();
        return nullptr;
    }
    return nullptr;
}

bool wxHtmlHelpController::Quit()
{
    DestroyHelpWindow();
    return true;
}

// Make the help controller's frame 'modal' if
// needed
void wxHtmlHelpController::MakeModalIfNeeded()
{
    if ((m_FrameStyle & wxHF_EMBEDDED) == 0)
    {
        wxHtmlHelpFrame* frame = dynamic_cast<wxHtmlHelpFrame*>(FindTopLevelWindow());
        wxHtmlHelpDialog* dialog = dynamic_cast<wxHtmlHelpDialog*>(FindTopLevelWindow());
        if (frame)
            frame->AddGrabIfNeeded();
        else if (dialog && (m_FrameStyle & wxHF_MODAL))
        {
            dialog->ShowModal();
        }
    }
}

bool wxHtmlHelpController::Display(const std::string& x)
{
    CreateHelpWindow();
    bool success = m_helpWindow->Display(x);
    MakeModalIfNeeded();
    return success;
}

bool wxHtmlHelpController::Display(int id)
{
    CreateHelpWindow();
    bool success = m_helpWindow->Display(id);
    MakeModalIfNeeded();
    return success;
}

bool wxHtmlHelpController::DisplayContents()
{
    CreateHelpWindow();
    bool success = m_helpWindow->DisplayContents();
    MakeModalIfNeeded();
    return success;
}

bool wxHtmlHelpController::DisplayIndex()
{
    CreateHelpWindow();
    bool success = m_helpWindow->DisplayIndex();
    MakeModalIfNeeded();
    return success;
}

bool wxHtmlHelpController::KeywordSearch(const std::string& keyword,
                                         wxHelpSearchMode mode)
{
    CreateHelpWindow();
    bool success = m_helpWindow->KeywordSearch(keyword, mode);
    MakeModalIfNeeded();
    return success;
}

/*
 * wxHtmlModalHelp
 * A convenience class, to use like this:
 *
 * wxHtmlModalHelp help(parent, helpFile, topic);
 */

wxHtmlModalHelp::wxHtmlModalHelp(wxWindow* parent, const std::string& helpFile, const std::string& topic, unsigned int style)
{
    // Force some mandatory styles
    style |= wxHF_DIALOG | wxHF_MODAL;

    wxHtmlHelpController controller(style, parent);
    controller.Initialize(helpFile);

    if (topic.empty())
        controller.DisplayContents();
    else
        controller.DisplaySection(topic);
}

#endif // wxUSE_WXHTML_HELP

