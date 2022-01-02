/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/slider.cpp
// Purpose:     wxSlider, using trackbar control
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart 1998
//                  Vadim Zeitlin 2004
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SLIDER

#include "wx/slider.h"

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"

#include "wx/msw/subwin.h"

#include <boost/nowide/convert.hpp>

import WX.WinDef;

import <algorithm>;
import <cmath>;
import <string_view>;
import <utility>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

namespace
{

// indices of labels in wxSlider::m_labels
enum
{
    SliderLabel_Min,
    SliderLabel_Max,
    SliderLabel_Value,
    SliderLabel_Last
};

// the gaps between the slider and the labels, in pixels
const int HGAP = 5;
const int VGAP = 4;
// this value is arbitrary:
const int TICK = 8;

} // anonymous namespace

bool wxSlider::Create(wxWindow *parent,
                      wxWindowID id,
                      int value,
                      int minValue,
                      int maxValue,
                      const wxPoint& pos,
                      const wxSize& size,
                      unsigned int style,
                      const wxValidator& validator,
                      std::string_view name)
{
    wxCHECK_MSG( minValue < maxValue, false,
        "Slider minimum must be strictly less than the maximum." );

    // our styles are redundant: wxSL_LEFT/RIGHT imply wxSL_VERTICAL and
    // wxSL_TOP/BOTTOM imply wxSL_HORIZONTAL, but for backwards compatibility
    // reasons we can't really change it, instead try to infer the orientation
    // from the flags given to us here
    switch ( style & (wxSL_LEFT | wxSL_RIGHT | wxSL_TOP | wxSL_BOTTOM) )
    {
        case wxSL_LEFT:
        case wxSL_RIGHT:
            style |= wxSL_VERTICAL;
            break;

        case wxSL_TOP:
        case wxSL_BOTTOM:
            style |= wxSL_HORIZONTAL;
            break;

        case 0:
            // no specific direction, do we have at least the orientation?
            if ( !(style & (wxSL_HORIZONTAL | wxSL_VERTICAL)) )
            {
                // no, choose default
                style |= wxSL_BOTTOM | wxSL_HORIZONTAL;
            }
    }

    wxASSERT_MSG( !(style & wxSL_VERTICAL) || !(style & wxSL_HORIZONTAL),
                    "incompatible slider direction and orientation" );


    // initialize everything
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    // ensure that we have correct values for GetLabelsSize()
    m_rangeMin = minValue;
    m_rangeMax = maxValue;

    // create the labels first, so that our DoGetBestSize() could take them
    // into account
    //
    // note that we could simply create 3 wxStaticTexts here but it could
    // result in some observable side effects at wx level (e.g. the parent of
    // wxSlider would have 3 more children than expected) and so we prefer not
    // to do it like this
    if ( m_windowStyle & wxSL_LABELS )
    {
        m_labels = new wxSubwindows(SliderLabel_Last);

        WXHWND hwndParent = GetHwndOf(parent);
        for ( size_t n = 0; n < SliderLabel_Last; n++ )
        {
            wxWindowIDRef lblid = NewControlId();

            WXHWND wnd = ::CreateWindowW
                         (
                            L"STATIC",
                            nullptr,
                            WS_CHILD | WS_VISIBLE | SS_CENTER,
                            0, 0, 0, 0,
                            hwndParent,
                            (WXHMENU)wxUIntToPtr(lblid.GetValue()),
                            wxGetInstance(),
                            nullptr
                         );

            m_labels->Set(n, wnd, lblid);
        }
    }

    // now create the main control too
    if ( !MSWCreateControl(TRACKBAR_CLASSA, "", pos, size) )
        return false;

    if ( m_labels )
    {
        m_labels->SetFont(GetFont());
    }

    // and initialize everything
    SetRange(minValue, maxValue);
    SetValue(value);
    SetPageSize( std::max(1, (maxValue - minValue)/10) );

    // we need to position the labels correctly if we have them and if
    // SetSize() hadn't been called before (when best size was determined by
    // MSWCreateControl()) as in this case they haven't been put in place yet
    if ( m_labels && size.x != wxDefaultCoord && size.y != wxDefaultCoord )
    {
        SetSize(size);
    }

    Bind(wxEVT_DPI_CHANGED, &wxSlider::OnDPIChanged, this);

    return true;
}

