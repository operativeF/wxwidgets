module;

#include "wx/msw/uxtheme.h"

#include "wx/app.h"

module WX.AUI.ToolBarArt;

wxAuiMSWToolBarArt::wxAuiMSWToolBarArt()
{
    if ( wxUxThemeIsActive() )
    {
        m_themed = true;

        // Determine sizes from theme
        wxWindow* window = dynamic_cast<wxApp*>(wxApp::GetInstance())->GetTopWindow();
        wxUxThemeHandle hTheme(window, "Rebar");

        SIZE overflowSize;
        ::GetThemePartSize(hTheme, nullptr, RP_CHEVRON, 0,
            nullptr, TS_TRUE, &overflowSize);
        m_overflowSize = overflowSize.cx;

        SIZE gripperSize;
        ::GetThemePartSize(hTheme, nullptr, RP_GRIPPER, 0,
            nullptr, TS_TRUE, &gripperSize);
        m_gripperSize = gripperSize.cx;

        wxUxThemeHandle hThemeToolbar(window, "Toolbar");

        SIZE seperatorSize;
        ::GetThemePartSize(hThemeToolbar, nullptr, TP_SEPARATOR, 0,
            nullptr, TS_TRUE, &seperatorSize);
        m_separatorSize = seperatorSize.cx;

        // TP_DROPDOWNBUTTON is only 7px, too small to fit the dropdown arrow,
        // use 14px instead.
        m_dropdownSize = window->FromDIP(14);

        SIZE buttonSize;
        ::GetThemePartSize(hThemeToolbar, nullptr, TP_BUTTON, 0,
            nullptr, TS_TRUE, &buttonSize);
        m_buttonSize.Set(buttonSize.cx, buttonSize.cy);
    }
    else
        m_themed = false;
}

wxAuiToolBarArt* wxAuiMSWToolBarArt::Clone()
{
    return new wxAuiMSWToolBarArt;
}

void wxAuiMSWToolBarArt::DrawBackground(
    wxDC& dc,
    wxWindow* wnd,
    const wxRect& rect)
{
    if ( m_themed )
    {
        RECT r;
        wxCopyRectToRECT(rect, r);

        wxUxThemeHandle hTheme(wnd, "Rebar");

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            RP_BACKGROUND,
            0,
            &r,
            nullptr);
    }
    else
        wxAuiGenericToolBarArt::DrawBackground(dc, wnd, rect);
}

void wxAuiMSWToolBarArt::DrawLabel(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item,
    const wxRect& rect)
{
    wxAuiGenericToolBarArt::DrawLabel(dc, wnd, item, rect);
}

void wxAuiMSWToolBarArt::DrawButton(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item,
    const wxRect& rect)
{
    if ( m_themed )
    {
        RECT r;
        wxCopyRectToRECT(rect, r);

        wxUxThemeHandle hTheme(wnd, "Toolbar");

        int btnState;
        if ( item.GetState() & wxAUI_BUTTON_STATE_DISABLED )
            btnState = TS_DISABLED;
        else if ( item.GetState() & wxAUI_BUTTON_STATE_PRESSED )
            btnState = TS_PRESSED;
        else if ( item.GetState() & wxAUI_BUTTON_STATE_HOVER &&
            item.GetState() & wxAUI_BUTTON_STATE_CHECKED )
            btnState = TS_HOTCHECKED;
        else if ( item.GetState() & wxAUI_BUTTON_STATE_CHECKED )
            btnState = TS_CHECKED;
        else if ( (item.GetState() & wxAUI_BUTTON_STATE_HOVER) || item.IsSticky() )
            btnState = TS_HOT;
        else
            btnState = TS_NORMAL;

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            TP_BUTTON,
            btnState,
            &r,
            nullptr);

        int textWidth = 0, textHeight = 0;

        if ( m_flags & wxAUI_TB_TEXT )
        {
            dc.SetFont(m_font);

            int tx, ty;

            dc.GetTextExtent("ABCDHgj", &tx, &textHeight);
            textWidth = 0;
            dc.GetTextExtent(item.GetLabel(), &textWidth, &ty);
        }

        int bmpX = 0, bmpY = 0;
        int textX = 0, textY = 0;

        if ( m_textOrientation == wxAUI_TBTOOL_TEXT_BOTTOM )
        {
            bmpX = rect.x +
                (rect.width / 2) -
                (item.GetBitmap().GetWidth() / 2);

            bmpY = rect.y +
                ((rect.height - textHeight) / 2) -
                (item.GetBitmap().GetHeight() / 2);

            textX = rect.x + (rect.width / 2) - (textWidth / 2) + 1;
            textY = rect.y + rect.height - textHeight - 1;
        }
        else if ( m_textOrientation == wxAUI_TBTOOL_TEXT_RIGHT )
        {
            bmpX = rect.x + wnd->FromDIP(3);

            bmpY = rect.y +
                (rect.height / 2) -
                (item.GetBitmap().GetHeight() / 2);

            textX = bmpX + wnd->FromDIP(3) + item.GetBitmap().GetWidth();
            textY = rect.y +
                (rect.height / 2) -
                (textHeight / 2);
        }

        wxBitmap bmp;
        if ( item.GetState() & wxAUI_BUTTON_STATE_DISABLED )
            bmp = item.GetDisabledBitmap();
        else
            bmp = item.GetBitmap();

        if ( bmp.IsOk() )
            dc.DrawBitmap(bmp, bmpX, bmpY, true);

        // set the item's text color based on if it is disabled
        if ( item.GetState() & wxAUI_BUTTON_STATE_DISABLED )
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        else
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT));

        if ( (m_flags & wxAUI_TB_TEXT) && !item.GetLabel().empty() )
        {
            dc.wxDrawText(item.GetLabel(), wxPoint{textX, textY});
        }
    }
    else
        wxAuiGenericToolBarArt::DrawButton(dc, wnd, item, rect);
}

