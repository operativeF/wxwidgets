/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/dirdlgg.h
// Purpose:     wxGenericDirCtrl class
//              Builds on wxDirCtrl class written by Robert Roebling for the
//              wxFile application, modified by Harm van der Heijden.
//              Further modified for Windows.
// Author:      Robert Roebling, Harm van der Heijden, Julian Smart et al
// Modified by:
// Created:     21/3/2000
// Copyright:   (c) Robert Roebling, Harm van der Heijden, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIRDLGG_H_
#define _WX_DIRDLGG_H_

class wxGenericDirCtrl;
class wxTextCtrl;
class wxTreeEvent;

#ifndef wxDD_DEFAULT_STYLE
#define wxDD_DEFAULT_STYLE      (wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
#endif

#include "wx/dialog.h"

//-----------------------------------------------------------------------------
// wxGenericDirDialog
//-----------------------------------------------------------------------------

class wxGenericDirDialog : public wxDirDialogBase
{
public:
    wxGenericDirDialog() = default;

    wxGenericDirDialog(wxWindow* parent,
                       const std::string& title = wxDirSelectorPromptStr,
                       const std::string& defaultPath = {},
                       unsigned int style = wxDD_DEFAULT_STYLE,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& sz = wxDefaultSize,//Size(450, 550),
                       const std::string& name = wxDirDialogNameStr);

    bool Create(wxWindow* parent,
                const std::string& title = wxDirSelectorPromptStr,
                const std::string& defaultPath = {},
                unsigned int style = wxDD_DEFAULT_STYLE,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& sz = wxDefaultSize,//Size(450, 550),
                const std::string& name = wxDirDialogNameStr);

    //// Accessors
    void SetPath(const wxString& path) override;
    wxString GetPath() const override;

    //// Overrides
    int ShowModal() override;
    void EndModal(int retCode) override;

    // this one is specific to wxGenericDirDialog
    wxTextCtrl* GetInputCtrl() const { return m_input; }

protected:
    //// Event handlers
    void OnCloseWindow(wxCloseEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnTreeSelected(wxTreeEvent &event);
    void OnTreeKeyDown(wxTreeEvent &event);
    void OnNew(wxCommandEvent& event);
    void OnGoHome(wxCommandEvent& event);
    void OnShowHidden(wxCommandEvent& event);

    wxGenericDirCtrl* m_dirCtrl{nullptr};
    wxTextCtrl*       m_input{nullptr};

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_DIRDLGG_H_