WXDWORD wxSlider::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    // TBS_HORZ, TBS_RIGHT and TBS_BOTTOM are 0 but do include them for clarity
    msStyle |= style & wxSL_VERTICAL ? TBS_VERT : TBS_HORZ;

    // allow setting thumb size
    msStyle |= TBS_FIXEDLENGTH;

    if ( style & wxSL_BOTH )
    {
        // this fully specifies the style combined with TBS_VERT/HORZ above
        msStyle |= TBS_BOTH;
    }
    else // choose one direction
    {
        if ( style & wxSL_LEFT )
            msStyle |= TBS_LEFT;
        else if ( style & wxSL_RIGHT )
            msStyle |= TBS_RIGHT;
        else if ( style & wxSL_TOP )
            msStyle |= TBS_TOP;
        else if ( style & wxSL_BOTTOM )
            msStyle |= TBS_BOTTOM;
    }

    if ( style & wxSL_AUTOTICKS )
        msStyle |= TBS_AUTOTICKS;
    else
        msStyle |= TBS_NOTICKS;

    if ( style & wxSL_SELRANGE )
        msStyle |= TBS_ENABLESELRANGE;

    return msStyle;
}

wxSlider::~wxSlider()
{
    delete m_labels;
}

// ----------------------------------------------------------------------------
// event handling
// ----------------------------------------------------------------------------

bool wxSlider::MSWOnScroll([[maybe_unused]] int orientation,
                           WXWORD wParam,
                           [[maybe_unused]] WXWORD pos,
                           WXHWND control)
{
    wxEventType scrollEvent;
    switch ( wParam )
    {
        case SB_TOP:
            scrollEvent = wxEVT_SCROLL_TOP;
            break;

        case SB_BOTTOM:
            scrollEvent = wxEVT_SCROLL_BOTTOM;
            break;

        case SB_LINEUP:
            scrollEvent = wxEVT_SCROLL_LINEUP;
            break;

        case SB_LINEDOWN:
            scrollEvent = wxEVT_SCROLL_LINEDOWN;
            break;

        case SB_PAGEUP:
            scrollEvent = wxEVT_SCROLL_PAGEUP;
            break;

        case SB_PAGEDOWN:
            scrollEvent = wxEVT_SCROLL_PAGEDOWN;
            break;

        case SB_THUMBTRACK:
            scrollEvent = wxEVT_SCROLL_THUMBTRACK;
            m_isDragging = true;
            break;

        case SB_THUMBPOSITION:
            if ( m_isDragging )
            {
                scrollEvent = wxEVT_SCROLL_THUMBRELEASE;
                m_isDragging = false;
            }
            else
            {
                // this seems to only happen when the mouse wheel is used: in
                // this case, as it might be unexpected to get THUMBRELEASE
                // without preceding THUMBTRACKs, we don't generate it at all
                // but generate CHANGED event because the control itself does
                // not send us SB_ENDSCROLL for whatever reason when mouse
                // wheel is used
                scrollEvent = wxEVT_SCROLL_CHANGED;
            }
            break;

        case SB_ENDSCROLL:
            scrollEvent = wxEVT_SCROLL_CHANGED;
            break;

        default:
            // unknown scroll event?
            return false;
    }

    int newPos = ValueInvertOrNot((int) ::SendMessageW((WXHWND) control, TBM_GETPOS, 0, 0));
    if ( (newPos < GetMin()) || (newPos > GetMax()) )
    {
        // out of range - but we did process it
        return true;
    }

    SetValue(newPos);

    wxScrollEvent event(scrollEvent, m_windowId);
    bool          processed = false;

    event.SetPosition(newPos);
    event.SetEventObject( this );
    processed = HandleWindowEvent(event);

    // Do not generate wxEVT_SLIDER when the native scroll message
    // parameter is SB_ENDSCROLL, which always follows only after
    // another scroll message which already changed the slider value.
    // Therefore, sending wxEVT_SLIDER after SB_ENDSCROLL
    // would result in two wxEVT_SLIDER events with the same value.
    if ( wParam != SB_ENDSCROLL )
    {
        wxCommandEvent cevent( wxEVT_SLIDER, GetId() );

        cevent.SetInt( newPos );
        cevent.SetEventObject( this );

        processed = HandleWindowEvent( cevent );
    }

    return processed;
}

