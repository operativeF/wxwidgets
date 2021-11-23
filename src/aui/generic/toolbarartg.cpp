module;

#include "wx/font.h"
#include "wx/menu.h"
#include "wx/pen.h"
#include "wx/settings.h"
#include "wx/window.h"

module WX.AUI.ToolBarArt.Generic;

import WX.AUI.AUIBar;
import WX.AUI.DockArt; // FIXME: imported for wxAuiBitmapFromBits utility function
import WX.AUI.FrameManager;
import WX.AUI.ToolBarArt.Base;

namespace
{

wxColor GetBaseColor()
{

    wxColor baseColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);

    // the baseColour is too pale to use as our base colour,
    // so darken it a bit --
    if ((255-baseColour.Red()) +
        (255-baseColour.Green()) +
        (255-baseColour.Blue()) < 60)
    {
        baseColour = baseColour.ChangeLightness(92);
    }

    return baseColour;
}

bool IsThemeDark()
{
    return wxSystemSettings::GetAppearance().IsDark();
}

class ToolbarCommandCapture : public wxEvtHandler
{
public:

    ToolbarCommandCapture() { m_lastId = 0; }
    int GetCommandId() const { return m_lastId; }

    bool ProcessEvent(wxEvent& evt) override
    {
        if (evt.GetEventType() == wxEVT_MENU)
        {
            m_lastId = evt.GetId();
            return true;
        }

        if (GetNextHandler())
            return GetNextHandler()->ProcessEvent(evt);

        return false;
    }

private:
    int m_lastId;
};

} // namespace anonymous


wxAuiGenericToolBarArt::wxAuiGenericToolBarArt()
{
    UpdateColoursFromSystem();

    m_flags = 0;
    m_textOrientation = wxAUI_TBTOOL_TEXT_BOTTOM;

    m_separatorSize = wxWindow::FromDIP( 7, nullptr);
    m_gripperSize   = wxWindow::FromDIP( 7, nullptr);
    m_overflowSize  = wxWindow::FromDIP(16, nullptr);
    m_dropdownSize  = wxWindow::FromDIP(10, nullptr);


    m_font = *wxNORMAL_FONT;
}

wxAuiGenericToolBarArt::~wxAuiGenericToolBarArt()
{
    m_font = *wxNORMAL_FONT;
}

wxAuiToolBarArt* wxAuiGenericToolBarArt::Clone()
{
    return new wxAuiGenericToolBarArt;
}

