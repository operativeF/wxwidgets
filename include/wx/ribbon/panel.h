///////////////////////////////////////////////////////////////////////////////
// Name:        wx/ribbon/panel.h
// Purpose:     Ribbon-style container for a group of related tools / controls
// Author:      Peter Cawley
// Modified by:
// Created:     2009-05-25
// Copyright:   (C) Peter Cawley
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////
#ifndef _WX_RIBBON_PANEL_H_
#define _WX_RIBBON_PANEL_H_

#if wxUSE_RIBBON

#include "wx/defs.h"

#include "wx/bitmap.h"
#include "wx/ribbon/control.h"

import WX.Cfg.Flags;

enum wxRibbonPanelOption
{
    wxRIBBON_PANEL_NO_AUTO_MINIMISE = 1 << 0,
    wxRIBBON_PANEL_EXT_BUTTON       = 1 << 3,
    wxRIBBON_PANEL_MINIMISE_BUTTON  = 1 << 4,
    wxRIBBON_PANEL_STRETCH          = 1 << 5,
    wxRIBBON_PANEL_FLEXIBLE         = 1 << 6,

    wxRIBBON_PANEL_DEFAULT_STYLE    = 0
};

class WXDLLIMPEXP_RIBBON wxRibbonPanel : public wxRibbonControl
{
public:
    wxRibbonPanel();

    wxRibbonPanel(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxString& label = {},
                  const wxBitmap& minimised_icon = wxNullBitmap,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  unsigned int style = wxRIBBON_PANEL_DEFAULT_STYLE);

    bool Create(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxString& label = {},
                const wxBitmap& icon = wxNullBitmap,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxRIBBON_PANEL_DEFAULT_STYLE);

    ~wxRibbonPanel();

    wxBitmap& GetMinimisedIcon() {return m_minimised_icon;}
    const wxBitmap& GetMinimisedIcon() const {return m_minimised_icon;}
    bool IsMinimised() const;
    bool IsMinimised(wxSize at_size) const;
    bool IsHovered() const;
    bool IsExtButtonHovered() const;
    bool CanAutoMinimise() const;

    bool ShowExpanded();
    bool HideExpanded();

    void SetArtProvider(wxRibbonArtProvider* art) override;

    bool Realize() override;
    bool Layout() override;
    wxSize GetMinSize() const override;

    bool IsSizingContinuous() const override;

    void AddChild(wxWindowBase *child) override;
    void RemoveChild(wxWindowBase *child) override;

    virtual bool HasExtButton() const;

    wxRibbonPanel* GetExpandedDummy();
    wxRibbonPanel* GetExpandedPanel();

    // Finds the best width and height given the parent's width and height
    wxSize GetBestSizeForParentSize(const wxSize& parentSize) const override;

    long GetFlags() { return m_flags; }

    void HideIfExpanded();

protected:
    wxSize DoGetBestSize() const override;
    virtual wxSize GetPanelSizerBestSize() const;
    wxSize  GetPanelSizerMinSize() const;
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }
    wxSize GetMinNotMinimisedSize() const;

    wxSize DoGetNextSmallerSize(wxOrientation direction,
                                      wxSize relative_to) const override;
    wxSize DoGetNextLargerSize(wxOrientation direction,
                                     wxSize relative_to) const override;

    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;
    void OnSize(wxSizeEvent& evt);
    void OnEraseBackground(wxEraseEvent& evt);
    void OnPaint(wxPaintEvent& evt);
    void OnMouseEnter(wxMouseEvent& evt);
    void OnMouseEnterChild(wxMouseEvent& evt);
    void OnMouseLeave(wxMouseEvent& evt);
    void OnMouseLeaveChild(wxMouseEvent& evt);
    void OnMouseClick(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnKillFocus(wxFocusEvent& evt);
    void OnChildKillFocus(wxFocusEvent& evt);

    void TestPositionForHover(const wxPoint& pos);
    bool ShouldSendEventToDummy(wxEvent& evt);
    bool TryAfter(wxEvent& evt) override;

    void CommonInit(const wxString& label, const wxBitmap& icon, unsigned int style);
    static wxRect GetExpandedPosition(wxRect panel,
                                      wxSize expanded_size,
                                      wxDirection direction);

    wxBitmap m_minimised_icon;
    wxBitmap m_minimised_icon_resized;
    wxSize m_smallest_unminimised_size;
    wxSize m_minimised_size;
    wxDirection m_preferred_expand_direction;
    wxRibbonPanel* m_expanded_dummy{nullptr};
    wxRibbonPanel* m_expanded_panel{nullptr};
    wxWindow* m_child_with_focus;
    long m_flags;
    bool m_minimised;
    bool m_hovered;
    bool m_ext_button_hovered;
    wxRect m_ext_button_rect;

#ifndef SWIG
    wxDECLARE_CLASS(wxRibbonPanel);
    wxDECLARE_EVENT_TABLE();
#endif
};


class WXDLLIMPEXP_RIBBON wxRibbonPanelEvent : public wxCommandEvent
{
public:
    wxRibbonPanelEvent(wxEventType command_type = wxEVT_NULL,
                       int win_id = 0,
                       wxRibbonPanel* panel = nullptr)
        : wxCommandEvent(command_type, win_id)
        , m_panel(panel)
    {
    }
    wxEvent *Clone() const override { return new wxRibbonPanelEvent(*this); }

    wxRibbonPanel* GetPanel() {return m_panel;}
    void SetPanel(wxRibbonPanel* panel) {m_panel = panel;}

protected:
    wxRibbonPanel* m_panel;

#ifndef SWIG
private:
    public:
	wxRibbonPanelEvent& operator=(const wxRibbonPanelEvent&) = delete;
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
#endif
};

#ifndef SWIG

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_RIBBON, wxEVT_RIBBONPANEL_EXTBUTTON_ACTIVATED, wxRibbonPanelEvent);

typedef void (wxEvtHandler::*wxRibbonPanelEventFunction)(wxRibbonPanelEvent&);

#define wxRibbonPanelEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxRibbonPanelEventFunction, func)

#define EVT_RIBBONPANEL_EXTBUTTON_ACTIVATED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_RIBBONPANEL_EXTBUTTON_ACTIVATED, winid, wxRibbonPanelEventHandler(fn))
#else

// wxpython/swig event work
%constant wxEventType wxEVT_RIBBONPANEL_EXTBUTTON_ACTIVATED;

%pythoncode {
    EVT_RIBBONPANEL_EXTBUTTON_ACTIVATED = wx.PyEventBinder( wxEVT_RIBBONPANEL_EXTBUTTON_ACTIVATED, 1 )
}
#endif

#endif // wxUSE_RIBBON

#endif // _WX_RIBBON_PANEL_H_
