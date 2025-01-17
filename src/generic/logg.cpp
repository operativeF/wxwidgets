/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/logg.cpp
// Purpose:     wxLog-derived classes which need GUI support (the rest is in
//              src/common/log.cpp)
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.09.99 (extracted from src/common/log.cpp)
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/app.h"
#include "wx/button.h"
#include "wx/display.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/menu.h"
#include "wx/frame.h"
#include "wx/filedlg.h"
#include "wx/msgdlg.h"
#include "wx/textctrl.h"
#include "wx/statbmp.h"
#include "wx/wxcrtvararg.h"

#if wxUSE_LOGGUI || wxUSE_LOGWINDOW

#include "wx/clipbrd.h"
#include "wx/dataobj.h"
#include "wx/statline.h"
#include "wx/artprov.h"
#include "wx/collpane.h"
#include "wx/msgout.h"
#include "wx/scopeguard.h"

#ifdef  __WXMSW__
    // for OutputDebugString()
    #include  "wx/msw/private.h"
#endif // Windows


#if wxUSE_LOG_DIALOG
    #include "wx/listctrl.h"
    #include "wx/imaglist.h"
#endif // wxUSE_LOG_DIALOG/!wxUSE_LOG_DIALOG

import WX.Cmn.Time;
import WX.Cmn.TextFile;
import WX.Core.Sizer;
import WX.File.File;

import WX.Utils.Settings;

import WX.Image;

import <string>;
import <string_view>;

#define CAN_SAVE_FILES (wxUSE_FILE && wxUSE_FILEDLG)

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

#if wxUSE_LOG_DIALOG

// this function is a wrapper around strftime(3)
// allows to exclude the usage of wxDateTime
static std::string TimeStamp(std::string_view format, time_t t)
{
    char buf[4096];
    struct tm tm;
    if ( !wxStrftime(buf, WXSIZEOF(buf), format, wxLocaltime_r(&t, &tm)) )
    {
        // buffer is too small?
        wxFAIL_MSG("strftime() failed");
    }
    return {buf};
}


class wxLogDialog : public wxDialog
{
public:
    wxLogDialog(wxWindow *parent,
                const std::vector<std::string>& messages,
                const std::vector<int>& severity,
                const std::vector<long>& timess,
                std::string_view caption,
                unsigned int style);
    ~wxLogDialog();

	wxLogDialog& operator=(wxLogDialog&&) = delete;

    // event handlers
    void OnOk(wxCommandEvent& event);
#if wxUSE_CLIPBOARD
    void OnCopy(wxCommandEvent& event);
#endif // wxUSE_CLIPBOARD
#if CAN_SAVE_FILES
    void OnSave(wxCommandEvent& event);
#endif // CAN_SAVE_FILES
    void OnListItemActivated(wxListEvent& event);

private:
    // create controls needed for the details display
    void CreateDetailsControls(wxWindow *);

    // if necessary truncates the given string and adds an ellipsis
    std::string EllipsizeString(std::string text)
    {
        if (ms_maxLength > 0 && text.length() > ms_maxLength)
        {
            std::string ret{text};
            ret.substr(0, ms_maxLength);
            ret += "...";
            return ret;
        }

        return text;
    }

#if CAN_SAVE_FILES || wxUSE_CLIPBOARD
    // return the contents of the dialog as a multiline string
    std::string GetLogMessages() const;
#endif // CAN_SAVE_FILES || wxUSE_CLIPBOARD


    // the data for the listctrl
    std::vector<std::string> m_messages;
    std::vector<int> m_severity;
    std::vector<long> m_times;

    // the controls which are not shown initially (but only when details
    // button is pressed)
    wxListCtrl *m_listctrl;

    // the translated "Details" string
    static std::string ms_details;

    // the maximum length of the log message
    static size_t ms_maxLength;

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxLogDialog, wxDialog)
    EVT_BUTTON(wxID_OK, wxLogDialog::OnOk)
#if wxUSE_CLIPBOARD
    EVT_BUTTON(wxID_COPY,   wxLogDialog::OnCopy)
#endif // wxUSE_CLIPBOARD
#if CAN_SAVE_FILES
    EVT_BUTTON(wxID_SAVE,   wxLogDialog::OnSave)
#endif // CAN_SAVE_FILES
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, wxLogDialog::OnListItemActivated)
wxEND_EVENT_TABLE()