void wxAuiGenericToolBarArt::UpdateColoursFromSystem()
{
    m_baseColour = GetBaseColor();
    m_highlightColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    wxColor darker3Colour = m_baseColour.ChangeLightness(60);
    wxColor darker5Colour = m_baseColour.ChangeLightness(40);

    int pen_width = wxWindow::FromDIP(1, nullptr);
    m_gripperPen1 = wxPen(darker5Colour, pen_width);
    m_gripperPen2 = wxPen(darker3Colour, pen_width);
    m_gripperPen3 = wxPen(*wxStockGDI::GetColour(wxStockGDI::COLOUR_WHITE), pen_width);

    // Note: update the bitmaps here as they depend on the system colours too.

    // TODO: Provide x1.5 and x2.0 versions or migrate to SVG.
    static constexpr unsigned char buttonDropdownBits[] = { 0xe0, 0xf1, 0xfb };
    static constexpr unsigned char overflowBits[] = { 0x80, 0xff, 0x80, 0xc1, 0xe3, 0xf7 };

    m_buttonDropDownBmp = wxAuiBitmapFromBits(buttonDropdownBits, wxSize{5, 3},
                                              wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    m_disabledButtonDropDownBmp = wxAuiBitmapFromBits(
                                                buttonDropdownBits, wxSize{5, 3},
                                                wxColor(128,128,128));
    m_overflowBmp = wxAuiBitmapFromBits(overflowBits, wxSize{7, 6},
                                        wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    m_disabledOverflowBmp = wxAuiBitmapFromBits(overflowBits, wxSize{7, 6}, wxColor(128,128,128));
}

void wxAuiGenericToolBarArt::SetFlags(unsigned int flags)
{
    m_flags = flags;
}

void wxAuiGenericToolBarArt::SetFont(const wxFont& font)
{
    m_font = font;
}

void wxAuiGenericToolBarArt::SetTextOrientation(int orientation)
{
    m_textOrientation = orientation;
}

unsigned int wxAuiGenericToolBarArt::GetFlags()
{
    return m_flags;
}

wxFont wxAuiGenericToolBarArt::GetFont()
{
    return m_font;
}

int wxAuiGenericToolBarArt::GetTextOrientation()
{
    return m_textOrientation;
}

void wxAuiGenericToolBarArt::DrawBackground(
                                    wxDC& dc,
                                    wxWindow* WXUNUSED(wnd),
                                    const wxRect& _rect)
{
    wxRect rect = _rect;
    rect.height++;

    int startLightness = 150;
    int endLightness = 90;

    if ((m_baseColour.Red() < 75)
        && (m_baseColour.Green() < 75)
        && (m_baseColour.Blue() < 75))
    {
        //dark mode, we cannot go very light
        startLightness = 110;
        endLightness = 90;
    }
    wxColour startColour = m_baseColour.ChangeLightness(startLightness);
    wxColour endColour = m_baseColour.ChangeLightness(endLightness);

    dc.GradientFillLinear(rect, startColour, endColour, wxSOUTH);
}

void wxAuiGenericToolBarArt::DrawPlainBackground(wxDC& dc,
                                                   wxWindow* WXUNUSED(wnd),
                                                   const wxRect& rect)
{
    dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    dc.SetPen(*wxTRANSPARENT_PEN);

    dc.DrawRectangle(rect);
}

void wxAuiGenericToolBarArt::DrawLabel(
                                    wxDC& dc,
                                    wxWindow* WXUNUSED(wnd),
                                    const wxAuiToolBarItem& item,
                                    const wxRect& rect)
{
    dc.SetFont(m_font);
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));

    // we only care about the text height here since the text
    // will get cropped based on the width of the item
    int textWidth = 0, textHeight = 0;
    dc.GetTextExtent("ABCDHgj", &textWidth, &textHeight);

    // set the clipping region
    wxRect clipRect = rect;
    clipRect.width -= 1;
    dc.SetClippingRegion(clipRect);

    int textX, textY;
    textX = rect.x + 1;
    textY = rect.y + (rect.height-textHeight)/2;
    dc.wxDrawText(item.GetLabel(), wxPoint{textX, textY});
    dc.DestroyClippingRegion();
}


