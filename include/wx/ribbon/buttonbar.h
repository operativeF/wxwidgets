///////////////////////////////////////////////////////////////////////////////
// Name:        wx/ribbon/buttonbar.h
// Purpose:     Ribbon control similar to a tool bar
// Author:      Peter Cawley
// Modified by:
// Created:     2009-07-01
// Copyright:   (C) Peter Cawley
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////
#ifndef _WX_RIBBON_BUTTON_BAR_H_
#define _WX_RIBBON_BUTTON_BAR_H_

#if wxUSE_RIBBON

#include "wx/ribbon/art.h"
#include "wx/ribbon/control.h"

import WX.Cfg.Flags;

class wxRibbonButtonBarButtonBase;
class wxRibbonButtonBarLayout;
struct wxRibbonButtonBarButtonInstance;

using wxArrayRibbonButtonBarLayout = std::vector<wxRibbonButtonBarLayout*>;
using wxArrayRibbonButtonBarButtonBase = std::vector<wxRibbonButtonBarButtonBase*>;

class wxRibbonButtonBar : public wxRibbonControl
{
public:
    wxRibbonButtonBar();

    wxRibbonButtonBar(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  unsigned int style = 0);

    ~wxRibbonButtonBar();

    bool Create(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0);

    virtual wxRibbonButtonBarButtonBase* AddButton(
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string,
                wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL);
    // NB: help_string cannot be optional as that would cause the signature
    // to be identical to the full version of AddButton when 3 arguments are
    // given.

