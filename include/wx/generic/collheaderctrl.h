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

class WXDLLIMPEXP_CORE wxGenericCollapsibleHeaderCtrl
    : public wxCollapsibleHeaderCtrlBase
{
public:
    wxGenericCollapsibleHeaderCtrl() { 
    m_collapsed = true;
    m_inWindow = false;
    m_mouseDown = false;
 }

    wxGenericCollapsibleHeaderCtrl(wxWindow *parent,
        wxWindowID id,
        const std::string& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        const std::string& name = wxCollapsibleHeaderCtrlNameStr)
    {
        
    m_collapsed = true;
    m_inWindow = false;
    m_mouseDown = false;


        Create(parent, id, label, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent,
        wxWindowID id,
        const std::string& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        const std::string& name = wxCollapsibleHeaderCtrlNameStr);

    void SetCollapsed(bool collapsed = true) override;

    bool IsCollapsed() const override
        { return m_collapsed; }

protected:

    wxSize DoGetBestClientSize() const override;

private:
    bool m_collapsed;
    bool m_inWindow;
    bool m_mouseDown;

    

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

    wxGenericCollapsibleHeaderCtrl(const wxGenericCollapsibleHeaderCtrl&) = delete;
	wxGenericCollapsibleHeaderCtrl& operator=(const wxGenericCollapsibleHeaderCtrl&) = delete;
};


#endif // _WX_GENERIC_COLLAPSIBLEHEADER_CTRL_H_