void wxAuiGenericToolBarArt::DrawButton(
                                    wxDC& dc,
                                    wxWindow* wnd,
                                    const wxAuiToolBarItem& item,
                                    const wxRect& rect)
{
    int textWidth = 0, textHeight = 0;

    if (m_flags & wxAUI_TB_TEXT)
    {
        dc.SetFont(m_font);

        int tx, ty;

        dc.GetTextExtent("ABCDHgj", &tx, &textHeight);
        textWidth = 0;
        dc.GetTextExtent(item.GetLabel(), &textWidth, &ty);
    }

    int bmpX = 0, bmpY = 0;
    int textX = 0, textY = 0;

    const wxBitmap& bmp = item.GetState() & wxAUI_BUTTON_STATE_DISABLED
                            ? item.GetDisabledBitmap()
                            : item.GetBitmap();

    const wxSize bmpSize = bmp.IsOk() ? bmp.GetScaledSize() : wxSize(0, 0);

    if (m_textOrientation == wxAUI_TBTOOL_TEXT_BOTTOM)
    {
        bmpX = rect.x +
                (rect.width/2) -
                (bmpSize.x/2);

        bmpY = rect.y +
                ((rect.height-textHeight)/2) -
                (bmpSize.y/2);

        textX = rect.x + (rect.width/2) - (textWidth/2) + 1;
        textY = rect.y + rect.height - textHeight - 1;
    }
    else if (m_textOrientation == wxAUI_TBTOOL_TEXT_RIGHT)
    {
        bmpX = rect.x + wnd->FromDIP(3);

        bmpY = rect.y +
                (rect.height/2) -
                (bmpSize.y/2);

        textX = bmpX + wnd->FromDIP(3) + bmpSize.x;
        textY = rect.y +
                 (rect.height/2) -
                 (textHeight/2);
    }


    if (!(item.GetState() & wxAUI_BUTTON_STATE_DISABLED))
    {
        if (item.GetState() & wxAUI_BUTTON_STATE_PRESSED)
        {
            dc.SetPen(wxPen(m_highlightColour));
            dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 20 : 150)));
            dc.DrawRectangle(rect);
        }
        else if ((item.GetState() & wxAUI_BUTTON_STATE_HOVER) || item.IsSticky())
        {
            dc.SetPen(wxPen(m_highlightColour));
            dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 40 : 170)));

            // draw an even lighter background for checked item hovers (since
            // the hover background is the same color as the check background)
            if (item.GetState() & wxAUI_BUTTON_STATE_CHECKED)
                dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 50 : 180)));

            dc.DrawRectangle(rect);
        }
        else if (item.GetState() & wxAUI_BUTTON_STATE_CHECKED)
        {
            // it's important to put this code in an else statement after the
            // hover, otherwise hovers won't draw properly for checked items
            dc.SetPen(wxPen(m_highlightColour));
            dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 40 : 170)));
            dc.DrawRectangle(rect);
        }
    }

    if ( bmp.IsOk() )
        dc.DrawBitmap(bmp, bmpX, bmpY, true);

    // set the item's text color based on if it is disabled
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    if (item.GetState() & wxAUI_BUTTON_STATE_DISABLED)
    {
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    if ( (m_flags & wxAUI_TB_TEXT) && !item.GetLabel().empty() )
    {
        dc.wxDrawText(item.GetLabel(), wxPoint{textX, textY});
    }
}


void wxAuiGenericToolBarArt::DrawDropDownButton(
                                    wxDC& dc,
                                    wxWindow* wnd,
                                    const wxAuiToolBarItem& item,
                                    const wxRect& rect)
{
    int dropdownWidth = GetElementSize(wxAUI_TBART_DROPDOWN_SIZE);
    int textWidth = 0, textHeight = 0, textX = 0, textY = 0;
    int bmpX = 0, bmpY = 0, dropBmpX = 0, dropBmpY = 0;

    wxRect buttonRect = wxRect(rect.x,
                                rect.y,
                                rect.width-dropdownWidth,
                                rect.height);
    wxRect dropDownRect = wxRect(rect.x+rect.width-dropdownWidth-1,
                                  rect.y,
                                  dropdownWidth+1,
                                  rect.height);

    if (m_flags & wxAUI_TB_TEXT)
    {
        dc.SetFont(m_font);

        int tx, ty;
        if (m_flags & wxAUI_TB_TEXT)
        {
            dc.GetTextExtent("ABCDHgj", &tx, &textHeight);
            textWidth = 0;
        }

        dc.GetTextExtent(item.GetLabel(), &textWidth, &ty);
    }



    dropBmpX = dropDownRect.x +
                (dropDownRect.width/2) -
                std::lround(m_buttonDropDownBmp.GetScaledWidth()/2);
    dropBmpY = dropDownRect.y +
                std::lround(dropDownRect.height/2) -
                std::lround(m_buttonDropDownBmp.GetScaledHeight()/2);


    if (m_textOrientation == wxAUI_TBTOOL_TEXT_BOTTOM)
    {
        bmpX = buttonRect.x +
                (buttonRect.width/2) -
                std::lround(item.GetBitmap().GetScaledWidth()/2);
        bmpY = buttonRect.y +
                ((buttonRect.height-textHeight)/2) -
                std::lround(item.GetBitmap().GetScaledHeight()/2);

        textX = rect.x + (rect.width/2) - (textWidth/2) + 1;
        textY = rect.y + rect.height - textHeight - 1;
    }
    else if (m_textOrientation == wxAUI_TBTOOL_TEXT_RIGHT)
    {
        bmpX = rect.x + wnd->FromDIP(3);

        bmpY = rect.y +
                (rect.height/2) -
                std::lround(item.GetBitmap().GetScaledHeight()/2);

        textX = bmpX + wnd->FromDIP(3) + std::lround(item.GetBitmap().GetScaledWidth());
        textY = rect.y +
                 (rect.height/2) -
                 (textHeight/2);
    }


    if (item.GetState() & wxAUI_BUTTON_STATE_PRESSED)
    {
        dc.SetPen(wxPen(m_highlightColour));
        dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 10 : 140)));
        dc.DrawRectangle(buttonRect);

        dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 40 : 170)));
        dc.DrawRectangle(dropDownRect);
    }
    else if (item.GetState() & wxAUI_BUTTON_STATE_HOVER ||
             item.IsSticky())
    {
        dc.SetPen(wxPen(m_highlightColour));
        dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 40 : 170)));
        dc.DrawRectangle(buttonRect);
        dc.DrawRectangle(dropDownRect);
    }
    else if (item.GetState() & wxAUI_BUTTON_STATE_CHECKED)
    {
        // Notice that this branch must come after the hover one to ensure the
        // correct appearance when the mouse hovers over a checked item.m_
        dc.SetPen(wxPen(m_highlightColour));
        dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(IsThemeDark() ? 40 : 170)));
        dc.DrawRectangle(buttonRect);
        dc.DrawRectangle(dropDownRect);
    }

    wxBitmap bmp;
    wxBitmap dropbmp;
    if (item.GetState() & wxAUI_BUTTON_STATE_DISABLED)
    {
        bmp = item.GetDisabledBitmap();
        dropbmp = m_disabledButtonDropDownBmp;
    }
    else
    {
        bmp = item.GetBitmap();
        dropbmp = m_buttonDropDownBmp;
    }

    if (!bmp.IsOk())
        return;

    dc.DrawBitmap(bmp, bmpX, bmpY, true);
    dc.DrawBitmap(dropbmp, dropBmpX, dropBmpY, true);

    // set the item's text color based on if it is disabled
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    if (item.GetState() & wxAUI_BUTTON_STATE_DISABLED)
    {
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    if ( (m_flags & wxAUI_TB_TEXT) && !item.GetLabel().empty() )
    {
        dc.wxDrawText(item.GetLabel(), wxPoint{textX, textY});
    }
}

