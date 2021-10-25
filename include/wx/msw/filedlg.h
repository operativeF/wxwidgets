/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/filedlg.h
// Purpose:     wxFileDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEDLG_H_
#define _WX_FILEDLG_H_

#include "wx/geometry/rect.h"

#include <string>
#include <vector>

//-------------------------------------------------------------------------
// wxFileDialog
//-------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFileDialog: public wxFileDialogBase
{
public:
    wxFileDialog(wxWindow *parent,
                 const std::string& message = wxFileSelectorPromptStr,
                 const std::string& defaultDir = {},
                 const std::string& defaultFile = {},
                 const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                 unsigned int style = wxFD_DEFAULT_STYLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& sz = wxDefaultSize,
                 const std::string& name = wxFileDialogNameStr);

    wxFileDialog& operator=(wxFileDialog&&) = delete;

    std::vector<wxString> GetPaths() const override;
    std::vector<wxString> GetFilenames() const override;
    bool SupportsExtraControl() const override { return true; }
    void MSWOnInitDialogHook(WXHWND hwnd);

    int ShowModal() override;

    // wxMSW-specific implementation from now on
    // -----------------------------------------

    // called from the hook procedure on CDN_INITDONE reception
    virtual void MSWOnInitDone(WXHWND hDlg);

    // called from the hook procedure on CDN_SELCHANGE.
    void MSWOnSelChange(WXHWND hDlg);

    // called from the hook procedure on CDN_TYPECHANGE.
    void MSWOnTypeChange(WXHWND hDlg, int nFilterIndex);

protected:

    void DoMoveWindow(wxRect boundary) override;
    void DoCentre(unsigned int dir) override;
    wxSize DoGetSize() const override;
    wxPoint DoGetPosition() const override;

private:
    std::vector<wxString> m_fileNames;

    // remember if our SetPosition() or Centre() (which requires special
    // treatment) was called
    bool m_bMovedWindow{false};
    unsigned int m_centreDir{};        // nothing to do if 0

    wxDECLARE_DYNAMIC_CLASS(wxFileDialog);
};

#endif // _WX_FILEDLG_H_

