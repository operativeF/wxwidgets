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
#include "wx/app.h"

import <string>;

static std::string GetLabelForSysLink(std::string_view text, const std::string& url)
{
    // Any "&"s in the text should appear on the screen and not be (mis)
    // interpreted as mnemonics.
    return fmt::format("<A HREF=\"{:s}\">{:s}</A>",
        url,
        wxControl::EscapeMnemonics(text));
}

// ----------------------------------------------------------------------------
// wxHyperlinkCtrl
// ----------------------------------------------------------------------------

class wxHyperlinkCtrl : public wxGenericHyperlinkCtrl
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
                    std::string_view name = wxHyperlinkCtrlNameStr)
    {
        CreateControl(parent, id, pos, size, style, wxDefaultValidator, name);

        SetURL( url );
        SetVisited( false );

        DWORD exstyle;
        DWORD msStyle = MSWGetStyle(style, &exstyle);

        // "SysLink" would be WC_LINK but it's a wide-string
        MSWCreateControl("SysLink", msStyle, pos, size, GetLabelForSysLink( label, url ), exstyle);

        // Make sure both the label and URL are non-empty strings.
        SetURL(url.empty() ? label : url);
        SetLabel(label.empty() ? url : label);

        ConnectMenuHandlers();
    }

    // overridden base class methods
    // -----------------------------

    void SetURL(const std::string& url) override;

    void SetLabel(std::string_view label) override;

protected:
    DWORD MSWGetStyle(unsigned int style, DWORD *exstyle) const override;
    wxSize DoGetBestClientSize() const override;

private:
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;

    wxDECLARE_DYNAMIC_CLASS( wxHyperlinkCtrl );
};

#endif // _WX_MSW_HYPERLINK_H_