    virtual wxRibbonButtonBarButtonBase* AddDropdownButton(
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* AddHybridButton(
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* AddToggleButton(
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* AddButton(
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxBitmap& bitmap_small = wxNullBitmap,
                const wxBitmap& bitmap_disabled = wxNullBitmap,
                const wxBitmap& bitmap_small_disabled = wxNullBitmap,
                wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* InsertButton(
                size_t pos,
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string,
                wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL);

    virtual wxRibbonButtonBarButtonBase* InsertDropdownButton(
                size_t pos,
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* InsertHybridButton(
                size_t pos,
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* InsertToggleButton(
                size_t pos,
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxString& help_string = {});

    virtual wxRibbonButtonBarButtonBase* InsertButton(
                size_t pos,
                int button_id,
                const wxString& label,
                const wxBitmap& bitmap,
                const wxBitmap& bitmap_small = wxNullBitmap,
                const wxBitmap& bitmap_disabled = wxNullBitmap,
                const wxBitmap& bitmap_small_disabled = wxNullBitmap,
                wxRibbonButtonKind kind = wxRIBBON_BUTTON_NORMAL,
                const wxString& help_string = {});

    void SetItemClientObject(wxRibbonButtonBarButtonBase* item, wxClientData* data);
    wxClientData* GetItemClientObject(const wxRibbonButtonBarButtonBase* item) const;
    void SetItemClientData(wxRibbonButtonBarButtonBase* item, void* data);
    void* GetItemClientData(const wxRibbonButtonBarButtonBase* item) const;

    virtual size_t GetButtonCount() const;
    virtual wxRibbonButtonBarButtonBase *GetItem(size_t n) const;
    virtual wxRibbonButtonBarButtonBase *GetItemById(int id) const;
    virtual int GetItemId(wxRibbonButtonBarButtonBase *button) const;


    bool Realize() override;
    virtual void ClearButtons();
    virtual bool DeleteButton(int button_id);
    virtual void EnableButton(int button_id, bool enable = true);
    virtual void ToggleButton(int button_id, bool checked);

    virtual void SetButtonIcon(
                int button_id,
                const wxBitmap& bitmap,
                const wxBitmap& bitmap_small = wxNullBitmap,
                const wxBitmap& bitmap_disabled = wxNullBitmap,
                const wxBitmap& bitmap_small_disabled = wxNullBitmap);

    virtual void SetButtonText(int button_id, const wxString& label);
    virtual void SetButtonTextMinWidth(int button_id,
                int min_width_medium, int min_width_large);
    virtual void SetButtonTextMinWidth(int button_id, const wxString& label);
    virtual void SetButtonMinSizeClass(int button_id,
                wxRibbonButtonBarButtonState min_size_class);
    virtual void SetButtonMaxSizeClass(int button_id,
                wxRibbonButtonBarButtonState max_size_class);

    virtual wxRibbonButtonBarButtonBase *GetActiveItem() const;
    virtual wxRibbonButtonBarButtonBase *GetHoveredItem() const;

    void SetArtProvider(wxRibbonArtProvider* art) override;
    bool IsSizingContinuous() const override;

    wxSize GetMinSize() const override;

    void SetShowToolTipsForDisabled(bool show);
    bool GetShowToolTipsForDisabled() const;

protected:
    friend class wxRibbonButtonBarEvent;
    wxSize DoGetBestSize() const override;
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    void OnEraseBackground(wxEraseEvent& evt);
    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnMouseEnter(wxMouseEvent& evt);
    void OnMouseLeave(wxMouseEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
    void OnMouseUp(wxMouseEvent& evt);

    wxSize DoGetNextSmallerSize(wxOrientation direction,
                                      wxSize relative_to) const override;
    wxSize DoGetNextLargerSize(wxOrientation direction,
                                     wxSize relative_to) const override;

    void CommonInit(unsigned int style);
    void MakeLayouts();
    void TryCollapseLayout(wxRibbonButtonBarLayout* original,
                     size_t first_btn, size_t* last_button,
                     wxRibbonButtonBarButtonState target_size);
    void FetchButtonSizeInfo(wxRibbonButtonBarButtonBase* button,
        wxRibbonButtonBarButtonState size, wxDC& dc);
    void UpdateWindowUI(unsigned int flags) override;

    wxArrayRibbonButtonBarLayout m_layouts;
    wxArrayRibbonButtonBarButtonBase m_buttons;
    wxRibbonButtonBarButtonInstance* m_hovered_button;
    wxRibbonButtonBarButtonInstance* m_active_button;

    wxPoint m_layout_offset;
    wxSize m_bitmap_size_large;
    wxSize m_bitmap_size_small;
    int m_current_layout;
    bool m_layouts_valid{false};
    bool m_lock_active_state;
    bool m_show_tooltips_for_disabled;

#ifndef SWIG
    wxDECLARE_CLASS(wxRibbonButtonBar);
    wxDECLARE_EVENT_TABLE();
#endif
};

class wxRibbonButtonBarEvent : public wxCommandEvent
{
public:
    wxRibbonButtonBarEvent(wxEventType command_type = wxEVT_NULL,
                       int win_id = 0,
                       wxRibbonButtonBar* bar = nullptr,
                       wxRibbonButtonBarButtonBase* button = nullptr)
        : wxCommandEvent(command_type, win_id)
        , m_bar(bar), m_button(button)
    {
    }
    wxEvent *Clone() const override { return new wxRibbonButtonBarEvent(*this); }

    wxRibbonButtonBar* GetBar() {return m_bar;}
    wxRibbonButtonBarButtonBase *GetButton() { return m_button; }
    void SetBar(wxRibbonButtonBar* bar) {m_bar = bar;}
    void SetButton(wxRibbonButtonBarButtonBase* button) { m_button = button; }
    bool PopupMenu(wxMenu* menu);

protected:
    wxRibbonButtonBar* m_bar;
    wxRibbonButtonBarButtonBase *m_button;

#ifndef SWIG
private:
    public:
	wxRibbonButtonBarEvent& operator=(const wxRibbonButtonBarEvent&) = delete;
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
#endif
};

#ifndef SWIG

wxDECLARE_EVENT(wxEVT_RIBBONBUTTONBAR_CLICKED, wxRibbonButtonBarEvent);
wxDECLARE_EVENT(wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, wxRibbonButtonBarEvent);

typedef void (wxEvtHandler::*wxRibbonButtonBarEventFunction)(wxRibbonButtonBarEvent&);

#define wxRibbonButtonBarEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxRibbonButtonBarEventFunction, func)

#define EVT_RIBBONBUTTONBAR_CLICKED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_RIBBONBUTTONBAR_CLICKED, winid, wxRibbonButtonBarEventHandler(fn))
#define EVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, winid, wxRibbonButtonBarEventHandler(fn))
#else

// wxpython/swig event work
%constant wxEventType wxEVT_RIBBONBUTTONBAR_CLICKED;
%constant wxEventType wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED;

%pythoncode {
    EVT_RIBBONBUTTONBAR_CLICKED = wx.PyEventBinder( wxEVT_RIBBONBUTTONBAR_CLICKED, 1 )
    EVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED = wx.PyEventBinder( wxEVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, 1 )
}
#endif

#endif // wxUSE_RIBBON

#endif // _WX_RIBBON_BUTTON_BAR_H_
