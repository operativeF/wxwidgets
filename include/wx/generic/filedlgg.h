/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/filedlgg.h
// Purpose:     wxGenericFileDialog
// Author:      Robert Roebling
// Modified by:
// Created:     8/17/99
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEDLGG_H_
#define _WX_FILEDLGG_H_

#include "wx/listctrl.h"
#include "wx/datetime.h"
#include "wx/filefn.h"
#include "wx/artprov.h"
#include "wx/filedlg.h"
#include "wx/generic/filectrlg.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxBitmapButton;
class wxGenericFileCtrl;
class wxGenericFileDialog;
class wxFileCtrlEvent;

//-------------------------------------------------------------------------
// wxGenericFileDialog
//-------------------------------------------------------------------------

class wxGenericFileDialog: public wxFileDialogBase
{
public:
    wxGenericFileDialog() { Init(); }

    wxGenericFileDialog(wxWindow *parent,
                        const std::string& message = wxFileSelectorPromptStr,
                        const std::string& defaultDir = {},
                        const std::string& defaultFile = {},
                        const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                        unsigned int style = wxFD_DEFAULT_STYLE,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& sz = wxDefaultSize,
                        const std::string& name = wxFileDialogNameStr,
                        bool bypassGenericImpl = false );

    bool Create( wxWindow *parent,
                 const std::string& message = wxFileSelectorPromptStr,
                 const std::string& defaultDir = {},
                 const std::string& defaultFile = {},
                 const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                 unsigned int style = wxFD_DEFAULT_STYLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& sz = wxDefaultSize,
                 const std::string& name = wxFileDialogNameStr,
                 bool bypassGenericImpl = false );

    ~wxGenericFileDialog();

    void SetDirectory(const wxString& dir) override
        { m_filectrl->SetDirectory(dir); }
    void SetFilename(const wxString& name) override
        { m_filectrl->SetFilename(name); }
    void SetMessage(const wxString& message) override { SetTitle(message); }
    void SetPath(const wxString& path) override
        { m_filectrl->SetPath(path); }
    void SetFilterIndex(int filterIndex) override
        { m_filectrl->SetFilterIndex(filterIndex); }
    void SetWildcard(const wxString& wildCard) override
        { m_filectrl->SetWildcard(wildCard); }

    wxString GetPath() const override
        {
            wxCHECK_MSG( !HasFlag(wxFD_MULTIPLE), wxString(), "When using wxFD_MULTIPLE, must call GetPaths() instead" );
            return m_filectrl->GetPath();
        }
    std::vector<wxString> GetPaths() const override
        { return m_filectrl->GetPaths(); }
    wxString GetDirectory() const override
        { return m_filectrl->GetDirectory(); }
    wxString GetFilename() const override
        {
            wxCHECK_MSG( !HasFlag(wxFD_MULTIPLE), wxString(), "When using wxFD_MULTIPLE, must call GetFilenames() instead" );
            return m_filectrl->GetFilename();
        }
    void GetFilenames(std::vector<wxString>& files) const override
        { m_filectrl->GetFilenames(files); }
    wxString GetWildcard() const override
        { return m_filectrl->GetWildcard(); }
    int GetFilterIndex() const override
        { return m_filectrl->GetFilterIndex(); }
    bool SupportsExtraControl() const override { return true; }

    // implementation only from now on
    // -------------------------------

    int ShowModal() override;
    bool Show( bool show = true ) override;

    void OnList( wxCommandEvent &event );
    void OnReport( wxCommandEvent &event );
    void OnUp( wxCommandEvent &event );
    void OnHome( wxCommandEvent &event );
    void OnOk( wxCommandEvent &event );
    void OnNew( wxCommandEvent &event );
    void OnFileActivated( wxFileCtrlEvent &event);

private:
    // if true, don't use this implementation at all
    bool m_bypassGenericImpl;

protected:
    // update the state of m_upDirButton and m_newDirButton depending on the
    // currently selected directory
    void OnUpdateButtonsUI(wxUpdateUIEvent& event);

    wxString               m_filterExtension;
    wxGenericFileCtrl     *m_filectrl;
    wxBitmapButton        *m_upDirButton;
    wxBitmapButton        *m_newDirButton;

private:
    void Init();
    wxBitmapButton* AddBitmapButton( wxWindowID winId, const wxArtID& artId,
                                     const wxString& tip, wxSizer *sizer );

    wxDECLARE_EVENT_TABLE();

    // these variables are preserved between wxGenericFileDialog calls
    static long ms_lastViewStyle;     // list or report?
    static bool ms_lastShowHidden;    // did we show hidden files?
};

#ifdef wxHAS_GENERIC_FILEDIALOG

class wxFileDialog: public wxGenericFileDialog
{
public:
    wxFileDialog() = default;

    wxFileDialog(wxWindow *parent,
                 const std::string& message = wxFileSelectorPromptStr,
                 const std::string& defaultDir = {},
                 const std::string& defaultFile = {},
                 const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                 unsigned int style = 0,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize)
          :wxGenericFileDialog(parent, message,
                               defaultDir, defaultFile, wildCard,
                               style,
                               pos, size)
    {
    }
};

#endif // wxHAS_GENERIC_FILEDIALOG

#endif // _WX_FILEDLGG_H_