#endif // wxUSE_LOG_DIALOG

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

#if CAN_SAVE_FILES

// pass an uninitialized file object, the function will ask the user for the
// filename and try to open it, returns true on success (file was opened),
// false if file couldn't be opened/created and -1 if the file selection
// dialog was cancelled
static int OpenLogFile(wxFile& file, std::string* filename = nullptr, wxWindow *parent = nullptr);

#endif // CAN_SAVE_FILES

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxLogGui implementation (FIXME MT-unsafe)
// ----------------------------------------------------------------------------

#if wxUSE_LOGGUI

wxLogGui::wxLogGui()
{
    Clear();
}

void wxLogGui::Clear()
{
    m_bErrors =
    m_bWarnings =
    m_bHasMessages = false;

    m_aMessages.clear();
    m_aSeverity.clear();
    m_aTimes.clear();
}

unsigned int wxLogGui::GetSeverityIcon() const
{
    return m_bErrors ? wxICON_STOP
                     : m_bWarnings ? wxICON_EXCLAMATION
                                   : wxICON_INFORMATION;
}

std::string wxLogGui::GetTitle() const
{
    std::string titleFormat;
    switch ( GetSeverityIcon() )
    {
        case wxICON_STOP:
            titleFormat = _("%s Error");
            break;

        case wxICON_EXCLAMATION:
            titleFormat = _("%s Warning");
            break;

        default:
            wxFAIL_MSG( "unexpected icon severity" );
            [[fallthrough]];

        case wxICON_INFORMATION:
            titleFormat = _("%s Information");
    }

    return fmt::format("{{:s}:s}", titleFormat, wxTheApp ? wxTheApp->GetAppDisplayName() : _("Application"));
}

void
wxLogGui::DoShowSingleLogMessage(std::string_view message,
                                 std::string_view title,
                                 unsigned int style)
{
    wxMessageBox(message, title, wxOK | style);
}

void
wxLogGui::DoShowMultipleLogMessages(const std::vector<std::string>& messages,
                                    const std::vector<int>& severities,
                                    const std::vector<long>& times,
                                    std::string_view title,
                                    unsigned int style)
{
#if wxUSE_LOG_DIALOG
    wxLogDialog dlg(nullptr,
                    messages, severities, times,
                    title, style);

    // clear the message list before showing the dialog because while it's
    // shown some new messages may appear
    Clear();

    std::ignore = dlg.ShowModal();
#else // !wxUSE_LOG_DIALOG
    // start from the most recent message
    std::string message;
    const size_t nMsgCount = messages.size();
    message.reserve(nMsgCount*100);
    for ( size_t n = nMsgCount; n > 0; n-- ) {
        message << m_aMessages[n - 1] << "\n";
    }

    DoShowSingleLogMessage(message, title, style);
#endif // wxUSE_LOG_DIALOG/!wxUSE_LOG_DIALOG
}

void wxLogGui::Flush()
{
    wxLog::Flush();

    if ( !m_bHasMessages )
        return;

    // do it right now to block any new calls to Flush() while we're here
    m_bHasMessages = false;

    // note that this must be done before examining m_aMessages as it may log
    // yet another message
    const unsigned int repeatCount = LogLastRepeatIfNeeded();

    const size_t nMsgCount = m_aMessages.size();

    if ( repeatCount > 0 )
    {
        m_aMessages[nMsgCount - 1] += fmt::format(" ({})", m_aMessages[nMsgCount - 2]);
    }

    const std::string title = GetTitle();
    const unsigned int style = GetSeverityIcon();

    // avoid showing other log dialogs until we're done with the dialog we're
    // showing right now: nested modal dialogs make for really bad UI!
    Suspend();

    // and ensure that we allow showing the log again afterwards, even if an
    // exception is thrown
    wxON_BLOCK_EXIT0(wxLog::Resume);

    if ( nMsgCount == 1 )
    {
        // make a copy before calling Clear()
        const std::string message(m_aMessages[0]);
        Clear();

        DoShowSingleLogMessage(message, title, style);
    }
    else // more than one message
    { // FIXME: More efficient way to do this?
        std::vector<std::string> messages;
        std::vector<int> severities;
        std::vector<long> times;

        messages.swap(m_aMessages);
        severities.swap(m_aSeverity);
        times.swap(m_aTimes);

        Clear();

        DoShowMultipleLogMessages(messages, severities, times, title, style);
    }
}