void wxSlider::Command (wxCommandEvent & event)
{
    SetValue (event.GetInt());
    ProcessCommand (event);
}

// ----------------------------------------------------------------------------
// geometry stuff
// ----------------------------------------------------------------------------

wxRect wxSlider::GetBoundingBox() const
{
    // take care not to call our own functions which would call us recursively

    wxRect rect(wxSliderBase::DoGetPosition(), wxSliderBase::DoGetSize());

    if ( m_labels )
    {
        wxRect lrect = m_labels->GetBoundingBox();
        GetParent()->ScreenToClient(&lrect.x, &lrect.y);
        rect.Union(lrect);
    }

    return rect;
}

wxSize wxSlider::DoGetSize() const
{
    wxRect rect = GetBoundingBox();

    return {rect.width, rect.height};
}

wxPoint wxSlider::DoGetPosition() const
{
    wxRect rect = GetBoundingBox();

    return {rect.x, rect.y};
}

int wxSlider::GetLabelsSize(int *widthMin, int *widthMax) const
{
    if ( widthMin && widthMax )
    {
        *widthMin = GetTextExtent(Format(m_rangeMin)).x;
        *widthMax = GetTextExtent(Format(m_rangeMax)).x;

        if ( HasFlag(wxSL_INVERSE) )
        {
            std::swap(*widthMin, *widthMax);
        }
    }

    return HasFlag(wxSL_LABELS) ? GetCharHeight() : 0;
}

