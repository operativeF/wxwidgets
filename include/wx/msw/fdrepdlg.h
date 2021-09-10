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

#include <string>

// ----------------------------------------------------------------------------
// wxFindReplaceDialog: dialog for searching / replacing text
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxFindReplaceDialog : public wxFindReplaceDialogBase
{
public:
    
    wxFindReplaceDialog() = default;
    wxFindReplaceDialog(wxWindow *parent,
                        wxFindReplaceData *data,
                        const std::string &title,
                        int style = 0);

    wxFindReplaceDialog(const wxFindReplaceDialog&) = delete;
    wxFindReplaceDialog& operator=(const wxFindReplaceDialog&) = delete;
    wxFindReplaceDialog(wxFindReplaceDialog&&) = default;
    wxFindReplaceDialog& operator=(wxFindReplaceDialog&&) = default;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxFindReplaceData *data,
                const std::string &title,
                int style = 0);

    ~wxFindReplaceDialog();

    // implementation only from now on

    wxFindReplaceDialogImpl *GetImpl() const { return m_impl; }

    // override some base class virtuals
    bool Show(bool show = true) override;
    void SetTitle( const std::string& title) override;
    std::string GetTitle() const override;

protected:
    wxSize DoGetSize() const override;
    wxSize DoGetClientSize() const override;
    void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO) override;

private:
    std::string                m_title;

    wxFindReplaceDialogImpl* m_impl{nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxFindReplaceDialog);
};

#endif // _WX_MSW_FDREPDLG_H_
