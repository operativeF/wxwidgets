///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/statbmpg.cpp
// Purpose:     wxGenericStaticBitmap
// Author:      Marcin Wojdyr, Stefan Csomor
// Created:     2008-06-16
// Copyright:   wxWidgets developers
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_STATBMP

#include "wx/dcclient.h"
#include "wx/generic/statbmpg.h"

#if wxUSE_GRAPHICS_CONTEXT
    #include "wx/graphics.h"
#else
    #include "wx/image.h"
    #include "wx/math.h"
#endif

import <cmath>;

bool wxGenericStaticBitmap::Create(wxWindow *parent,
                                   wxWindowID id,
                                   const wxBitmap& bitmap,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   unsigned int style,
                                   std::string_view name)
{
    if (! wxControl::Create(parent, id, pos, size, style,
                            wxDefaultValidator, name))
        return false;
    m_scaleMode = ScaleMode::None;
    SetBitmap(bitmap);
    Bind(wxEVT_PAINT, &wxGenericStaticBitmap::OnPaint, this);
    return true;
}

void wxGenericStaticBitmap::OnPaint([[maybe_unused]] wxPaintEvent& event)
{
    if ( !m_bitmap.IsOk() )
        return;

    wxPaintDC dc(this);
    const wxSize drawSize = GetClientSize();
    if ( !drawSize.x || !drawSize.y )
        return;

    const wxSize bmpSize = m_bitmap.GetSize();
    double w = 0;
    double h = 0;
    switch ( m_scaleMode )
    {
        case ScaleMode::None:
            dc.DrawBitmap(m_bitmap, 0, 0, true);
            return;
        case ScaleMode::Fill:
            w = drawSize.x;
            h = drawSize.y;
            break;
        case ScaleMode::AspectFill:
        case ScaleMode::AspectFit:
        {
            double scaleFactor;
            double scaleX = (double)drawSize.x / (double)bmpSize.x;
            double scaleY = (double)drawSize.y / (double)bmpSize.y;
            if ( ( m_scaleMode == ScaleMode::AspectFit && scaleY < scaleX ) ||
                 ( m_scaleMode == ScaleMode::AspectFill && scaleY > scaleX ) )
                scaleFactor = scaleY;
            else
                scaleFactor = scaleX;

            w = bmpSize.x * scaleFactor;
            h = bmpSize.y * scaleFactor;

            break;
        }
    }

    wxASSERT_MSG(w, "Unknown scale mode");

    double x = (drawSize.x - w) / 2;
    double y = (drawSize.y - h) / 2;
#if wxUSE_GRAPHICS_CONTEXT
    std::unique_ptr<wxGraphicsContext> const
        gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(dc));
    gc->DrawBitmap(m_bitmap, x, y, w, h);
#else
    wxImage img = m_bitmap.ConvertToImage();
    img.Rescale(std::lround(w), std::lround(h), wxImageResizeQuality::High);
    dc.DrawBitmap(wxBitmap(img), std::lround(x), std::lround(y), true);
#endif
}

// under OSX_cocoa is a define, avoid duplicate info
#ifndef wxGenericStaticBitmap

wxIMPLEMENT_DYNAMIC_CLASS(wxGenericStaticBitmap, wxStaticBitmapBase);

#endif

#endif // wxUSE_STATBMP