void wxSlider::DoMoveWindow(wxRect boundary)
{
    // all complications below are because we need to position the labels,
    // without them everything is easy
    if ( !m_labels )
    {
        wxSliderBase::DoMoveWindow(boundary);
        return;
    }

    const int thumbSize = GetThumbLength();
    const int tickSize = FromDIP(TICK);

    int minLabelWidth,
        maxLabelWidth;
    const int labelHeight = GetLabelsSize(&minLabelWidth, &maxLabelWidth);
    const int longestLabelWidth = std::max(minLabelWidth, maxLabelWidth);
    if ( !HasFlag(wxSL_MIN_MAX_LABELS) )
    {
        minLabelWidth =
        maxLabelWidth = 0;
    }

    int tickOffset = 0;
    if ( HasFlag(wxSL_TICKS) )
        tickOffset = tickSize;
    if ( HasFlag(wxSL_BOTH) )
        tickOffset *= 2;

    // be careful to position the slider itself after moving the labels as
    // otherwise our GetBoundingBox(), which is called from WM_SIZE handler,
    // would return a wrong result and wrong size would be cached internally
    if ( HasFlag(wxSL_VERTICAL) )
    {
        // Position of the slider.
        int sliderOffset = 0;
        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            if ( !HasFlag(wxSL_LEFT) )
                sliderOffset += longestLabelWidth + HGAP;
        }

        int labelHeightUsed = 0;

        if ( HasFlag(wxSL_MIN_MAX_LABELS) )
        {
            int xPos;
            int holdTopX;
            int holdBottomX;
            if ( HasFlag(wxSL_LEFT) )
            {
                // Label aligned to the left edge of the slider.
                xPos = sliderOffset + thumbSize / 2;
                if ( HasFlag(wxSL_TICKS) )
                    xPos += tickSize;

                holdTopX = xPos - minLabelWidth / 2;
                holdBottomX = xPos - maxLabelWidth / 2;
                if ( holdTopX + minLabelWidth > boundary.width )
                    holdTopX = boundary.width - minLabelWidth;
                if ( holdBottomX + maxLabelWidth > boundary.width )
                    holdBottomX = boundary.width - maxLabelWidth;
            }
            else // wxSL_RIGHT
            {
                // Label aligned to the right edge of the slider.
                xPos = sliderOffset + thumbSize / 2;
                if ( HasFlag(wxSL_TICKS) && HasFlag(wxSL_BOTH) )
                    xPos += tickSize;

                holdTopX = xPos - minLabelWidth / 2;
                holdBottomX = xPos - maxLabelWidth / 2;
                if ( holdTopX < 0 )
                    holdTopX = 0;
                if ( holdBottomX < 0 )
                    holdBottomX = 0;
            }

            DoMoveSibling((WXHWND)(*m_labels)[SliderLabel_Min], wxRect{
                boundary.x + holdTopX,
                boundary.y,
                minLabelWidth, labelHeight});
            DoMoveSibling((WXHWND)(*m_labels)[SliderLabel_Max], wxRect{
                boundary.x + holdBottomX,
                boundary.y + boundary.height - labelHeight,
                maxLabelWidth, labelHeight});

            labelHeightUsed = labelHeight;
        }

        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            DoMoveSibling((WXHWND)(*m_labels)[SliderLabel_Value], wxRect{
                boundary.x + ( HasFlag(wxSL_LEFT) ? thumbSize + tickOffset + HGAP : 0 ),
                boundary.y + (boundary.height - labelHeight) / 2,
                longestLabelWidth, labelHeight});
        }

        // position the slider itself along the left/right edge
        wxSliderBase::DoMoveWindow(wxRect{
            boundary.x + sliderOffset,
            boundary.y + labelHeightUsed,
            thumbSize + tickOffset,
            boundary.height - (labelHeightUsed * 2)});
    }
    else // horizontal
    {
        int yLabelMinMax =
            ((thumbSize + tickOffset) / 2) - (labelHeight / 2);
        int xLabelValue =
            minLabelWidth +
            ((boundary.width - (minLabelWidth + maxLabelWidth)) / 2) -
            (longestLabelWidth / 2);

        int ySlider = 0;
        int minLabelWidthUsed = 0;
        int maxLabelWidthUsed = 0;

        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            DoMoveSibling((WXHWND)(*m_labels)[SliderLabel_Value], wxRect{
                boundary.x + xLabelValue,
                boundary.y + (HasFlag(wxSL_BOTTOM) ? 0 : thumbSize + tickOffset),
                longestLabelWidth, labelHeight});

            if ( HasFlag(wxSL_BOTTOM) )
            {
                ySlider += labelHeight;
                yLabelMinMax += labelHeight;
            }
        }

        if ( HasFlag(wxSL_MIN_MAX_LABELS) )
        {
            DoMoveSibling((WXHWND)(*m_labels)[SliderLabel_Min], wxRect{
                boundary.x,
                boundary.y + yLabelMinMax,
                minLabelWidth, labelHeight});
            DoMoveSibling((WXHWND)(*m_labels)[SliderLabel_Max], wxRect{
                boundary.x + boundary.width - maxLabelWidth,
                boundary.y + yLabelMinMax,
                maxLabelWidth, labelHeight});

            minLabelWidthUsed = minLabelWidth + VGAP;
            maxLabelWidthUsed = maxLabelWidth + VGAP;
        }

        // position the slider itself along the top/bottom edge
        wxSliderBase::DoMoveWindow(wxRect{
            boundary.x + minLabelWidthUsed,
            boundary.y + ySlider,
            boundary.width - (minLabelWidthUsed + maxLabelWidthUsed),
            thumbSize + tickOffset});
    }
}