void wxAuiGenericToolBarArt::DrawControlLabel(
                                    wxDC& dc,
                                    wxWindow* WXUNUSED(wnd),
                                    const wxAuiToolBarItem& item,
                                    const wxRect& rect)
{
    if (!(m_flags & wxAUI_TB_TEXT))
        return;

    if (m_textOrientation != wxAUI_TBTOOL_TEXT_BOTTOM)
        return;

    int textX = 0, textY = 0;
    int textWidth = 0, textHeight = 0;

    dc.SetFont(m_font);

    int tx, ty;
    if (m_flags & wxAUI_TB_TEXT)
    {
        dc.GetTextExtent("ABCDHgj", &tx, &textHeight);
        textWidth = 0;
    }

    dc.GetTextExtent(item.GetLabel(), &textWidth, &ty);

    // don't draw the label if it is wider than the item width
    if (textWidth > rect.width)
        return;

    // set the label's text color
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));

    textX = rect.x + (rect.width/2) - (textWidth/2) + 1;
    textY = rect.y + rect.height - textHeight - 1;

    if ( (m_flags & wxAUI_TB_TEXT) && !item.GetLabel().empty() )
    {
        dc.wxDrawText(item.GetLabel(), wxPoint{textX, textY});
    }
}

wxSize wxAuiGenericToolBarArt::GetLabelSize(
                                        wxDC& dc,
                                        wxWindow* WXUNUSED(wnd),
                                        const wxAuiToolBarItem& item)
{
    dc.SetFont(m_font);

    // get label's height
    int width = 0, height = 0;
    dc.GetTextExtent("ABCDHgj", &width, &height);

    // get item's width
    width = item.GetMinSize().x;

    if (width == -1)
    {
        // no width specified, measure the text ourselves
        width = dc.GetTextExtent(item.GetLabel()).GetX();
    }

    return {width, height};
}