// log all kinds of messages
void wxLogGui::DoLogRecord(wxLogLevel level,
                           std::string_view msg,
                           const wxLogRecordInfo& info)
{
    switch ( level )
    {
        case wxLOG_Info:
        case wxLOG_Message:
            {
                m_aMessages.emplace_back(msg.begin(), msg.end());
                m_aSeverity.push_back(wxLOG_Message);
                m_aTimes.push_back((long)(info.timestampMS / 1000));
                m_bHasMessages = true;
            }
            break;

        case wxLOG_Status:
#if wxUSE_STATUSBAR
            {
                wxFrame *pFrame = nullptr;

                // check if the frame was passed to us explicitly
                wxUIntPtr ptr = 0;
                if ( info.GetNumValue(wxLOG_KEY_FRAME, &ptr) )
                {
                    pFrame = static_cast<wxFrame *>(wxUIntToPtr(ptr));
                }

                // find the top window and set it's status text if it has any
                if ( pFrame == nullptr ) {
                    wxWindow *pWin = wxTheApp->GetTopWindow();
                    if ( dynamic_cast<wxFrame*>(pWin) ) {
                        pFrame = (wxFrame *)pWin;
                    }
                }

                if ( pFrame && pFrame->GetStatusBar() )
                    pFrame->SetStatusText(msg);
            }
#endif // wxUSE_STATUSBAR
            break;

        case wxLOG_Error:
            if ( !m_bErrors ) {
#if !wxUSE_LOG_DIALOG
                // discard earlier informational messages if this is the 1st
                // error because they might not make sense any more and showing
                // them in a message box might be confusing
                m_aMessages.Empty();
                m_aSeverity.Empty();
                m_aTimes.Empty();
#endif // wxUSE_LOG_DIALOG
                m_bErrors = true;
            }
            [[fallthrough]];

        case wxLOG_Warning:
            if ( !m_bErrors ) {
                // for the warning we don't discard the info messages
                m_bWarnings = true;
            }

            m_aMessages.emplace_back(msg.begin(), msg.end());
            m_aSeverity.push_back((int)level);
            m_aTimes.push_back((long)(info.timestampMS / 1000));
            m_bHasMessages = true;
            break;

        case wxLOG_Debug:
        case wxLOG_Trace:
            // let the base class deal with debug/trace messages
            wxLog::DoLogRecord(level, msg, info);
            break;

        case wxLOG_FatalError:
        case wxLOG_Max:
            // fatal errors are shown immediately and terminate the program so
            // we should never see them here
            wxFAIL_MSG("unexpected log level");
            break;

        case wxLOG_Progress:
        case wxLOG_User:
            // just ignore those: passing them to the base class would result
            // in asserts from DoLogText() because DoLogTextAtLevel() would
            // call it as it doesn't know how to handle these levels otherwise
            break;
    }
}

#endif   // wxUSE_LOGGUI

// ----------------------------------------------------------------------------
// wxLogWindow and wxLogFrame implementation
// ----------------------------------------------------------------------------

#if wxUSE_LOGWINDOW

// log frame class
// ---------------
class wxLogFrame : public wxFrame
{
public:
    wxLogFrame(wxWindow *pParent, wxLogWindow *log, const std::string& szTitle);
    ~wxLogFrame();

    wxLogFrame(const wxLogFrame&) = delete;
	wxLogFrame& operator=(const wxLogFrame&) = delete;

    // Don't prevent the application from exiting if just this frame remains.
    bool ShouldPreventAppExit() const override { return false; }

    // menu callbacks
    void OnClose(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
#if CAN_SAVE_FILES
    void OnSave(wxCommandEvent& event);
#endif // CAN_SAVE_FILES
    void OnClear(wxCommandEvent& event);

    // do show the message in the text control
    void ShowLogMessage(std::string_view message)
    {
        auto msgWithBreak = fmt::format("{}\n", message);
        m_pTextCtrl->AppendText(msgWithBreak);
    }

private:
    // use standard ids for our commands!
    enum
    {
        Menu_Close = wxID_CLOSE,
        Menu_Save  = wxID_SAVE,
        Menu_Clear = wxID_CLEAR
    };

    // common part of OnClose() and OnCloseWindow()
    void DoClose();

    wxTextCtrl  *m_pTextCtrl;
    wxLogWindow *m_log;

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxLogFrame, wxFrame)
    // wxLogWindow menu events
    EVT_MENU(Menu_Close, wxLogFrame::OnClose)
#if CAN_SAVE_FILES
    EVT_MENU(Menu_Save,  wxLogFrame::OnSave)
#endif // CAN_SAVE_FILES
    EVT_MENU(Menu_Clear, wxLogFrame::OnClear)

