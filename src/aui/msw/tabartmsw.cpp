module;

#include "wx/msw/uxtheme.h"

#include "wx/renderer.h"

#include <memory>

module WX.AUI.TabArt;

import <algorithm>;
import <string>;

wxAuiMSWTabArt::wxAuiMSWTabArt()
{
    m_themed = wxUxThemeIsActive();
}

std::unique_ptr<wxAuiTabArt> wxAuiMSWTabArt::Clone()
{
    return std::make_unique<wxAuiMSWTabArt>(*this);
}

void wxAuiMSWTabArt::SetSizingInfo(const wxSize& tab_ctrl_size,
    size_t tab_count)
{
    wxAuiGenericTabArt::SetSizingInfo(tab_ctrl_size, tab_count);
}


void wxAuiMSWTabArt::DrawBorder(wxDC& dc, wxWindow* wnd, const wxRect& rect)
{
    if ( !IsThemed() )
    {
        wxAuiGenericTabArt::DrawBorder(dc, wnd, rect);
        return;
    }

    wxRect drawRect(rect);

    drawRect.y += m_maxTabHeight + wnd->FromDIP(1);
    drawRect.height -= m_maxTabHeight;

    // Mask border not covered by native theme
    wxRect topDrawRect(rect);
    topDrawRect.height = drawRect.height;
    dc.SetPen(wxPen(wnd->GetBackgroundColour(), GetBorderWidth(wnd)));
    dc.DrawRectangle(topDrawRect);

    RECT r;
    wxCopyRectToRECT(drawRect, r);

    wxUxThemeHandle hTheme(wnd, "TAB");

    ::DrawThemeBackground(
        hTheme,
        GetHdcOf(dc.GetTempHDC()),
        TABP_PANE,
        0,
        &r,
        nullptr);
}

void wxAuiMSWTabArt::DrawBackground(wxDC& dc,
    wxWindow* wnd,
    const wxRect& rect)
{
    if ( !IsThemed() )
    {
        wxAuiGenericTabArt::DrawBackground(dc, wnd, rect);
        return;
    }

    int borderHeight = 2;

    wxRect drawRect = rect;
    drawRect.height -= borderHeight;

    // Draw background
    dc.SetBrush(wxBrush(wnd->GetBackgroundColour()));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(drawRect);

    // Draw top border
    drawRect.y = drawRect.height;
    drawRect.height = borderHeight + 2;

    drawRect.Inflate(1, 0);

    RECT r;
    wxCopyRectToRECT(drawRect, r);

    wxUxThemeHandle hTheme(wnd, "TAB");

    ::DrawThemeBackground(
        hTheme,
        GetHdcOf(dc.GetTempHDC()),
        TABP_PANE,
        0,
        &r,
        nullptr);
}

