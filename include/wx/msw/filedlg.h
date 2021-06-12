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

//-------------------------------------------------------------------------
// wxFileDialog
//-------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFileDialog: public wxFileDialogBase
{
public:
    wxFileDialog(wxWindow *parent,
                 const wxString& message = wxASCII_STR(wxFileSelectorPromptStr),
                 const wxString& defaultDir = wxEmptyString,
                 const wxString& defaultFile = wxEmptyString,
                 const wxString& wildCard = wxASCII_STR(wxFileSelectorDefaultWildcardStr),
                 long style = wxFD_DEFAULT_STYLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& sz = wxDefaultSize,
                 const wxString& name = wxASCII_STR(wxFileDialogNameStr));

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

    void DoMoveWindow(int x, int y, int width, int height) override;
    void DoCentre(int dir) override;
    wxSize DoGetSize() const override;
    wxPoint DoGetPosition() const override;

private:
    std::vector<wxString> m_fileNames;

    // remember if our SetPosition() or Centre() (which requires special
    // treatment) was called
    bool m_bMovedWindow{false};
    int m_centreDir{0};        // nothing to do if 0

    wxDECLARE_DYNAMIC_CLASS(wxFileDialog);
    wxFileDialog(const wxFileDialog&) = delete;
	wxFileDialog& operator=(const wxFileDialog&) = delete;
};

#endif // _WX_FILEDLG_H_

