/////////////////////////////////////////////////////////////////////////////
// Name:      src/msw/region.cpp
// Purpose:   wxRegion implementation using Win32 API
// Author:    Vadim Zeitlin
// Modified by:
// Created:   Fri Oct 24 10:46:34 MET 1997
// Copyright: (c) 1997-2002 wxWidgets team
// Licence:   wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/region.h"

import WX.WinDef;
import WX.Win.UniqueHnd;
import Utils.Geometry;

wxIMPLEMENT_DYNAMIC_CLASS(wxRegion, wxGDIObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxRegionIterator, wxObject);

// ----------------------------------------------------------------------------
// wxRegionRefData implementation
// ----------------------------------------------------------------------------

using namespace msw::utils;

struct wxRegionRefData : public wxGDIRefData
{
    wxRegionRefData() = default;

    wxRegionRefData(const wxRegionRefData& data)  
    {
        WXDWORD noBytes = ::GetRegionData(data.m_region.get(), 0, nullptr);
        RGNDATA *rgnData = (RGNDATA*) new char[noBytes];

        ::GetRegionData(data.m_region.get(), noBytes, rgnData);

        m_region.reset(::ExtCreateRegion(nullptr, noBytes, rgnData));

        delete[] (char*) rgnData;
    }

    wxRegionRefData& operator=(const wxRegionRefData&) = delete;
    wxRegionRefData& operator=(wxRegionRefData&&) = default;
    wxRegionRefData(wxRegionRefData&&) = default;

    unique_region m_region{};
};

#define M_REGION (((wxRegionRefData*)m_refData)->m_region)
#define M_REGION_OF(rgn) (((wxRegionRefData*)(rgn.m_refData))->m_region)

// ============================================================================
// wxRegion implementation
// ============================================================================

// ----------------------------------------------------------------------------
// ctors and dtor
// ----------------------------------------------------------------------------

wxRegion::wxRegion(WXHRGN hRegion)
{
    m_refData = new wxRegionRefData;
    M_REGION.reset(hRegion);
}

static WXHRGN CreateRectRgnMSW(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
    // (x,y) has to represent the left-top corner of the region
    // so if size values are negative we need to recalculate
    // parameters of the region to get (x,y) at this corner.
    if ( w < 0 )
    {
        w = -w;
        x -= (w - 1);
    }
    if ( h < 0 )
    {
        h = -h;
        y -= (h - 1);
    }
    return ::CreateRectRgn(x, y, x + w, y + h);
}

wxRegion::wxRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
    m_refData = new wxRegionRefData;
    M_REGION.reset(CreateRectRgnMSW(x, y, w, h));
}

wxRegion::wxRegion(const wxPoint& topLeft, const wxPoint& bottomRight)
{
    m_refData = new wxRegionRefData;
    M_REGION.reset(CreateRectRgnMSW(topLeft.x, topLeft.y, bottomRight.x-topLeft.x, bottomRight.y-topLeft.y));
}

wxRegion::wxRegion(const wxRect& rect)
{
    m_refData = new wxRegionRefData;
    M_REGION.reset(CreateRectRgnMSW(rect.x, rect.y, rect.width, rect.height));
}

wxRegion::wxRegion(size_t n, const wxPoint *points, wxPolygonFillMode fillStyle)
{
    m_refData = new wxRegionRefData;
    M_REGION.reset(::CreatePolygonRgn
               (
                    reinterpret_cast<const POINT*>(points),
                    n,
                    fillStyle == wxPolygonFillMode::OddEven ? ALTERNATE : WINDING
               ));
}

wxGDIRefData *wxRegion::CreateGDIRefData() const
{
    return new wxRegionRefData;
}

wxGDIRefData *wxRegion::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxRegionRefData(*dynamic_cast<const wxRegionRefData*>(data));
}

// ----------------------------------------------------------------------------
// wxRegion operations
// ----------------------------------------------------------------------------

// Clear current region
void wxRegion::Clear()
{
    UnRef();
}

bool wxRegion::DoOffset(wxCoord x, wxCoord y)
{
    wxCHECK_MSG( GetHRGN(), false, "invalid wxRegion" );

    if ( !x && !y )
    {
        // nothing to do
        return true;
    }

    AllocExclusive();

    if ( ::OffsetRgn(GetHRGN(), x, y) == ERROR )
    {
        wxLogLastError("OffsetRgn");

        return false;
    }

    return true;
}