void wxAuiMSWToolBarArt::DrawDropDownButton(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item,
    const wxRect& rect)
{
    if ( m_themed )
    {
        wxUxThemeHandle hTheme(wnd, "Toolbar");

        int textWidth = 0, textHeight = 0, textX = 0, textY = 0;
        int bmpX = 0, bmpY = 0;

        wxRect buttonRect = wxRect(rect.x,
            rect.y,
            rect.width - m_dropdownSize,
            rect.height);
        wxRect dropDownRect = wxRect(rect.x + rect.width - m_dropdownSize - 1,
            rect.y,
            m_dropdownSize + 1,
            rect.height);

        if ( m_flags & wxAUI_TB_TEXT )
        {
            dc.SetFont(m_font);

            int tx, ty;
            if ( m_flags & wxAUI_TB_TEXT )
            {
                dc.GetTextExtent("ABCDHgj", &tx, &textHeight);
                textWidth = 0;
            }

            dc.GetTextExtent(item.GetLabel(), &textWidth, &ty);
        }

        RECT btnR;
        wxCopyRectToRECT(buttonRect, btnR);
        RECT dropDownR;
        wxCopyRectToRECT(dropDownRect, dropDownR);

        int btnState;
        if ( item.GetState() & wxAUI_BUTTON_STATE_DISABLED )
            btnState = TS_DISABLED;
        else if ( item.GetState() & wxAUI_BUTTON_STATE_PRESSED )
            btnState = TS_PRESSED;
        else if ( (item.GetState() & wxAUI_BUTTON_STATE_HOVER) || item.IsSticky() )
            btnState = TS_HOT;
        else
            btnState = TS_NORMAL;

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            TP_SPLITBUTTON,
            btnState,
            &btnR,
            nullptr);

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            TP_SPLITBUTTONDROPDOWN,
            btnState,
            &dropDownR,
            nullptr);

        if ( m_textOrientation == wxAUI_TBTOOL_TEXT_BOTTOM )
        {
            bmpX = buttonRect.x +
                (buttonRect.width / 2) -
                (item.GetBitmap().GetWidth() / 2);
            bmpY = buttonRect.y +
                ((buttonRect.height - textHeight) / 2) -
                (item.GetBitmap().GetHeight() / 2);

            textX = rect.x + (rect.width / 2) - (textWidth / 2) + 1;
            textY = rect.y + rect.height - textHeight - 1;
        }
        else if ( m_textOrientation == wxAUI_TBTOOL_TEXT_RIGHT )
        {
            bmpX = rect.x + wnd->FromDIP(3);

            bmpY = rect.y +
                (rect.height / 2) -
                (item.GetBitmap().GetHeight() / 2);

            textX = bmpX + wnd->FromDIP(3) + item.GetBitmap().GetWidth();
            textY = rect.y +
                (rect.height / 2) -
                (textHeight / 2);
        }

        wxBitmap bmp;
        if ( item.GetState() & wxAUI_BUTTON_STATE_DISABLED )
        {
            bmp = item.GetDisabledBitmap();
        }
        else
        {
            bmp = item.GetBitmap();
        }

        if ( !bmp.IsOk() )
            return;

        dc.DrawBitmap(bmp, bmpX, bmpY, true);

        // set the item's text color based on if it is disabled
        if ( item.GetState() & wxAUI_BUTTON_STATE_DISABLED )
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        else
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT));

        if ( (m_flags & wxAUI_TB_TEXT) && !item.GetLabel().empty() )
        {
            dc.wxDrawText(item.GetLabel(), wxPoint{textX, textY});
        }

    }
    else
        wxAuiGenericToolBarArt::DrawDropDownButton(dc, wnd, item, rect);
}

