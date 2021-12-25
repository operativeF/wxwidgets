//////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/filedlgg.cpp
// Purpose:     wxGenericFileDialog
// Author:      Robert Roebling
// Modified by:
// Created:     12/12/98
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILEDLG

// NOTE : it probably also supports MAC, untested
#if !defined(__UNIX__) && !defined(__WIN32__)
#error wxGenericFileDialog currently only supports Unix and MSW
#endif

#include "wx/hash.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/msgdlg.h"
#include "wx/bmpbuttn.h"
#include "wx/checkbox.h"
#include "wx/choice.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/filedlg.h"     // wxFD_OPEN, wxFD_SAVE...

#include "wx/longlong.h"
#include "wx/config.h"
#include "wx/imaglist.h"
#include "wx/artprov.h"
#include "wx/filefn.h"
#include "wx/filectrl.h"
#include "wx/generic/filedlgg.h"
#include "wx/debug.h"
#include "wx/modalhook.h"

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif
#if wxUSE_CONFIG
    #include "wx/config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __UNIX__
    #include <dirent.h>
    #include <pwd.h>
    #ifndef __VMS
    # include <grp.h>
    #endif
#endif

#if defined(__UNIX__)
#include <unistd.h>
#endif

import <ctime>;

import WX.Utils.Settings;
import WX.Core.Sizer;

#if defined(WX_WINDOWS)
#define IsTopMostDir(dir)   (dir.empty())
#else
#define IsTopMostDir(dir)   (dir == "/")
#endif

//-----------------------------------------------------------------------------
// wxGenericFileDialog
//-----------------------------------------------------------------------------

#define  ID_LIST_MODE     (wxID_FILEDLGG    )
#define  ID_REPORT_MODE   (wxID_FILEDLGG + 1)
#define  ID_UP_DIR        (wxID_FILEDLGG + 2)
#define  ID_HOME_DIR      (wxID_FILEDLGG + 3)
#define  ID_NEW_DIR       (wxID_FILEDLGG + 4)
#define  ID_FILE_CTRL     (wxID_FILEDLGG + 5)

wxBEGIN_EVENT_TABLE(wxGenericFileDialog,wxDialog)
    EVT_BUTTON(ID_LIST_MODE, wxGenericFileDialog::OnList)
    EVT_BUTTON(ID_REPORT_MODE, wxGenericFileDialog::OnReport)
    EVT_BUTTON(ID_UP_DIR, wxGenericFileDialog::OnUp)
    EVT_BUTTON(ID_HOME_DIR, wxGenericFileDialog::OnHome)
    EVT_BUTTON(ID_NEW_DIR, wxGenericFileDialog::OnNew)
    EVT_BUTTON(wxID_OK, wxGenericFileDialog::OnOk)
    EVT_FILECTRL_FILEACTIVATED(ID_FILE_CTRL, wxGenericFileDialog::OnFileActivated)

    EVT_UPDATE_UI(ID_UP_DIR, wxGenericFileDialog::OnUpdateButtonsUI)
#if defined(WX_WINDOWS)
    EVT_UPDATE_UI(ID_NEW_DIR, wxGenericFileDialog::OnUpdateButtonsUI)
#endif // defined(WX_WINDOWS)
wxEND_EVENT_TABLE()

long wxGenericFileDialog::ms_lastViewStyle = wxLC_LIST;
bool wxGenericFileDialog::ms_lastShowHidden = false;

void wxGenericFileDialog::Init()
{
    m_bypassGenericImpl = false;

    m_filectrl   = NULL;
    m_upDirButton  = NULL;
    m_newDirButton = NULL;
}

wxGenericFileDialog::wxGenericFileDialog(wxWindow *parent,
                           const std::string& message,
                           const std::string& defaultDir,
                           const std::string& defaultFile,
                           const std::string& wildCard,
                           long  style,
                           const wxPoint& pos,
                           const wxSize& sz,
                           const std::string& name,
                           bool  bypassGenericImpl ) : wxFileDialogBase()
{
    Init();
    Create( parent, message, defaultDir, defaultFile, wildCard, style, pos, sz, name, bypassGenericImpl );
}