wxSize wxSlider::DoGetBestSize() const
{
    // this value is arbitrary:
    const int length = FromDIP(100);

    // We need 2 extra pixels (which are not scaled by the DPI by the native
    // control) on either side to account for the focus rectangle.
    const int thumbSize = GetThumbLength() + 4;

    const int tickSize = FromDIP(TICK);

    int *width;
    wxSize size;
    if ( HasFlag(wxSL_VERTICAL) )
    {
        size.Set(thumbSize, length);
        width = &size.x;

        if ( m_labels )
        {
            int widthMin,
                widthMax;
            int hLabel = GetLabelsSize(&widthMin, &widthMax);
            const int longestLabelWidth = std::max(widthMin, widthMax);

            // account for the labels
            if ( HasFlag(wxSL_VALUE_LABEL) )
                size.x += longestLabelWidth + HGAP;

            if ( HasFlag(wxSL_MIN_MAX_LABELS) )
            {
                size.x = std::max(size.x, longestLabelWidth);
                size.y += hLabel * 2;
            }
        }
    }
    else // horizontal
    {
        size.Set(length, thumbSize);
        width = &size.y;

        if ( m_labels )
        {
            int labelSize = GetLabelsSize();

            // Min/max labels are compensated by the thumb so we don't need
            // extra space for them

            // The value label is always on top of the control and so does need
            // extra space in any case.
            if ( HasFlag(wxSL_VALUE_LABEL) )
                size.y += labelSize;
        }
    }

    // need extra space to show ticks
    if ( HasFlag(wxSL_TICKS) )
    {
        *width += tickSize;
        // and maybe twice as much if we show them on both sides
        if ( HasFlag(wxSL_BOTH) )
            *width += tickSize;
    }
    return size;
}

WXHBRUSH wxSlider::DoMSWControlColor(WXHDC pDC, wxColour colBg, WXHWND hWnd)
{
    const WXHBRUSH hBrush = wxSliderBase::DoMSWControlColor(pDC, colBg, hWnd);

    // The native control doesn't repaint itself when it's invalidated, so we
    // do it explicitly from here, as this is the only way to propagate the
    // parent background colour to the slider when it changes.
    if ( hWnd == GetHwnd() && hBrush != m_hBrushBg )
    {
        m_hBrushBg = hBrush;

        // Anything really refreshing the slider would work here, we use a
        // dummy WM_ENABLE but using TBM_SETPOS would work too, for example.
        ::PostMessageW(hWnd, WM_ENABLE, ::IsWindowEnabled(hWnd), 0);
    }

    return hBrush;
}

void wxSlider::MSWUpdateFontOnDPIChange(const wxSize& newDPI)
{
    wxSliderBase::MSWUpdateFontOnDPIChange(newDPI);

    if ( m_labels && m_font.IsOk() )
    {
        m_labels->SetFont(m_font);
    }
}

void wxSlider::OnDPIChanged(wxDPIChangedEvent& event)
{
    int thumbLen = GetThumbLength();

    const double scaleFactor = (double)event.GetNewDPI().x / event.GetOldDPI().x;
    const double thumbLenScaled = thumbLen * scaleFactor;
    thumbLen = (int)(scaleFactor > 1.0 ? std::ceil(thumbLenScaled) : std::floor(thumbLenScaled));

    SetThumbLength(thumbLen);
}

// ----------------------------------------------------------------------------
// slider-specific methods
// ----------------------------------------------------------------------------

int wxSlider::GetValue() const
{
    return ValueInvertOrNot(::SendMessageW(GetHwnd(), TBM_GETPOS, 0, 0));
}

void wxSlider::SetValue(int value)
{
    ::SendMessageW(GetHwnd(), TBM_SETPOS, (WXWPARAM)TRUE, (WXLPARAM)ValueInvertOrNot(value));

    if ( m_labels )
    {
        ::SetWindowTextW((*m_labels)[SliderLabel_Value], boost::nowide::widen(Format(value)).c_str());
    }
}

