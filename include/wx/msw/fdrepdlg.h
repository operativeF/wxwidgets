/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/fdrepdlg.h
// Purpose:     wxFindReplaceDialog class
// Author:      Markus Greither
// Modified by: 31.07.01: VZ: integrated into wxWidgets
// Created:     23/03/2001
// Copyright:   (c) Markus Greither
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_FDREPDLG_H_
#define _WX_MSW_FDREPDLG_H_

// ----------------------------------------------------------------------------
// wxFindReplaceDialog: dialog for searching / replacing text
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFindReplaceDialog : public wxFindReplaceDialogBase
{
public:
    
    wxFindReplaceDialog() = default;
    wxFindReplaceDialog(wxWindow *parent,
                        wxFindReplaceData *data,
                        const wxString &title,
                        int style = 0);

    wxFindReplaceDialog(const wxFindReplaceDialog&) = delete;
	wxFindReplaceDialog& operator=(const wxFindReplaceDialog&) = delete;

    bool Create(wxWindow *parent,
                wxFindReplaceData *data,
                const wxString &title,
                int style = 0);

    ~wxFindReplaceDialog() override;

    // implementation only from now on

    wxFindReplaceDialogImpl *GetImpl() const { return m_impl; }

    // override some base class virtuals
    bool Show(bool show = true) override;
    void SetTitle( const wxString& title) override;
    wxString GetTitle() const override;

protected:
    void DoGetSize(int *width, int *height) const override;
    void DoGetClientSize(int *width, int *height) const override;
    void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO) override;

    wxString                m_title;

    wxFindReplaceDialogImpl *m_impl {nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxFindReplaceDialog);
};

#endif // _WX_MSW_FDREPDLG_H_