void wxAuiMSWTabArt::DrawTab(wxDC& dc,
    wxWindow* wnd,
    const wxAuiNotebookPage& page,
    const wxRect& in_rect,
    int close_button_state,
    wxRect* out_tab_rect,
    wxRect* out_button_rect,
    int* x_extent)
{
    if ( !IsThemed() )
    {
        wxAuiGenericTabArt::DrawTab(dc, wnd, page, in_rect, close_button_state, out_tab_rect, out_button_rect, x_extent);
        return;
    }

    if ( !m_closeBtnSize.IsFullySpecified() )
        InitSizes(wnd, dc);

    // figure out the size of the tab
    wxSize tabSize = GetTabSize(dc,
        wnd,
        page.caption,
        page.bitmap,
        page.active,
        close_button_state,
        x_extent);

    wxCoord tabHeight = tabSize.y;
    wxCoord tabWidth = tabSize.x;
    wxCoord tabX = in_rect.x;
    wxCoord tabY = 0;

    if (!page.active)
    {
        tabY += wnd->FromDIP(2);
        tabHeight -= wnd->FromDIP(2);
    }
    else
    {
        tabX -= wnd->FromDIP(2);
        tabWidth += wnd->FromDIP(4);
        tabHeight += 2;
    }

    int clipWidth = tabWidth;
    if ( tabX + clipWidth > in_rect.x + in_rect.width )
        clipWidth = (in_rect.x + in_rect.width) - tabX;
    dc.SetClippingRegion(tabX - wnd->FromDIP(2), tabY, clipWidth + wnd->FromDIP(4), tabHeight);


    // draw tab
    wxRect tabRect(tabX, tabY, tabWidth, tabHeight);

    int tabState;
    if ( page.active )
        tabState = TIS_SELECTED;
    else if ( page.hover )
        tabState = TIS_HOT;
    else
        tabState = TIS_NORMAL;

    wxUxThemeHandle hTabTheme(wnd, "TAB");
    RECT tabR;
    wxCopyRectToRECT(tabRect, tabR);
    ::DrawThemeBackground(hTabTheme, GetHdcOf(dc.GetTempHDC()), TABP_TABITEM,
        tabState,
        &tabR, nullptr);

    // Apparently, in at least some Windows 10 installations the call above
    // does not draw the left edge of the first tab and it needs to be drawn
    // separately, or it wouldn't be drawn at all.
    if ( tabX == GetIndentSize() )
    {
        ::DrawThemeBackground
            (
                hTabTheme,
                GetHdcOf(dc.GetTempHDC()),
                TABP_TABITEMLEFTEDGE,
                tabState,
                &tabR,
                nullptr
            );
    }

    wxRect textRect = tabRect;
    if ( !page.active )
        textRect.Offset(0, wnd->FromDIP(1));
    if ( close_button_state != wxAUI_BUTTON_STATE_HIDDEN )
        textRect.width -= m_closeBtnSize.x + wnd->FromDIP(3);

    dc.SetFont(wnd->GetFont());
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    dc.DrawLabel(page.caption, page.bitmap, textRect, wxALIGN_CENTRE);

    // draw focus rectangle
    if ( page.active && (wxWindow::FindFocus() == wnd) )
    {
        wxRect focusRect = tabRect;
        focusRect.Deflate(wnd->FromDIP(2));

        wxRendererNative::Get().DrawFocusRect(wnd, dc, focusRect, 0);
    }

    // draw close button
    if ( close_button_state != wxAUI_BUTTON_STATE_HIDDEN )
    {
        wxUxThemeHandle hToolTipTheme(wnd, "TOOLTIP");

        int btnState{};
        if ( close_button_state == wxAUI_BUTTON_STATE_HOVER )
            btnState = TTCS_HOT;
        else if ( close_button_state == wxAUI_BUTTON_STATE_PRESSED )
            btnState = TTCS_PRESSED;
        else
            btnState = TTCS_NORMAL;

        int offsetY = tabY;
        if ( wxGetWinVersion() < wxWinVersion_Vista )
            offsetY++; // WinXP theme needs a little more padding

        wxRect rect(tabX + tabWidth - m_closeBtnSize.x - wnd->FromDIP(4),
            offsetY + (tabHeight / 2) - (m_closeBtnSize.y / 2),
            m_closeBtnSize.x,
            m_closeBtnSize.y);

        RECT btnR;
        wxCopyRectToRECT(rect, btnR);
        ::DrawThemeBackground(hToolTipTheme, GetHdcOf(dc.GetTempHDC()), TTP_CLOSE, btnState, &btnR, nullptr);

        if ( out_button_rect )
            *out_button_rect = rect;
    }

    *out_tab_rect = wxRect(tabX, tabY, tabWidth, tabHeight);

    dc.DestroyClippingRegion();
}

int wxAuiMSWTabArt::GetIndentSize()
{
    if ( IsThemed() )
        return wxWindow::FromDIP(3, nullptr); // This should be 1 but we can't draw into the border from DrawTab
    else
        return wxAuiGenericTabArt::GetIndentSize();
}

int wxAuiMSWTabArt::GetBorderWidth(wxWindow* wnd)
{
    return wxAuiGenericTabArt::GetBorderWidth(wnd);
}

int wxAuiMSWTabArt::GetAdditionalBorderSpace(wxWindow* wnd)
{
    if ( IsThemed() )
    {
        return wxWindowBase::FromDIP(4, nullptr);
    }
    else
        return wxAuiGenericTabArt::GetAdditionalBorderSpace(wnd);
}

wxSize wxAuiMSWTabArt::GetTabSize(wxDC& dc,
    wxWindow* wnd,
    const std::string& caption,
    const wxBitmap& bitmap,
    bool active,
    int close_button_state,
    int* x_extent)
{
    if ( !IsThemed() )
        return wxAuiGenericTabArt::GetTabSize(dc, wnd, caption, bitmap, active, close_button_state, x_extent);

    if ( !m_closeBtnSize.IsFullySpecified() )
        InitSizes(wnd, dc);

    wxCoord textWidth, textHeight, tmp;

    dc.SetFont(wnd->GetFont());
    dc.GetTextExtent(caption, &textWidth, &tmp);
    dc.GetTextExtent("ABCDEFXj", &tmp, &textHeight);

    wxCoord tabWidth = std::max(m_tabSize.x, textWidth);
    wxCoord tabHeight = std::max(m_tabSize.y, textHeight);

    // if the close button is showing, add space for it
    if ( close_button_state != wxAUI_BUTTON_STATE_HIDDEN )
    {
        tabWidth += m_closeBtnSize.x;
        tabHeight = std::max(tabHeight, m_closeBtnSize.y);
    }

    // if there's a bitmap, add space for it
    if ( bitmap.IsOk() )
    {
        tabWidth += bitmap.GetWidth() + wnd->FromDIP(3); // bitmap padding
        tabHeight = std::max(tabHeight, bitmap.GetHeight() + wnd->FromDIP(2));
    }

    // add padding
    tabWidth += wnd->FromDIP(12);
    tabHeight += wnd->FromDIP(3);

    if ( m_flags & wxAUI_NB_TAB_FIXED_WIDTH )
    {
        tabWidth = m_fixedTabWidth;
    }
    else
    {
        int minTabWidth = wnd->FromDIP(46);
        if (tabWidth < minTabWidth)
            tabWidth = minTabWidth;
    }

    *x_extent = tabWidth;

    if (tabHeight > m_maxTabHeight)
        m_maxTabHeight = tabHeight;

    return {tabWidth, tabHeight};
}