// combine another region with this one
bool wxRegion::DoCombine(const wxRegion& rgn, wxRegionOp op)
{
    // we can't use the API functions if we don't have a valid region handle
    if ( !m_refData )
    {
        // combining with an empty/invalid region works differently depending
        // on the operation
        switch ( op )
        {
            case wxRegionOp::Copy:
            case wxRegionOp::Or:
            case wxRegionOp::Xor:
                *this = rgn;
                break;

            default:
                wxFAIL_MSG( "unknown region operation" );
                [[fallthrough]];

            case wxRegionOp::And:
            case wxRegionOp::Diff:
                // leave empty/invalid
                return false;
        }
    }
    else // we have a valid region
    {
        AllocExclusive();

        int mode;
        switch ( op )
        {
            case wxRegionOp::And:
                mode = RGN_AND;
                break;

            case wxRegionOp::Or:
                mode = RGN_OR;
                break;

            case wxRegionOp::Xor:
                mode = RGN_XOR;
                break;

            case wxRegionOp::Diff:
                mode = RGN_DIFF;
                break;

            default:
                wxFAIL_MSG( "unknown region operation" );
                [[fallthrough]];

            case wxRegionOp::Copy:
                mode = RGN_COPY;
                break;
        }

        if ( ::CombineRgn(M_REGION.get(), M_REGION.get(), M_REGION_OF(rgn).get(), mode) == ERROR )
        {
            wxLogLastError("CombineRgn");

            return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxRegion bounding box
// ----------------------------------------------------------------------------

// Outer bounds of region
bool wxRegion::DoGetBox(wxCoord& x, wxCoord& y, wxCoord&w, wxCoord &h) const
{
    if (m_refData)
    {
        RECT rect;
        ::GetRgnBox(M_REGION.get(), & rect);
        x = rect.left;
        y = rect.top;
        w = rect.right - rect.left;
        h = rect.bottom - rect.top;

        return true;
    }
    else
    {
        x = y = w = h = 0;

        return false;
    }
}

// Is region empty?
bool wxRegion::IsEmpty() const
{
    wxCoord x, y, w, h;
    GetBox(x, y, w, h);

    return (w == 0) && (h == 0);
}

bool wxRegion::DoIsEqual(const wxRegion& region) const
{
    return ::EqualRgn(M_REGION.get(), M_REGION_OF(region).get()) != 0;
}

// ----------------------------------------------------------------------------
// wxRegion hit testing
// ----------------------------------------------------------------------------

// Does the region contain the point (x,y)?
wxRegionContain wxRegion::DoContainsPoint(wxCoord x, wxCoord y) const
{
    if (!m_refData)
        return wxRegionContain::Outside;

    return ::PtInRegion(M_REGION.get(), (int) x, (int) y) ? wxRegionContain::Inside : wxRegionContain::Outside;
}

// Does the region contain the rectangle (x, y, w, h)?
wxRegionContain wxRegion::DoContainsRect(const wxRect& rect) const
{
    if (!m_refData)
        return wxRegionContain::Outside;

    RECT rc;
    wxCopyRectToRECT(rect, rc);

    return ::RectInRegion(M_REGION.get(), &rc) ? wxRegionContain::Inside : wxRegionContain::Outside;
}

// Get internal region handle
WXHRGN wxRegion::GetHRGN() const
{
    return (WXHRGN)(m_refData ? M_REGION.get() : nullptr);
}

// ============================================================================
// wxRegionIterator implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxRegionIterator ctors/dtor
// ----------------------------------------------------------------------------

// Initialize iterator for region
wxRegionIterator::wxRegionIterator(const wxRegion& region)
{
    Reset(region);
}

wxRegionIterator& wxRegionIterator::operator=(const wxRegionIterator& ri)
{
    if (this == &ri)
        return *this;

    m_current = ri.m_current;
    m_numRects = ri.m_numRects;
    if ( m_numRects )
    {
        m_rects.reset(new wxRect[m_numRects]);

        // FIXME: Copy algorithm
        for ( long n = 0; n < m_numRects; n++ )
            m_rects[n] = ri.m_rects[n];
    }
    else
    {
        m_rects.reset();
    }

    return *this;
}

// ----------------------------------------------------------------------------
// wxRegionIterator operations
// ----------------------------------------------------------------------------

// Reset iterator for a new region.
void wxRegionIterator::Reset(const wxRegion& region)
{
    m_current = 0;
    m_region = region;

    if (m_region.Empty())
    {
        m_rects.reset();
        m_numRects = 0;
    }
    else
    {
        WXDWORD noBytes = ::GetRegionData(((wxRegionRefData*)region.m_refData)->m_region.get(), 0, nullptr);
        
        RGNDATA *rgnData = (RGNDATA*) new char[noBytes];
        ::GetRegionData(((wxRegionRefData*)region.m_refData)->m_region.get(), noBytes, rgnData);

        RGNDATAHEADER* header = (RGNDATAHEADER*) rgnData;

        m_rects.reset(new wxRect[header->nCount]);

        RECT* rect = (RECT*) ((char*)rgnData + sizeof(RGNDATAHEADER));
        for (size_t i = 0; i < header->nCount; i++)
        {
            m_rects[i] = wxRect(rect->left, rect->top,
                                 rect->right - rect->left, rect->bottom - rect->top);
            rect ++; // Advances pointer by sizeof(RECT)
        }

        m_numRects = header->nCount;

        delete[] (char*) rgnData;
    }
}

wxRegionIterator& wxRegionIterator::operator++()
{
    if (m_current < m_numRects)
        ++m_current;

    return *this;
}

wxRegionIterator wxRegionIterator::operator ++ (int)
{
    wxRegionIterator tmp = *this;
    if (m_current < m_numRects)
        ++m_current;

    return tmp;
}

// ----------------------------------------------------------------------------
// wxRegionIterator accessors
// ----------------------------------------------------------------------------

wxCoord wxRegionIterator::GetX() const
{
    wxCHECK_MSG( m_current < m_numRects, 0, "invalid wxRegionIterator" );

    return m_rects[m_current].x;
}

wxCoord wxRegionIterator::GetY() const
{
    wxCHECK_MSG( m_current < m_numRects, 0, "invalid wxRegionIterator" );

    return m_rects[m_current].y;
}

wxCoord wxRegionIterator::GetW() const
{
    wxCHECK_MSG( m_current < m_numRects, 0, "invalid wxRegionIterator" );

    return m_rects[m_current].width;
}

wxCoord wxRegionIterator::GetH() const
{
    wxCHECK_MSG( m_current < m_numRects, 0, "invalid wxRegionIterator" );

    return m_rects[m_current].height;
}