    EVT_CLOSE(wxLogFrame::OnCloseWindow)
wxEND_EVENT_TABLE()

wxLogFrame::wxLogFrame(wxWindow *pParent, wxLogWindow *log, const std::string& szTitle)
          : wxFrame(pParent, wxID_ANY, szTitle),
            m_log(log)
{
    m_pTextCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
            wxDefaultSize,
            wxTE_MULTILINE  |
            wxHSCROLL       |
            // needed for Win32 to avoid 65Kb limit but it doesn't work well
            // when using RichEdit 2.0 which we always do in the Unicode build
            wxTE_READONLY);

#if wxUSE_MENUS
    // create menu
    wxMenuBar *pMenuBar = new wxMenuBar;
    wxMenu *pMenu = new wxMenu;
#if CAN_SAVE_FILES
    pMenu->Append(Menu_Save,  _("Save &As..."), _("Save log contents to file"));
#endif // CAN_SAVE_FILES
    pMenu->Append(Menu_Clear, _("C&lear"), _("Clear the log contents"));
    pMenu->AppendSeparator();
    pMenu->Append(Menu_Close, _("&Close"), _("Close this window"));
    pMenuBar->Append(pMenu, _("&Log"));
    SetMenuBar(pMenuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // status bar for menu prompts
    CreateStatusBar();
#endif // wxUSE_STATUSBAR
}

void wxLogFrame::DoClose()
{
    if ( m_log->OnFrameClose(this) )
    {
        // instead of closing just hide the window to be able to Show() it
        // later
        Show(false);
    }
}

void wxLogFrame::OnClose([[maybe_unused]] wxCommandEvent& event)
{
    DoClose();
}

void wxLogFrame::OnCloseWindow([[maybe_unused]] wxCloseEvent& event)
{
    DoClose();
}

#if CAN_SAVE_FILES
void wxLogFrame::OnSave([[maybe_unused]] wxCommandEvent& event)
{
    std::string filename;
    wxFile file;
    int rc = OpenLogFile(file, &filename, this);
    if ( rc == -1 )
    {
        // cancelled
        return;
    }

    bool bOk = rc != 0;

    // retrieve text and save it
    // -------------------------
    int nLines = m_pTextCtrl->GetNumberOfLines();
    for ( int nLine = 0; bOk && nLine < nLines; nLine++ ) {
        bOk = file.Write(m_pTextCtrl->GetLineText(nLine) +
                         wxTextFile::GetEOL());
    }

    if ( bOk )
        bOk = file.Close();

    if ( !bOk ) {
        wxLogError(_("Can't save log contents to file."));
    }
    else {
        wxLogStatus((wxFrame*)this, _("Log saved to the file '%s'."), filename.c_str());
    }
}
#endif // CAN_SAVE_FILES

void wxLogFrame::OnClear([[maybe_unused]] wxCommandEvent& event)
{
    m_pTextCtrl->Clear();
}

wxLogFrame::~wxLogFrame()
{
    m_log->OnFrameDelete(this);
}

// wxLogWindow
// -----------

wxLogWindow::wxLogWindow(wxWindow *pParent,
                         const std::string& szTitle,
                         bool bShow,
                         bool bDoPass)
    : m_pLogFrame(new wxLogFrame(pParent, this, szTitle))
{
    // FIXME:
    // Initialize it to NULL to ensure that we don't crash if any log messages
    // are generated before the frame is fully created (while this doesn't
    // happen normally, it might, in principle).
    // m_pLogFrame = nullptr;
    // m_pLogFrame = new wxLogFrame(pParent, this, szTitle);

    PassMessages(bDoPass);

    if ( bShow )
        m_pLogFrame->Show();
}

void wxLogWindow::Show(bool bShow)
{
    m_pLogFrame->Show(bShow);
}

void wxLogWindow::DoLogTextAtLevel(wxLogLevel level, std::string_view msg)
{
    if ( !m_pLogFrame )
        return;

    // don't put trace messages in the text window for 2 reasons:
    // 1) there are too many of them
    // 2) they may provoke other trace messages (e.g. wxMSW code uses
    //    wxLogTrace to log Windows messages and adding text to the control
    //    sends more of them) thus sending a program into an infinite loop
    if ( level == wxLOG_Trace )
        return;

    m_pLogFrame->ShowLogMessage(msg);
}

wxFrame *wxLogWindow::GetFrame() const
{
    return m_pLogFrame;
}

bool wxLogWindow::OnFrameClose([[maybe_unused]] wxFrame * frame)
{
    // allow to close
    return true;
}

void wxLogWindow::OnFrameDelete([[maybe_unused]] wxFrame * frame)
{
    m_pLogFrame = nullptr;
}

wxLogWindow::~wxLogWindow()
{
    // may be NULL if log frame already auto destroyed itself
    delete m_pLogFrame;
}

#endif // wxUSE_LOGWINDOW

// ----------------------------------------------------------------------------
// wxLogDialog
// ----------------------------------------------------------------------------

#if wxUSE_LOG_DIALOG

std::string wxLogDialog::ms_details;
size_t wxLogDialog::ms_maxLength = 0;

wxLogDialog::wxLogDialog(wxWindow *parent,
                         const std::vector<std::string>& messages,
                         const std::vector<int>& severity,
                         const std::vector<long>& times,
                         std::string_view caption,
                         unsigned int style)
           : wxDialog(parent, wxID_ANY, caption,
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    // init the static variables:

    if ( ms_details.empty() )
    {
        // ensure that we won't loop here if wxGetTranslation()
        // happens to pop up a Log message while translating this :-)
        ms_details = wxTRANSLATE("&Details");
        ms_details = wxGetTranslation(ms_details);
    }

    if ( ms_maxLength == 0 )
    {
        ms_maxLength = (2 * wxDisplay().GetGeometry().GetSize().x / 3) / wxGetCharWidth();
    }

    size_t count = messages.size();
    m_messages.reserve(count);
    m_severity.reserve(count);
    m_times.reserve(count);

    for ( size_t n = 0; n < count; n++ )
    {
        m_messages.push_back(messages[n]);
        m_severity.push_back(severity[n]);
        m_times.push_back(times[n]);
    }

    m_listctrl = nullptr;

    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

    // create the controls which are always shown and layout them: we use
    // sizers even though our window is not resizable to calculate the size of
    // the dialog properly
    wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizerAll = new wxBoxSizer(isPda ? wxVERTICAL : wxHORIZONTAL);

    if (!isPda)
    {
        wxStaticBitmap *icon = new wxStaticBitmap
                                   (
                                    this,
                                    wxID_ANY,
                                    wxArtProvider::GetMessageBoxIcon(style)
                                   );
        sizerAll->Add(icon, wxSizerFlags().Centre());
    }

    // create the text sizer with a minimal size so that we are sure it won't be too small
    std::string message = EllipsizeString(messages.back());
    wxSizer *szText = CreateTextSizer(message);
    szText->SetMinSize(wxSize{std::min(300, wxDisplay().GetGeometry().GetSize().x / 3), -1});

    sizerAll->Add(szText, wxSizerFlags(1).Centre().Border(wxDirection::wxLEFT | wxDirection::wxRIGHT));

    wxButton *btnOk = new wxButton(this, wxID_OK);
    sizerAll->Add(btnOk, wxSizerFlags().Centre());

    sizerTop->Add(sizerAll, wxSizerFlags().Expand().Border());


    // add the details pane
#if wxUSE_COLLPANE
    wxCollapsiblePane * const
        collpane = new wxCollapsiblePane(this, wxID_ANY, ms_details);
    sizerTop->Add(collpane, wxSizerFlags(1).Expand().Border());

    wxWindow *win = collpane->GetPane();
#else
    wxPanel* win = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxBORDER_NONE);
#endif
    wxSizer * const paneSz = new wxBoxSizer(wxVERTICAL);