void wxAuiMSWTabArt::DrawButton(wxDC& dc,
    wxWindow* wnd,
    const wxRect& in_rect,
    int bitmap_id,
    int button_state,
    int orientation,
    wxRect* out_rect)
{
    if ( !IsThemed() )
    {
        wxAuiGenericTabArt::DrawButton(dc, wnd, in_rect, bitmap_id, button_state, orientation, out_rect);
        return;
    }

    std::string themeId;
    int part = 0;

    switch (bitmap_id)
    {
    case wxAUI_BUTTON_CLOSE:
        themeId = "Window";
        part = WP_CLOSEBUTTON;
        break;
    case wxAUI_BUTTON_LEFT:
        themeId = "Spin";
        part = SPNP_DOWNHORZ;
        break;
    case wxAUI_BUTTON_RIGHT:
        themeId = "Spin";
        part = SPNP_UPHORZ;
        break;
    case wxAUI_BUTTON_WINDOWLIST:
        themeId = "Combobox";
        part = CP_DROPDOWNBUTTON;
        break;
    }

    wxRect rect = in_rect;

    if ( orientation == wxDirection::wxLEFT )
    {
        rect.SetX(in_rect.x);
        rect.SetY(((in_rect.y + in_rect.height) / 2) - (m_closeBtnSize.y / 2));
        rect.SetWidth(m_closeBtnSize.x);
        rect.SetHeight(m_closeBtnSize.y);
    }
    else
    {
        rect = wxRect(in_rect.x + in_rect.width - m_closeBtnSize.x,
            ((in_rect.y + in_rect.height) / 2) - (m_closeBtnSize.y / 2),
            m_closeBtnSize.x, m_closeBtnSize.y);
    }

    if ( bitmap_id == wxAUI_BUTTON_LEFT ||
        bitmap_id == wxAUI_BUTTON_RIGHT )
    {
        rect.y = in_rect.y;
        rect.height = in_rect.height - wnd->FromDIP(7);
    }

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(m_baseColour));
    dc.DrawRectangle(rect);

    int btnState;
    if ( button_state == wxAUI_BUTTON_STATE_DISABLED )
        btnState = TTCS_PRESSED + 1;
    else if ( button_state == wxAUI_BUTTON_STATE_HOVER )
        btnState = TTCS_HOT;
    else if ( button_state == wxAUI_BUTTON_STATE_PRESSED )
        btnState = TTCS_PRESSED;
    else
        btnState = TTCS_NORMAL;

    wxUxThemeHandle hTheme(wnd, themeId);

    wxRect btnRect(rect);
    btnRect.width -= wnd->FromDIP(1);

    RECT btnR;
    wxCopyRectToRECT(btnRect, btnR);
    ::DrawThemeBackground(hTheme, GetHdcOf(dc.GetTempHDC()), part, btnState, &btnR, nullptr);

    if ( out_rect )
        *out_rect = rect;
}

int wxAuiMSWTabArt::ShowDropDown(wxWindow* wnd,
    const wxAuiNotebookPageArray& pages,
    int active_idx)
{
    return wxAuiGenericTabArt::ShowDropDown(wnd, pages, active_idx);
}

int wxAuiMSWTabArt::GetBestTabCtrlSize(wxWindow* wnd,
    const wxAuiNotebookPageArray& pages,
    const wxSize& requiredBmp_size)
{
    return wxAuiGenericTabArt::GetBestTabCtrlSize(wnd, pages, requiredBmp_size);
}

void wxAuiMSWTabArt::InitSizes(wxWindow* wnd, wxDC& dc)
{
    SIZE uxSize;

    // Borrow close button from tooltip (best fit on various backgrounds)
    wxUxThemeHandle hTooltipTheme(wnd, "Tooltip");

    ::GetThemePartSize(hTooltipTheme, GetHdcOf(dc.GetTempHDC()),
        TTP_CLOSE, 0, nullptr, TS_TRUE, &uxSize);
    m_closeBtnSize.Set(uxSize.cx, uxSize.cy);

    wxUxThemeHandle hTabTheme(wnd, "TAB");
    ::GetThemePartSize(hTabTheme, GetHdcOf(dc.GetTempHDC()),
        TABP_TABITEM, 0, nullptr, TS_TRUE, &uxSize);
    m_tabSize.Set(uxSize.cx, uxSize.cy);
}

bool wxAuiMSWTabArt::IsThemed() const
{
    return
        m_themed &&
        !(m_flags & wxAUI_NB_BOTTOM); // Native theme does not support bottom tabs
}
