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

#include "wx/geometry/rect.h"

#include <string>

// ----------------------------------------------------------------------------
// wxFindReplaceDialog: dialog for searching / replacing text
// ----------------------------------------------------------------------------

class wxFindReplaceDialog : public wxFindReplaceDialogBase
{
public:
    
    wxFindReplaceDialog() = default;
    wxFindReplaceDialog(wxWindow *parent,
                        wxFindReplaceData *data,
                        const std::string &title,
                        unsigned int style = 0);

    wxFindReplaceDialog& operator=(wxFindReplaceDialog&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxFindReplaceData *data,
                const std::string &title,
                unsigned int style = 0);

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
    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;

private:
    std::string                m_title;

    wxFindReplaceDialogImpl* m_impl{nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxFindReplaceDialog);
};

#endif // _WX_MSW_FDREPDLG_H_
