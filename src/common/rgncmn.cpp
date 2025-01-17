/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/rgncmn.cpp
// Purpose:     Methods of wxRegion that have a generic implementation
// Author:      Robin Dunn
// Modified by:
// Created:     27-Mar-2003
// Copyright:   (c) Robin Dunn
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/region.h"
#include "wx/dcmemory.h"
#include "wx/bitmap.h"
#include "wx/utils.h"

import WX.Image;

// ============================================================================
// wxRegionBase implementation
// ============================================================================

// ----------------------------------------------------------------------------
// region comparison
// ----------------------------------------------------------------------------

bool wxRegionBase::IsEqual(const wxRegion& region) const
{
    if ( m_refData == region.GetRefData() )
    {
        // regions are identical, hence equal
        return true;
    }

    if ( !m_refData || !region.GetRefData() )
    {
        // one, but not both, of the regions is invalid
        return false;
    }

    return DoIsEqual(region);
}

// ----------------------------------------------------------------------------
// region to/from bitmap conversions
// ----------------------------------------------------------------------------

wxBitmap wxRegionBase::ConvertToBitmap() const
{
    wxRect box = GetBox();
    wxBitmap bmp(wxSize{box.GetRight() + 1, box.GetBottom() + 1});
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();
    dc.SetDeviceClippingRegion(*dynamic_cast<const wxRegion *>(this));
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SelectObject(wxNullBitmap);
    return bmp;
}

#if wxUSE_IMAGE

namespace
{

bool DoRegionUnion(wxRegionBase& region,
                   const wxImage& image,
                   unsigned char loR,
                   unsigned char loG,
                   unsigned char loB,
                   int tolerance)
{
    unsigned char hiR = (unsigned char)std::min(0xFF, loR + tolerance);
    unsigned char hiG = (unsigned char)std::min(0xFF, loG + tolerance);
    unsigned char hiB = (unsigned char)std::min(0xFF, loB + tolerance);

    // Loop through the image row by row, pixel by pixel, building up
    // rectangles to add to the region.
    int width = image.GetWidth();
    int height = image.GetHeight();
    for (int y=0; y < height; y++)
    {
        wxRect rect;
        rect.y = y;
        rect.height = 1;

        for (int x=0; x < width; x++)
        {
            // search for a continuous range of non-transparent pixels
            int x0 = x;
            while ( x < width)
            {
                unsigned char R = image.GetRed(x,y);
                unsigned char G = image.GetGreen(x,y);
                unsigned char B = image.GetBlue(x,y);
                if (( R >= loR && R <= hiR) &&
                    ( G >= loG && G <= hiG) &&
                    ( B >= loB && B <= hiB))  // It's transparent
                    break;
                x++;
            }

            // Add the run of non-transparent pixels (if any) to the region
            if (x > x0) {
                rect.x = x0;
                rect.width = x - x0;
                region.Union(rect);
            }
        }
    }

    return true;
}

} // namespace anonymous

bool wxRegionBase::Union(const wxBitmap& bmp)
{
    if (bmp.GetMask())
    {
        wxImage image = bmp.ConvertToImage();
        wxASSERT_MSG( image.HasMask(), "wxBitmap::ConvertToImage doesn't preserve mask?" );
        return DoRegionUnion(*this, image,
                             image.GetMaskRed(),
                             image.GetMaskGreen(),
                             image.GetMaskBlue(),
                             0);
    }
    else
    {
        return Union(0, 0, bmp.GetWidth(), bmp.GetHeight());
    }
}

bool wxRegionBase::Union(const wxBitmap& bmp,
                         const wxColour& transColour,
                         int   tolerance)
{
    wxImage image = bmp.ConvertToImage();
    return DoRegionUnion(*this, image,
                         transColour.Red(),
                         transColour.Green(),
                         transColour.Blue(),
                         tolerance);
}

#endif // wxUSE_IMAGE

#ifdef wxHAS_REGION_COMBINE
// ============================================================================
// wxRegionWithCombine
// ============================================================================

// implement some wxRegionBase pure virtuals in terms of Combine()
bool wxRegionWithCombine::DoUnionWithRect(const wxRect& rect)
{
    return Combine(rect, wxRegionOp::Or);
}

bool wxRegionWithCombine::DoUnionWithRegion(const wxRegion& region)
{
    return DoCombine(region, wxRegionOp::Or);
}

bool wxRegionWithCombine::DoIntersect(const wxRegion& region)
{
    return DoCombine(region, wxRegionOp::And);
}

bool wxRegionWithCombine::DoSubtract(const wxRegion& region)
{
    return DoCombine(region, wxRegionOp::Diff);
}

bool wxRegionWithCombine::DoXor(const wxRegion& region)
{
    return DoCombine(region, wxRegionOp::Xor);
}

#endif // wxHAS_REGION_COMBINE
