///////////////////////////////////////////////////////////////////////////////
// Name:        wx/ribbon/page.h
// Purpose:     Container for ribbon-bar-style interface panels
// Author:      Peter Cawley
// Modified by:
// Created:     2009-05-25
// Copyright:   (C) Peter Cawley
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RIBBON_PAGE_H_
#define _WX_RIBBON_PAGE_H_

#if wxUSE_RIBBON

#include "wx/ribbon/control.h"
#include "wx/ribbon/panel.h"
#include "wx/bitmap.h"

import WX.Cfg.Flags;

class wxRibbonBar;
class wxRibbonPageScrollButton;

class wxRibbonPage : public wxRibbonControl
{
public:
    wxRibbonPage() = default;

    wxRibbonPage(wxRibbonBar* parent,
                 wxWindowID id = wxID_ANY,
                 const wxString& label = {},
                 const wxBitmap& icon = wxNullBitmap,
                 unsigned int style = 0);

    ~wxRibbonPage();

    bool Create(wxRibbonBar* parent,
                wxWindowID id = wxID_ANY,
                const wxString& label = {},
                const wxBitmap& icon = wxNullBitmap,
                unsigned int style = 0);

    void SetArtProvider(wxRibbonArtProvider* art) override;

    wxBitmap& GetIcon() {return m_icon;}
    wxSize GetMinSize() const override;
    void SetSizeWithScrollButtonAdjustment(int x, int y, int width, int height);
    void AdjustRectToIncludeScrollButtons(wxRect* rect) const;

    bool DismissExpandedPanel();

    bool Realize() override;
    bool Show(bool show = true) override;
    bool Layout() override;
    bool ScrollLines(int lines) override;
    bool ScrollPixels(int pixels);
    bool ScrollSections(int sections);

    wxOrientation GetMajorAxis() const;

    void RemoveChild(wxWindowBase *child) override;

    void HideIfExpanded();

protected:
    wxSize DoGetBestSize() const override;
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;
    bool DoActualLayout();
    void OnEraseBackground(wxEraseEvent& evt);
    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);

    bool ExpandPanels(wxOrientation direction, int maximum_amount);
    bool CollapsePanels(wxOrientation direction, int minimum_amount);
    bool ShowScrollButtons();
    void HideScrollButtons();

    void CommonInit(const wxString& label, const wxBitmap& icon);
    void PopulateSizeCalcArray(wxSize (wxWindow::*get_size)() const);

    wxArrayRibbonControl m_collapse_stack;
    wxBitmap m_icon;
    wxSize m_old_size;
    // NB: Scroll button windows are siblings rather than children (to get correct clipping of children)
    wxRibbonPageScrollButton* m_scroll_left_btn{nullptr};
    wxRibbonPageScrollButton* m_scroll_right_btn{nullptr};
    wxSize* m_size_calc_array{nullptr};
    size_t m_size_calc_array_size{};
    int m_scroll_amount{};
    int m_scroll_amount_limit{};
    int m_size_in_major_axis_for_children{};
    bool m_scroll_buttons_visible{false};

#ifndef SWIG
    wxDECLARE_CLASS(wxRibbonPage);
    wxDECLARE_EVENT_TABLE();
#endif
};

#endif // wxUSE_RIBBON

#endif // _WX_RIBBON_PAGE_H_
