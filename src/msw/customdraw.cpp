///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/customdraw.cpp
// Purpose:     wxMSWCustomDraw implementation
// Author:      Vadim Zeitlin
// Created:     2016-04-16
// Copyright:   (c) 2016 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/msw/private/customdraw.h"

// ============================================================================
// implementation
// ============================================================================

WXLPARAM wxMSWImpl::CustomDraw::HandleItemPrepaint(const wxItemAttr& attr, WXHDC hdc)
{
    if ( attr.HasTextColour() )
        ::SetTextColor(hdc, wxColourToRGB(attr.GetTextColour()));

    if ( attr.HasBackgroundColour() )
        ::SetBkColor(hdc, wxColourToRGB(attr.GetBackgroundColour()));

    if ( attr.HasFont() )
    {
        ::SelectObject(hdc, GetHfontOf(attr.GetFont()));

        return CDRF_NEWFONT;
    }

    return CDRF_DODEFAULT;
}

WXLPARAM wxMSWImpl::CustomDraw::HandleCustomDraw(WXLPARAM lParam)
{
    NMCUSTOMDRAW* nmcd = reinterpret_cast<NMCUSTOMDRAW*>(lParam);
    switch ( nmcd->dwDrawStage )
    {
        case CDDS_PREPAINT:
            if ( HasCustomDrawnItems() )
                return CDRF_NOTIFYITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT:
            const wxItemAttr* const attr = GetItemAttr(nmcd->dwItemSpec);
            if ( attr )
                return HandleItemPrepaint(*attr, nmcd->hdc);
    }

    return CDRF_DODEFAULT;
}
