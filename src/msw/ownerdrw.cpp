///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/ownerdrw.cpp
// Purpose:     implementation of wxOwnerDrawn class
// Author:      Vadim Zeitlin
// Modified by: Marcin Malich
// Created:     13.11.97
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_OWNER_DRAWN

#include "wx/msw/private.h"
#include "wx/msw/wrapcctl.h"            // for HIMAGELIST

#include "wx/ownerdrw.h"
#include "wx/msw/dc.h"
#include "wx/msw/private/dc.h"

import WX.Win.UniqueHnd;

// ============================================================================
// implementation of wxOwnerDrawn class
// ============================================================================

using msw::utils::unique_brush;

// draw the item
bool wxOwnerDrawn::OnDrawItem(wxDC& dc, const wxRect& rc,
                              wxODAction, wxODStatus stat)
{
    // we do nothing if item isn't ownerdrawn
    if ( !IsOwnerDrawn() )
        return true;

    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    WXHDC hdc = GetHdcOf(*impl);

    RECT rect;
    wxCopyRectToRECT(rc, rect);

    {
        // set the font and colors
        wxFont font;
        GetFontToUse(font);

        wxColour colText, colBack;
        GetColourToUse(stat, colText, colBack);

        SelectInHDC selFont(hdc, GetHfontOf(font));

        wxMSWImpl::wxTextColoursChanger textCol(hdc, colText, colBack);
        wxMSWImpl::wxBkModeChanger bkMode(hdc, wxBrushStyle::Transparent);

        auto hbr = unique_brush(::CreateSolidBrush(wxColourToPalRGB(colBack)));
        SelectInHDC selBrush(hdc, hbr.get());

        ::FillRect(hdc, &rect, hbr.get());

        // using native API because it recognizes '&'

        std::string text = GetName();

        boost::nowide::wstackstring stackText{text.c_str()};
        SIZE sizeRect;
        ::GetTextExtentPoint32W(hdc, stackText.get(), stackText.buffer_size, &sizeRect);

        unsigned int flags = DST_PREFIXTEXT;
        if ( (stat & wxODDisabled) && !(stat & wxODSelected) )
            flags |= DSS_DISABLED;

        if ( (stat & wxODHidePrefix) )
            flags |= DSS_HIDEPREFIX;

        const int x = rc.x + GetMarginWidth();
        const int y = rc.y + (rc.GetHeight() - sizeRect.cy) / 2;
        const int cx = rc.GetWidth() - GetMarginWidth();
        const int cy = sizeRect.cy;

        ::DrawStateW(hdc, nullptr, nullptr, reinterpret_cast<LPARAM>(stackText.get()),
                    text.length(), x, y, cx, cy, flags);

    } // reset to default the font, colors and brush

    if (stat & wxODHasFocus)
        ::DrawFocusRect(hdc, &rect);

    return true;
}

// ----------------------------------------------------------------------------
// global helper functions implemented here
// ----------------------------------------------------------------------------

BOOL wxDrawStateBitmap(WXHDC hDC, WXHBITMAP hBitmap, int x, int y, WXUINT uState)
{
    // determine size of bitmap image
    BITMAP bmp;
    if ( !::GetObjectW(hBitmap, sizeof(BITMAP), &bmp) )
        return FALSE;

    BOOL result;

    switch ( uState )
    {
        case wxDSB_NORMAL:
        case wxDSB_SELECTED:
            {
                // uses image list functions to draw
                //  - normal bitmap with support transparency
                //    (image list internally create mask etc.)
                //  - blend bitmap with the background colour
                //    (like default selected items)
                HIMAGELIST hIml = ::ImageList_Create(bmp.bmWidth, bmp.bmHeight,
                                                     ILC_COLOR32 | ILC_MASK, 1, 1);
                ::ImageList_Add(hIml, hBitmap, nullptr);
                WXUINT fStyle = uState == wxDSB_SELECTED ? ILD_SELECTED : ILD_NORMAL;
                result = ::ImageList_Draw(hIml, 0, hDC, x, y, fStyle);
                ::ImageList_Destroy(hIml);
            }
            break;

        case wxDSB_DISABLED:
            result = ::DrawStateW(hDC, nullptr, nullptr, (WXLPARAM)hBitmap, 0, x, y,
                                 bmp.bmWidth, bmp.bmHeight,
                                 DST_BITMAP | DSS_DISABLED);
            break;

        default:
            wxFAIL_MSG( "DrawStateBitmap: unknown wxDSBStates value" );
            result = FALSE;
    }

    return result;
}

#endif // wxUSE_OWNER_DRAWN
