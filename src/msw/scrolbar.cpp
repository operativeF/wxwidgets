/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/scrolbar.cpp
// Purpose:     wxScrollBar
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SCROLLBAR

#include "wx/scrolbar.h"

#include "wx/msw/private.h"

#include "wx/utils.h"

import WX.Utils.Cast;
import WX.Utils.Settings;

import WX.WinDef;

// Scrollbar
bool wxScrollBar::Create(wxWindow *parent, wxWindowID id,
           const wxPoint& pos,
           const wxSize& size, unsigned int style,
           const wxValidator& validator,
           std::string_view name)
{
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    if (!MSWCreateControl("ScrollBar", "", pos, size))
        return false;

    SetScrollbar(0, 1, 2, 1, false);

    return true;
}

bool wxScrollBar::MSWOnScroll([[maybe_unused]] int orientation, WXWORD wParam,
                              [[maybe_unused]] WXWORD pos, [[maybe_unused]] WXHWND control)
{
    // don't use pos parameter because it is limited to 16 bits, get the full
    // 32 bit position from the control itself instead
    WinStruct<SCROLLINFO> scrollInfo;
    scrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_TRACKPOS;

    if ( !::GetScrollInfo(GetHwnd(), SB_CTL, &scrollInfo) )
    {
        wxLogLastError("GetScrollInfo");
        return false;
    }

    int maxPos = scrollInfo.nMax;

    // A page size greater than one has the effect of reducing the effective
    // range, therefore the range has already been boosted artificially - so
    // reduce it again.
    if ( m_pageSize > 1 )
        maxPos -= (m_pageSize - 1);

    int position = scrollInfo.nPos;
    wxEventType scrollEvent = wxEVT_NULL;
    switch ( wParam )
    {
        case SB_TOP:
            position = 0;
            scrollEvent = wxEVT_SCROLL_TOP;
            break;

        case SB_BOTTOM:
            position = maxPos;
            scrollEvent = wxEVT_SCROLL_BOTTOM;
            break;

        case SB_LINEUP:
            position--;
            scrollEvent = wxEVT_SCROLL_LINEUP;
            break;

        case SB_LINEDOWN:
            position++;
            scrollEvent = wxEVT_SCROLL_LINEDOWN;
            break;

        case SB_PAGEUP:
            position -= GetPageSize();
            scrollEvent = wxEVT_SCROLL_PAGEUP;
            break;

        case SB_PAGEDOWN:
            position += GetPageSize();
            scrollEvent = wxEVT_SCROLL_PAGEDOWN;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            position = scrollInfo.nTrackPos;
            scrollEvent = wParam == SB_THUMBPOSITION ? wxEVT_SCROLL_THUMBRELEASE
                                                     : wxEVT_SCROLL_THUMBTRACK;
            break;

        case SB_ENDSCROLL:
            scrollEvent = wxEVT_SCROLL_CHANGED;
            break;
    }

    if ( position != scrollInfo.nPos )
    {
        if ( position < 0 )
            position = 0;
        if ( position > maxPos )
            position = maxPos;

        SetThumbPosition(position);
    }
    else if ( scrollEvent != wxEVT_SCROLL_THUMBRELEASE &&
                scrollEvent != wxEVT_SCROLL_CHANGED )
    {
        // don't process the event if there is no displacement,
        // unless this is a thumb release or end scroll event.
        return false;
    }

    wxScrollEvent event(scrollEvent, m_windowId);
    event.SetOrientation(IsVertical() ? wxVERTICAL : wxHORIZONTAL);
    event.SetPosition(position);
    event.SetEventObject( this );

    return HandleWindowEvent(event);
}

void wxScrollBar::SetThumbPosition(int viewStart)
{
    SCROLLINFO info
    {
        info.cbSize = sizeof(SCROLLINFO),
        info.nPage = 0,
        info.nMin = 0,
        info.nPos = viewStart,
        info.fMask = SIF_POS
    };

    ::SetScrollInfo((WXHWND) GetHWND(), SB_CTL, &info, TRUE);
}

int wxScrollBar::GetThumbPosition() const
{
    WinStruct<SCROLLINFO> scrollInfo;
    scrollInfo.fMask = SIF_POS;

    if ( !::GetScrollInfo(GetHwnd(), SB_CTL, &scrollInfo) )
    {
        wxLogLastError("GetScrollInfo");
    }
    return scrollInfo.nPos;
}

void wxScrollBar::SetScrollbar(int position, int thumbSize, int range, int pageSize,
    bool refresh)
{
    m_viewSize = pageSize;
    m_pageSize = thumbSize;
    m_objectSize = range;

    // The range (number of scroll steps) is the
    // object length minus the page size.
    int range1 = std::max((m_objectSize - m_pageSize), 0);

    // Try to adjust the range to cope with page size > 1
    // (see comment for SetPageLength)
    if ( m_pageSize > 1 )
    {
        range1 += (m_pageSize - 1);
    }

    SCROLLINFO info = {
        .cbSize = sizeof(SCROLLINFO),
        .fMask = SIF_PAGE | SIF_RANGE | SIF_POS,
        .nMin = 0,
        .nMax = range1,
        .nPage = wx::narrow_cast<WXUINT>(m_pageSize),
        .nPos = position
    };

    ::SetScrollInfo((WXHWND) GetHWND(), SB_CTL, &info, refresh);
}

void wxScrollBar::Command(wxCommandEvent& event)
{
    SetThumbPosition(event.GetInt());
    ProcessCommand(event);
}

wxSize wxScrollBar::DoGetBestSize() const
{
    int w = 100;
    int h = 100;

    if ( IsVertical() )
    {
        w = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, m_parent);
    }
    else
    {
        h = wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y, m_parent);
    }

    return {w, h};
}

WXDWORD wxScrollBar::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    // we never have an external border
    WXDWORD msStyle = wxControl::MSWGetStyle
                      (
                        (style & ~wxBORDER_MASK) | wxBORDER_NONE, exstyle
                      );

    // SBS_HORZ is 0 anyhow, but do mention it explicitly for clarity
    msStyle |= style & wxSB_HORIZONTAL ? SBS_HORZ : SBS_VERT;

    return msStyle;
}

WXHBRUSH wxScrollBar::MSWControlColor(WXHDC pDC, WXHWND hWnd)
{
    // unless we have an explicitly set bg colour, use default (gradient under
    // XP) brush instead of GetBackgroundColour() one as the base class would
    //
    // note that fg colour isn't used for a scrollbar
    return UseBgCol() ? wxControl::MSWControlColor(pDC, hWnd) : nullptr;
}

#endif // wxUSE_SCROLLBAR