bool wxGenericFileDialog::Create( wxWindow *parent,
                                  const std::string& message,
                                  const std::string& defaultDir,
                                  const std::string& defaultFile,
                                  const std::string& wildCard,
                                  long  style,
                                  const wxPoint& pos,
                                  const wxSize& sz,
                                  const std::string& name,
                                  bool  bypassGenericImpl )
{
    m_bypassGenericImpl = bypassGenericImpl;

    parent = GetParentForModalDialog(parent, style);

    if (!wxFileDialogBase::Create(parent, message, defaultDir, defaultFile,
                                  wildCard, style, pos, sz, name))
    {
        return false;
    }

    if (m_bypassGenericImpl)
        return true;

    if (!wxDialog::Create( parent, wxID_ANY, message, pos, sz,
                           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | style, name
                           ))
    {
        return false;
    }

#if wxUSE_CONFIG
    if (wxConfig::Get(false))
    {
        wxConfig::Get()->Read("/wxWindows/wxFileDialog/ViewStyle",
                              &ms_lastViewStyle);
        wxConfig::Get()->Read("/wxWindows/wxFileDialog/ShowHidden",
                              &ms_lastShowHidden);
    }
#endif

    if ((m_dir.empty()) || (m_dir == "."))
    {
        m_dir = wxGetCwd();
        if (m_dir.empty())
            m_dir = wxFILE_SEP_PATH;
    }

    const size_t len = m_dir.length();
    if ((len > 1) && (wxEndsWithPathSeparator(m_dir)))
        m_dir.Remove( len-1, 1 );

    m_filterExtension.clear();

    // layout

    const bool is_pda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *buttonsizer = new wxBoxSizer( wxHORIZONTAL );
    AddBitmapButton( ID_LIST_MODE, wxART_LIST_VIEW,
                     _("View files as a list view"), buttonsizer );
    AddBitmapButton( ID_REPORT_MODE, wxART_REPORT_VIEW,
                     _("View files as a detailed view"), buttonsizer );
    buttonsizer->Add( 30, 5, 1 );
    m_upDirButton = AddBitmapButton( ID_UP_DIR, wxART_GO_DIR_UP,
                                     _("Go to parent directory"), buttonsizer );

    AddBitmapButton( ID_HOME_DIR, wxART_GO_HOME,
                     _("Go to home directory"), buttonsizer );
    buttonsizer->Add( 20, 20 );

    m_newDirButton = AddBitmapButton( ID_NEW_DIR, wxART_NEW_DIR,
                                      _("Create new directory"), buttonsizer );

    if (is_pda)
        mainsizer->Add( buttonsizer, wxSizerFlags().Expand() );
    else
        mainsizer->Add( buttonsizer, wxSizerFlags().Expand()
                                                   .Border( wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP ) );

    long style2 = 0;
    if ( HasFdFlag(wxFD_MULTIPLE) )
        style2 |= wxFC_MULTIPLE;

    m_filectrl = new wxGenericFileCtrl( this, ID_FILE_CTRL,
                                        m_dir, defaultFile,
                                        wildCard,
                                        style2,
                                        wxDefaultPosition, wxSize(540,200)
                                        );

    m_filectrl->ShowHidden( ms_lastShowHidden );

    if (ms_lastViewStyle == wxLC_LIST)
    {
        m_filectrl->ChangeToListMode();
    }
    else if (ms_lastViewStyle == wxLC_REPORT)
    {
        m_filectrl->ChangeToReportMode();
    }

    mainsizer->Add(m_filectrl, wxSizerFlags(1).Expand().HorzBorder());

    wxSizer *bsizer = CreateButtonSizer(wxOK | wxCANCEL);
    if ( bsizer )
    {
        if (is_pda)
            mainsizer->Add(bsizer, wxSizerFlags().Expand().Border());
        else
            mainsizer->Add(bsizer, wxSizerFlags().Expand().DoubleBorder());
    }

    SetSizer( mainsizer );

    if (!is_pda)
    {
        mainsizer->SetSizeHints( this );

        Centre( wxBOTH );
    }

    return true;
}

wxGenericFileDialog::~wxGenericFileDialog()
{
    if (!m_bypassGenericImpl)
    {
#if wxUSE_CONFIG
        if (wxConfig::Get(false))
        {
            wxConfig::Get()->Write("/wxWindows/wxFileDialog/ViewStyle",
                                   ms_lastViewStyle);
            wxConfig::Get()->Write("/wxWindows/wxFileDialog/ShowHidden",
                                   ms_lastShowHidden);
        }
#endif
    }
}

