/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/stattext.h
// Purpose:     wxStaticText class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STATTEXT_H_
#define _WX_STATTEXT_H_

import Utils.Geometry;
import WX.WinDef;

class wxStaticText : public wxStaticTextBase
{
public:
    wxStaticText() = default;

    wxStaticText(wxWindow *parent,
                 wxWindowID id,
                 std::string_view label,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = 0,
                 std::string_view name = wxStaticTextNameStr)
    {
        Create(parent, id, label, pos, size, style, name);
    }

    wxStaticText& operator=(wxStaticText&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                std::string_view label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxStaticTextNameStr);

    // override some methods to resize the window properly
    void SetLabel(std::string_view label) override;
    bool SetFont( const wxFont &font ) override;

    WXDWORD MSWGetStyle(unsigned int flags, WXDWORD *exstyle = nullptr) const override;

protected:
    // implement/override some base class virtuals
    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;
    wxSize DoGetBestClientSize() const override;

    std::string WXGetVisibleLabel() const override;
    void WXSetVisibleLabel(const std::string& str) override;
};

#endif
    // _WX_STATTEXT_H_