wxSize wxAuiGenericToolBarArt::GetToolSize(
                                        wxDC& dc,
                                        wxWindow* wnd,
                                        const wxAuiToolBarItem& item)
{
    if (!item.GetBitmap().IsOk() && !(m_flags & wxAUI_TB_TEXT))
        return wnd->FromDIP(wxSize{16,16});

    const wxBitmap& bmp = item.GetBitmap();
    int width = bmp.IsOk() ? bmp.GetScaledWidth() : 0;
    int height = bmp.IsOk() ? bmp.GetScaledHeight() : 0;

    if (m_flags & wxAUI_TB_TEXT)
    {
        dc.SetFont(m_font);
        int tx, ty;

        if (m_textOrientation == wxAUI_TBTOOL_TEXT_BOTTOM)
        {
            dc.GetTextExtent("ABCDHgj", &tx, &ty);
            height += ty;

            if ( !item.GetLabel().empty() )
            {
                dc.GetTextExtent(item.GetLabel(), &tx, &ty);
                width = std::max(width, tx+wnd->FromDIP(6));
            }
        }
        else if ( m_textOrientation == wxAUI_TBTOOL_TEXT_RIGHT &&
                  !item.GetLabel().empty() )
        {
            width += wnd->FromDIP(3); // space between left border and bitmap
            width += wnd->FromDIP(3); // space between bitmap and text

            if ( !item.GetLabel().empty() )
            {
                dc.GetTextExtent(item.GetLabel(), &tx, &ty);
                width += tx;
                height = std::max(height, ty);
            }
        }
    }

    // if the tool has a dropdown button, add it to the width
    // and add some extra space in front of the drop down button
    if (item.HasDropDown())
    {
        int dropdownWidth = GetElementSize(wxAUI_TBART_DROPDOWN_SIZE);
        width += dropdownWidth + wnd->FromDIP(4);
    }

    return {width, height};
}

void wxAuiGenericToolBarArt::DrawSeparator(
                                    wxDC& dc,
                                    wxWindow* wnd,
                                    const wxRect& _rect)
{
    bool horizontal = true;
    if (m_flags & wxAUI_TB_VERTICAL)
        horizontal = false;

    wxRect rect = _rect;

    if (horizontal)
    {
        rect.x += (rect.width/2);
        rect.width = wnd->FromDIP(1);
        int new_height = (rect.height*3)/4;
        rect.y += (rect.height/2) - (new_height/2);
        rect.height = new_height;
    }
    else
    {
        rect.y += (rect.height/2);
        rect.height = wnd->FromDIP(1);
        int new_width = (rect.width*3)/4;
        rect.x += (rect.width/2) - (new_width/2);
        rect.width = new_width;
    }

    wxColour startColour = m_baseColour.ChangeLightness(80);
    wxColour endColour = m_baseColour.ChangeLightness(80);
    dc.GradientFillLinear(rect, startColour, endColour, horizontal ? wxSOUTH : wxEAST);
}

void wxAuiGenericToolBarArt::DrawGripper(wxDC& dc,
                                    wxWindow* wnd,
                                    const wxRect& rect)
{
    int i = 0;
    while (true)
    {
        int x, y;

        if (m_flags & wxAUI_TB_VERTICAL)
        {
            x = rect.x + (i*wnd->FromDIP(4)) + wnd->FromDIP(5);
            y = rect.y + wnd->FromDIP(3);
            if (x > rect.GetWidth()-wnd->FromDIP(5))
                break;
        }
        else
        {
            x = rect.x + wnd->FromDIP(3);
            y = rect.y + (i*wnd->FromDIP(4)) + wnd->FromDIP(5);
            if (y > rect.GetHeight()-wnd->FromDIP(5))
                break;
        }

        dc.SetPen(m_gripperPen1);
        dc.DrawPoint(x, y);
        dc.SetPen(m_gripperPen2);
        dc.DrawPoint(x                , y+wnd->FromDIP(1));
        dc.DrawPoint(x+wnd->FromDIP(1), y                );
        dc.SetPen(m_gripperPen3);
        dc.DrawPoint(x+wnd->FromDIP(2), y+wnd->FromDIP(1));
        dc.DrawPoint(x+wnd->FromDIP(2), y+wnd->FromDIP(2));
        dc.DrawPoint(x+wnd->FromDIP(1), y+wnd->FromDIP(2));

        i++;
    }

}

