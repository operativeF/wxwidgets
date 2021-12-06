/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/collheaderctrl.h
// Purpose:     wxGenericCollapsibleHeaderCtrl
// Author:      Tobias Taschner
// Created:     2015-09-19
// Copyright:   (c) 2015 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_COLLAPSIBLEHEADER_CTRL_H_
#define _WX_GENERIC_COLLAPSIBLEHEADER_CTRL_H_

import <string>;

class wxGenericCollapsibleHeaderCtrl
    : public wxCollapsibleHeaderCtrlBase
{
public:
    wxGenericCollapsibleHeaderCtrl() = default;

    wxGenericCollapsibleHeaderCtrl(wxWindow *parent,
        wxWindowID id,
        std::string_view label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        std::string_view name = wxCollapsibleHeaderCtrlNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxGenericCollapsibleHeaderCtrl& operator=(wxGenericCollapsibleHeaderCtrl&&) = delete;

    bool Create(wxWindow *parent,
        wxWindowID id,
        std::string_view label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        std::string_view name = wxCollapsibleHeaderCtrlNameStr);

    void SetCollapsed(bool collapsed = true) override;

    bool IsCollapsed() const override
        { return m_collapsed; }

protected:

    wxSize DoGetBestClientSize() const override;

private:
    bool m_collapsed{true};
    bool m_inWindow{false};
    bool m_mouseDown{false};

    void OnPaint(wxPaintEvent& event);

    // Handle set/kill focus events (invalidate for painting focus rect)
    void OnFocus(wxFocusEvent& event);

    // Handle click
    void OnLeftUp(wxMouseEvent& event);

    // Handle pressed state
    void OnLeftDown(wxMouseEvent& event);

    // Handle current state
    void OnEnterWindow(wxMouseEvent& event);

    void OnLeaveWindow(wxMouseEvent& event);

    // Toggle on space
    void OnChar(wxKeyEvent& event);

    void DoSetCollapsed(bool collapsed);
};


#endif // _WX_GENERIC_COLLAPSIBLEHEADER_CTRL_H_