wxBitmapButton* wxGenericFileDialog::AddBitmapButton( wxWindowID winId,
                                                      const wxArtID& artId,
                                                      const std::string& tip,
                                                      wxSizer *sizer)
{
    wxBitmapButton *but = new wxBitmapButton(this, winId,
                                         wxArtProvider::GetBitmap(artId, wxART_BUTTON));
    but->SetToolTip(tip);
    sizer->Add(but, wxSizerFlags().Border());
    return but;
}

int wxGenericFileDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    if (CreateExtraControl())
    {
        wxSizer *sizer = GetSizer();
        sizer->Insert(2 /* after m_filectrl */, m_extraControl,
                      wxSizerFlags().Expand().HorzBorder());
        sizer->Fit(this);
    }

    m_filectrl->SetDirectory(m_dir);

    return wxDialog::ShowModal();
}

bool wxGenericFileDialog::Show( bool show )
{
    // Called by ShowModal, so don't repeate the update
#ifndef __WIN32__
    if (show)
    {
        m_filectrl->SetDirectory(m_dir);
    }
#endif

    return wxDialog::Show( show );
}

void wxGenericFileDialog::OnOk( [[maybe_unused]] wxCommandEvent &event )
{
    std::vector<std::string> selectedFiles = m_filectrl->GetPaths();

    if (selectedFiles.empty())
        return;

    const std::string& path = selectedFiles[0];

    if (selectedFiles.size() == 1)
    {
        SetPath(path);
    }

    // check that the file [doesn't] exist if necessary
    if ( HasFdFlag(wxFD_SAVE) && HasFdFlag(wxFD_OVERWRITE_PROMPT) &&
                wxFileExists(path) )
    {
        if ( wxMessageBox
             (
                fmt::format
                (
                    _("File '%s' already exists, do you really want to overwrite it?"),
                    path
                ),
                _("Confirm"),
                wxYES_NO
             ) != wxYES)
            return;
    }
    else if ( HasFdFlag(wxFD_OPEN) && HasFdFlag(wxFD_FILE_MUST_EXIST) &&
                    !wxFileExists(path) )
    {
        wxMessageBox(_("Please choose an existing file."), _("Error"),
                     wxOK | wxICON_ERROR );
        return;
    }

    EndModal(wxID_OK);
}

void wxGenericFileDialog::OnList( [[maybe_unused]] wxCommandEvent &event )
{
    m_filectrl->ChangeToListMode();
    ms_lastViewStyle = wxLC_LIST;
    m_filectrl->GetFileList()->SetFocus();
}

void wxGenericFileDialog::OnReport( [[maybe_unused]] wxCommandEvent &event )
{
    m_filectrl->ChangeToReportMode();
    ms_lastViewStyle = wxLC_REPORT;
    m_filectrl->GetFileList()->SetFocus();
}

void wxGenericFileDialog::OnUp( [[maybe_unused]] wxCommandEvent &event )
{
    m_filectrl->GoToParentDir();
    m_filectrl->GetFileList()->SetFocus();
}

void wxGenericFileDialog::OnHome( [[maybe_unused]] wxCommandEvent &event )
{
    m_filectrl->GoToHomeDir();
    m_filectrl->SetFocus();
}

void wxGenericFileDialog::OnNew( [[maybe_unused]] wxCommandEvent &event )
{
    m_filectrl->GetFileList()->MakeDir();
}

void wxGenericFileDialog::OnFileActivated( [[maybe_unused]] wxFileCtrlEvent &event )
{
    wxCommandEvent dummy;
    OnOk( dummy );
}

void wxGenericFileDialog::OnUpdateButtonsUI(wxUpdateUIEvent& event)
{
    // surprisingly, we can be called before m_filectrl is set in Create() as
    // wxFileCtrl ctor itself can generate idle events, so we need this test
    if ( m_filectrl )
        event.Enable( !IsTopMostDir(m_filectrl->GetShownDirectory()) );
}

#ifdef wxHAS_GENERIC_FILEDIALOG

#endif // wxHAS_GENERIC_FILEDIALOG

#endif // wxUSE_FILEDLG