    CreateDetailsControls(win);

    paneSz->Add(m_listctrl, wxSizerFlags(1).Expand().Border(wxTOP));

#if wxUSE_CLIPBOARD || CAN_SAVE_FILES
    wxBoxSizer * const btnSizer = new wxBoxSizer(wxHORIZONTAL);

    wxSizerFlags flagsBtn;
    flagsBtn.Border(wxDirection::wxLEFT);

#if wxUSE_CLIPBOARD
    btnSizer->Add(new wxButton(win, wxID_COPY), flagsBtn);
#endif // wxUSE_CLIPBOARD

#if CAN_SAVE_FILES
    btnSizer->Add(new wxButton(win, wxID_SAVE), flagsBtn);
#endif // CAN_SAVE_FILES

    paneSz->Add(btnSizer, wxSizerFlags().Right().Border(wxTOP|wxBOTTOM));
#endif // wxUSE_CLIPBOARD || CAN_SAVE_FILES

    win->SetSizer(paneSz);
    paneSz->SetSizeHints(win);

    SetSizerAndFit(sizerTop);

    Centre();

    if (isPda)
    {
        // Move up the screen so that when we expand the dialog,
        // there's enough space.
        Move(wxPoint(GetPosition().x, GetPosition().y / 2));
    }
}