void wxAuiMSWToolBarArt::DrawControlLabel(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item,
    const wxRect& rect)
{
    wxAuiGenericToolBarArt::DrawControlLabel(dc, wnd, item, rect);
}

void wxAuiMSWToolBarArt::DrawSeparator(
    wxDC& dc,
    wxWindow* wnd,
    const wxRect& rect)
{
    if ( m_themed )
    {
        RECT r;
        wxCopyRectToRECT(rect, r);

        wxUxThemeHandle hTheme(wnd, "Toolbar");

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            (m_flags & wxAUI_TB_VERTICAL) ? TP_SEPARATORVERT : TP_SEPARATOR,
            0,
            &r,
            nullptr);
    }
    else
        wxAuiGenericToolBarArt::DrawSeparator(dc, wnd, rect);
}

void wxAuiMSWToolBarArt::DrawGripper(
    wxDC& dc,
    wxWindow* wnd,
    const wxRect& rect)
{
    if ( m_themed )
    {
        RECT r;
        wxCopyRectToRECT(rect, r);

        wxUxThemeHandle hTheme(wnd, "Rebar");

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            (m_flags & wxAUI_TB_VERTICAL) ? RP_GRIPPERVERT : RP_GRIPPER,
            0,
            &r,
            nullptr);
    }
    else
        wxAuiGenericToolBarArt::DrawGripper(dc, wnd, rect);
}

void wxAuiMSWToolBarArt::DrawOverflowButton(
    wxDC& dc,
    wxWindow* wnd,
    const wxRect& rect,
    int state)
{
    if ( m_themed )
    {
        RECT r;
        wxCopyRectToRECT(rect, r);

        wxUxThemeHandle hTheme(wnd, "Rebar");

        int chevState;
        if ( state & wxAUI_BUTTON_STATE_PRESSED )
            chevState = CHEVS_PRESSED;
        else if ( state & wxAUI_BUTTON_STATE_HOVER )
                chevState = CHEVS_HOT;
        else
            chevState = CHEVS_NORMAL;

        ::DrawThemeBackground(
            hTheme,
            GetHdcOf(dc.GetTempHDC()),
            (m_flags & wxAUI_TB_VERTICAL) ? RP_CHEVRONVERT : RP_CHEVRON,
            chevState,
            &r,
            nullptr);
    }
    else
        wxAuiGenericToolBarArt::DrawOverflowButton(dc, wnd, rect, state);
}

wxSize wxAuiMSWToolBarArt::GetLabelSize(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item)
{
    return wxAuiGenericToolBarArt::GetLabelSize(dc, wnd, item);
}

wxSize wxAuiMSWToolBarArt::GetToolSize(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item)
{
    if ( m_themed )
    {
        if ( !item.GetBitmap().IsOk() && !(m_flags & wxAUI_TB_TEXT) )
            return m_buttonSize;

        wxSize size = wxAuiGenericToolBarArt::GetToolSize(dc, wnd, item);

        size.IncBy(wnd->FromDIP(3)); // Add some padding for native theme

        return size;
    }
    else
        return wxAuiGenericToolBarArt::GetToolSize(dc, wnd, item);
}

int wxAuiMSWToolBarArt::GetElementSize(int element)
{
    return wxAuiGenericToolBarArt::GetElementSize(element);
}

void wxAuiMSWToolBarArt::SetElementSize(int elementId, int size)
{
    wxAuiGenericToolBarArt::SetElementSize(elementId, size);
}

int wxAuiMSWToolBarArt::ShowDropDown(wxWindow* wnd,
    const wxAuiToolBarItemArray& items)
{
    return wxAuiGenericToolBarArt::ShowDropDown(wnd, items);
}