void wxAuiGenericToolBarArt::DrawOverflowButton(wxDC& dc,
                                          wxWindow* WXUNUSED(wnd),
                                          const wxRect& rect,
                                          int state)
{
    if (state & wxAUI_BUTTON_STATE_HOVER ||
        state & wxAUI_BUTTON_STATE_PRESSED)
    {
        wxColor light_gray_bg = m_highlightColour.ChangeLightness(IsThemeDark() ? 40 : 170);

        if (m_flags & wxAUI_TB_VERTICAL)
        {
            dc.SetPen(wxPen(m_highlightColour));
            dc.DrawLine(rect.x, rect.y, rect.x+rect.width, rect.y);
            dc.SetPen(wxPen(light_gray_bg));
            dc.SetBrush(wxBrush(light_gray_bg));
            dc.DrawRectangle(rect.x, rect.y+1, rect.width, rect.height);
        }
        else
        {
            dc.SetPen(wxPen(m_highlightColour));
            dc.DrawLine(rect.x, rect.y, rect.x, rect.y+rect.height);
            dc.SetPen(wxPen(light_gray_bg));
            dc.SetBrush(wxBrush(light_gray_bg));
            dc.DrawRectangle(rect.x+1, rect.y, rect.width, rect.height);
        }
    }

    int x = rect.x+1+(rect.width-m_overflowBmp.GetScaledWidth())/2;
    int y = rect.y+1+(rect.height-m_overflowBmp.GetScaledHeight())/2;
    dc.DrawBitmap(m_overflowBmp, x, y, true);
}

int wxAuiGenericToolBarArt::GetElementSize(int element_id)
{
    switch (element_id)
    {
        case wxAUI_TBART_SEPARATOR_SIZE: return m_separatorSize;
        case wxAUI_TBART_GRIPPER_SIZE:   return m_gripperSize;
        case wxAUI_TBART_OVERFLOW_SIZE:  return m_overflowSize;
        case wxAUI_TBART_DROPDOWN_SIZE:  return m_dropdownSize;
        default: return 0;
    }
}

void wxAuiGenericToolBarArt::SetElementSize(int element_id, int size)
{
    switch (element_id)
    {
        case wxAUI_TBART_SEPARATOR_SIZE: m_separatorSize = size; break;
        case wxAUI_TBART_GRIPPER_SIZE:   m_gripperSize = size; break;
        case wxAUI_TBART_OVERFLOW_SIZE:  m_overflowSize = size; break;
        case wxAUI_TBART_DROPDOWN_SIZE:  m_dropdownSize = size; break;
    }
}

int wxAuiGenericToolBarArt::ShowDropDown(wxWindow* wnd,
                                         const wxAuiToolBarItemArray& items)
{
    wxMenu menuPopup;

    size_t items_added = 0;

    size_t count = items.GetCount();
    for (size_t i = 0; i < count; ++i)
    {
        wxAuiToolBarItem& item = items.Item(i);

        if (item.GetKind() == wxITEM_NORMAL)
        {
            auto text = item.GetShortHelp();

            if (text.empty())
                text = item.GetLabel();

            if (text.empty())
                text = " ";

            wxMenuItem* m =  new wxMenuItem(&menuPopup, item.GetId(), text, item.GetShortHelp());

            m->SetBitmap(item.GetBitmap());
            menuPopup.Append(m);
            items_added++;
        }
        else if (item.GetKind() == wxITEM_SEPARATOR)
        {
            if (items_added > 0)
                menuPopup.AppendSeparator();
        }
    }

    // find out where to put the popup menu of window items
    wxPoint pt = ::wxGetMousePosition();
    pt = wnd->ScreenToClient(pt);

    // find out the screen coordinate at the bottom of the tab ctrl
    wxRect cli_rect = wnd->GetClientRect();
    pt.y = cli_rect.y + cli_rect.height;

    ToolbarCommandCapture* cc = new ToolbarCommandCapture;
    wnd->PushEventHandler(cc);
    wnd->PopupMenu(&menuPopup, pt);
    int command = cc->GetCommandId();
    wnd->PopEventHandler(true);

    return command;
}