void wxSlider::SetRange(int minValue, int maxValue)
{
    // Remember the old logical value if we need to update the physical control
    // value after changing its range in wxSL_INVERSE case (and avoid an
    // unnecessary call to GetValue() otherwise as it's just not needed).
    const int valueOld = HasFlag(wxSL_INVERSE) ? GetValue() : 0;

    m_rangeMin = minValue;
    m_rangeMax = maxValue;

    ::SendMessageW(GetHwnd(), TBM_SETRANGEMIN, TRUE, m_rangeMin);
    ::SendMessageW(GetHwnd(), TBM_SETRANGEMAX, TRUE, m_rangeMax);

    if ( m_labels )
    {
        Move(wxDefaultPosition, wxSIZE_FORCE); // Force a re-layout the labels.

        // Update the label with the value adjusted by the control as
        // old value can be out of the new range.
        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            SetValue(GetValue());
        }
        ::SetWindowTextW((*m_labels)[SliderLabel_Min],
                        boost::nowide::widen(Format(ValueInvertOrNot(m_rangeMin))).c_str());
        ::SetWindowTextW((*m_labels)[SliderLabel_Max],
                        boost::nowide::widen(Format(ValueInvertOrNot(m_rangeMax))).c_str());
    }

    // When emulating wxSL_INVERSE style in wxWidgets, we need to update the
    // value after changing the range to ensure that the value seen by the user
    // code, i.e. the one returned by GetValue(), does not change.
    if ( HasFlag(wxSL_INVERSE) )
    {
        ::SendMessageW(GetHwnd(), TBM_SETPOS, TRUE, ValueInvertOrNot(valueOld));
    }
}

void wxSlider::DoSetTickFreq(int n)
{
    m_tickFreq = n;
    ::SendMessageW( GetHwnd(), TBM_SETTICFREQ, (WXWPARAM) n, (WXLPARAM) 0 );
}

void wxSlider::SetPageSize(int pageSize)
{
    ::SendMessageW( GetHwnd(), TBM_SETPAGESIZE, (WXWPARAM) 0, (WXLPARAM) pageSize );
    m_pageSize = pageSize;
}

int wxSlider::GetPageSize() const
{
    return m_pageSize;
}

void wxSlider::ClearSel()
{
    ::SendMessageW(GetHwnd(), TBM_CLEARSEL, (WXWPARAM) TRUE, (WXLPARAM) 0);
}

void wxSlider::ClearTicks()
{
    ::SendMessageW(GetHwnd(), TBM_CLEARTICS, (WXWPARAM) TRUE, (WXLPARAM) 0);
}

void wxSlider::SetLineSize(int lineSize)
{
    m_lineSize = lineSize;
    ::SendMessageW(GetHwnd(), TBM_SETLINESIZE, (WXWPARAM) 0, (WXLPARAM) lineSize);
}

int wxSlider::GetLineSize() const
{
    return (int)::SendMessageW(GetHwnd(), TBM_GETLINESIZE, 0, 0);
}

int wxSlider::GetSelEnd() const
{
    return (int)::SendMessageW(GetHwnd(), TBM_GETSELEND, 0, 0);
}

int wxSlider::GetSelStart() const
{
    return (int)::SendMessageW(GetHwnd(), TBM_GETSELSTART, 0, 0);
}

void wxSlider::SetSelection(int minPos, int maxPos)
{
    ::SendMessageW(GetHwnd(), TBM_SETSEL,
                  (WXWPARAM) TRUE /* redraw */,
                  (WXLPARAM) MAKELONG( minPos, maxPos) );
}

void wxSlider::SetThumbLength(int len)
{
    ::SendMessageW(GetHwnd(), TBM_SETTHUMBLENGTH, (WXWPARAM) len, (WXLPARAM) 0);

    InvalidateBestSize();
}

int wxSlider::GetThumbLength() const
{
    return (int)::SendMessageW( GetHwnd(), TBM_GETTHUMBLENGTH, 0, 0);
}

void wxSlider::SetTick(int tickPos)
{
    ::SendMessageW( GetHwnd(), TBM_SETTIC, (WXWPARAM) 0, (WXLPARAM) tickPos );
}

// ----------------------------------------------------------------------------
// composite control methods
// ----------------------------------------------------------------------------

WXHWND wxSlider::GetStaticMin() const
{
    return m_labels ? (WXHWND)(*m_labels)[SliderLabel_Min] : nullptr;
}

WXHWND wxSlider::GetStaticMax() const
{
    return m_labels ? (WXHWND)(*m_labels)[SliderLabel_Max] : nullptr;
}

WXHWND wxSlider::GetEditValue() const
{
    return m_labels ? (WXHWND)(*m_labels)[SliderLabel_Value] : nullptr;
}

WX_FORWARD_STD_METHODS_TO_SUBWINDOWS(wxSlider, wxSliderBase, m_labels)

#endif // wxUSE_SLIDER