void wxLogDialog::CreateDetailsControls(wxWindow *parent)
{
    std::string fmt = wxLog::GetTimestamp();
    bool hasTimeStamp = !fmt.empty();

    // create the list ctrl now
    m_listctrl = new wxListCtrl(parent, wxID_ANY,
                                wxDefaultPosition, wxDefaultSize,
                                wxBORDER_SIMPLE |
                                wxLC_REPORT |
                                wxLC_NO_HEADER |
                                wxLC_SINGLE_SEL);

    // no need to translate these strings as they're not shown to the
    // user anyhow (we use wxLC_NO_HEADER style)
    m_listctrl->InsertColumn(0, "Message");

    if (hasTimeStamp)
        m_listctrl->InsertColumn(1, "Time");

    // prepare the imagelist
    static constexpr int ICON_SIZE = 16;
    wxImageList *imageList = new wxImageList(ICON_SIZE, ICON_SIZE);

    // order should be the same as in the switch below!
    static std::string const icons[] =
    {
        wxART_ERROR,
        wxART_WARNING,
        wxART_INFORMATION
    };

    bool loadedIcons = true;

    for ( size_t icon = 0; icon < WXSIZEOF(icons); icon++ )
    {
        wxBitmap bmp = wxArtProvider::GetBitmap(icons[icon], wxART_MESSAGE_BOX,
                                                wxSize(ICON_SIZE, ICON_SIZE));

        // This may very well fail if there are insufficient colours available.
        // Degrade gracefully.
        if ( !bmp.IsOk() )
        {
            loadedIcons = false;

            break;
        }

        imageList->Add(bmp);
    }

    m_listctrl->SetImageList(imageList, wxIMAGE_LIST_SMALL);

    // fill the listctrl
    size_t count = m_messages.size();
    for ( size_t n = 0; n < count; n++ )
    {
        int image;

        if ( loadedIcons )
        {
            switch ( m_severity[n] )
            {
                case wxLOG_Error:
                    image = 0;
                    break;

                case wxLOG_Warning:
                    image = 1;
                    break;

                default:
                    image = 2;
            }
        }
        else // failed to load images
        {
            image = -1;
        }

        std::string msg = m_messages[n];
        wx::utils::ReplaceAll(msg, "\n", " ");

        msg = EllipsizeString(msg);

        m_listctrl->InsertItem(n, msg, image);

        if (hasTimeStamp)
            m_listctrl->SetItem(n, 1, TimeStamp(fmt, (time_t)m_times[n]));
    }

    // let the columns size themselves
    m_listctrl->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (hasTimeStamp)
        m_listctrl->SetColumnWidth(1, wxLIST_AUTOSIZE);

    // calculate an approximately nice height for the listctrl
    int height = GetCharHeight()*(count + 4);

    // but check that the dialog won't fall fown from the screen
    //
    // we use GetMinHeight() to get the height of the dialog part without the
    // details and we consider that the "Save" button below and the separator
    // line (and the margins around it) take about as much, hence double it
    int heightMax = wxDisplay().GetGeometry().GetSize().y - GetPosition().y - 2*GetMinHeight();

    // we should leave a margin
    heightMax *= 9;
    heightMax /= 10;

    m_listctrl->SetSize(wxSize{wxDefaultCoord, std::min(height, heightMax)});
}

void wxLogDialog::OnListItemActivated(wxListEvent& event)
{
    // show the activated item in a message box
    // This allow the user to correctly display the logs which are longer
    // than the listctrl and thus gets truncated or those which contains
    // newlines.

    // NB: don't do:
    //    std::string str = m_listctrl->GetItemText(event.GetIndex());
    // as there's a 260 chars limit on the items inside a wxListCtrl in wxMSW.
    std::string str = m_messages[event.GetIndex()];

    // wxMessageBox will nicely handle the '\n' in the string (if any)
    // and supports long strings
    wxMessageBox(str, "Log message", wxOK, this);
}

