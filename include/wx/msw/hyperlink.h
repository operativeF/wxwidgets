/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/hyperlink.h
// Purpose:     Hyperlink control
// Author:      Rickard Westerlund
// Created:     2010-08-04
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_HYPERLINK_H_
#define _WX_MSW_HYPERLINK_H_

#include "wx/generic/hyperlink.h"

#include <string>

// ----------------------------------------------------------------------------
// wxHyperlinkCtrl
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxHyperlinkCtrl : public wxGenericHyperlinkCtrl
{
public:
    // Default constructor (for two-step construction).
    wxHyperlinkCtrl() = default;

    // Constructor.
    wxHyperlinkCtrl(wxWindow *parent,
                    wxWindowID id,
                    const std::string& label,
                    const std::string& url,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = wxHL_DEFAULT_STYLE,
                    const std::string& name = wxHyperlinkCtrlNameStr)
    {
        Create(parent, id, label, url, pos, size, style, name);
    }

    // Creation function (for two-step construction).
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label,
                const std::string& url,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxHL_DEFAULT_STYLE,
                const std::string& name = wxHyperlinkCtrlNameStr);


    // overridden base class methods
    // -----------------------------

    void SetURL(const std::string& url) override;

    void SetLabel(const std::string& label) override;

protected:
    DWORD MSWGetStyle(unsigned int style, DWORD *exstyle) const override;
    wxSize DoGetBestClientSize() const override;

private:
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;

    wxDECLARE_DYNAMIC_CLASS( wxHyperlinkCtrl );
};

#endif // _WX_MSW_HYPERLINK_H_