void wxLogDialog::OnOk([[maybe_unused]] wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

#if CAN_SAVE_FILES || wxUSE_CLIPBOARD

std::string wxLogDialog::GetLogMessages() const
{
    std::string fmt = wxLog::GetTimestamp();
    if ( fmt.empty() )
    {
        // use the default format
        fmt = "%c";
    }

    const size_t count = m_messages.size();

    std::string text;
    text.reserve(count*m_messages[0].length());
    for ( size_t n = 0; n < count; n++ )
    {
        text += fmt::format("{}:{}{}", TimeStamp(fmt, (time_t)m_times[n]), m_messages[n], wxTextFile::GetEOL());
    }

    return text;
}

#endif // CAN_SAVE_FILES || wxUSE_CLIPBOARD

#if wxUSE_CLIPBOARD

void wxLogDialog::OnCopy([[maybe_unused]] wxCommandEvent& event)
{
    wxClipboardLocker clip;
    if ( !clip ||
            !wxTheClipboard->AddData(new wxTextDataObject(GetLogMessages())) )
    {
        wxLogError(_("Failed to copy dialog contents to the clipboard."));
    }
}

#endif // wxUSE_CLIPBOARD

#if CAN_SAVE_FILES

void wxLogDialog::OnSave([[maybe_unused]] wxCommandEvent& event)
{
    wxFile file;
    int rc = OpenLogFile(file, nullptr, this);
    if ( rc == -1 )
    {
        // cancelled
        return;
    }

    if ( !rc || !file.Write(GetLogMessages()) || !file.Close() )
    {
        wxLogError(_("Can't save log contents to file."));
    }
}

#endif // CAN_SAVE_FILES

wxLogDialog::~wxLogDialog()
{
    if ( m_listctrl )
    {
        delete m_listctrl->GetImageList(wxIMAGE_LIST_SMALL);
    }
}

#endif // wxUSE_LOG_DIALOG

#if CAN_SAVE_FILES

// pass an uninitialized file object, the function will ask the user for the
// filename and try to open it, returns true on success (file was opened),
// false if file couldn't be opened/created and -1 if the file selection
// dialog was cancelled
static int OpenLogFile(wxFile& file, std::string* pFilename, wxWindow *parent)
{
    // get the file name
    // -----------------
    std::string filename = wxSaveFileSelector("log", "txt", "log.txt", parent);
    if ( filename.empty() ) {
        // cancelled
        return -1;
    }

    // open file
    // ---------
    bool bOk = true; // suppress warning about it being possible uninitialized
    if ( wxFile::Exists(filename) ) {
        bool bAppend = false;
        std::string strMsg = fmt::format(_("Append log to file '{:s}' (choosing [No] will overwrite it)?"), filename);

        switch ( wxMessageBox(strMsg, _("Question"), wxICON_QUESTION | wxYES_NO | wxCANCEL) ) {
            case wxYES:
                bAppend = true;
                break;

            case wxNO:
                bAppend = false;
                break;

            case wxCANCEL:
                return -1;

            default:
                wxFAIL_MSG(_("invalid message box return value"));
        }

        if ( bAppend ) {
            bOk = file.Open(filename, wxFile::write_append);
        }
        else {
            bOk = file.Create(filename, true /* overwrite */);
        }
    }
    else {
        bOk = file.Create(filename);
    }

    if ( pFilename )
        *pFilename = filename;

    return bOk;
}

#endif // CAN_SAVE_FILES

#endif // !(wxUSE_LOGGUI || wxUSE_LOGWINDOW)

#if wxUSE_LOG && wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxLogTextCtrl implementation
// ----------------------------------------------------------------------------

wxLogTextCtrl::wxLogTextCtrl(wxTextCtrl *pTextCtrl)
    : m_pTextCtrl(pTextCtrl)
{
}

void wxLogTextCtrl::DoLogText(std::string_view msg)
{
    auto msgWithBreak = fmt::format("{}\n", msg);
    m_pTextCtrl->AppendText(msgWithBreak);
}

#endif // wxUSE_LOG && wxUSE_TEXTCTRL
