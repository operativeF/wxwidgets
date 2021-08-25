/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/region.h
// Purpose:     wxRegion class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) 1997-2002 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_REGION_H_
#define _WX_MSW_REGION_H_

#include <memory>

class WXDLLIMPEXP_CORE wxRegion : public wxRegionWithCombine
{
public:
    wxRegion() = default;
    wxRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h);
    wxRegion(const wxPoint& topLeft, const wxPoint& bottomRight);
    wxRegion(const wxRect& rect);
    wxRegion(WXHRGN hRegion); // Hangs on to this region
    wxRegion(size_t n, const wxPoint *points, wxPolygonFillMode fillStyle = wxODDEVEN_RULE );
#if wxUSE_IMAGE
    explicit wxRegion( const wxBitmap& bmp)
    {
        Union(bmp);
    }
    wxRegion( const wxBitmap& bmp,
              const wxColour& transColour, int tolerance = 0)
    {
        Union(bmp, transColour, tolerance);
    }
#endif // wxUSE_IMAGE

    // m_refData unrefed in ~wxObject
    ~wxRegion() override = default;

    // wxRegionBase methods
    void Clear() override;
    bool IsEmpty() const override;

    // Get internal region handle
    WXHRGN GetHRGN() const;

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

    bool DoIsEqual(const wxRegion& region) const override;
    bool DoGetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const override;
    wxRegionContain DoContainsPoint(wxCoord x, wxCoord y) const override;
    wxRegionContain DoContainsRect(const wxRect& rect) const override;

    bool DoOffset(wxCoord x, wxCoord y) override;
    bool DoCombine(const wxRegion& region, wxRegionOp op) override;

    friend class WXDLLIMPEXP_FWD_CORE wxRegionIterator;

    wxDECLARE_DYNAMIC_CLASS(wxRegion);
};

class WXDLLIMPEXP_CORE wxRegionIterator : public wxObject
{
public:
    wxRegionIterator() = default;
    wxRegionIterator(const wxRegion& region);
    wxRegionIterator(const wxRegionIterator& ri) : wxObject(ri) { *this = ri; }

    wxRegionIterator& operator=(const wxRegionIterator& ri);

    ~wxRegionIterator() override;

    void Reset() { m_current = 0; }
    void Reset(const wxRegion& region);

    bool HaveRects() const { return (m_current < m_numRects); }

    operator bool () const { return HaveRects(); }

    wxRegionIterator& operator++();
    wxRegionIterator operator++(int);

    wxCoord GetX() const;
    wxCoord GetY() const;
    wxCoord GetW() const;
    wxCoord GetWidth() const { return GetW(); }
    wxCoord GetH() const;
    wxCoord GetHeight() const { return GetH(); }

    wxRect GetRect() const { return wxRect(GetX(), GetY(), GetW(), GetH()); }

private:
    long     m_current {0};
    long     m_numRects {0};
    wxRegion m_region;
    std::unique_ptr<wxRect[]>  m_rects;

    wxDECLARE_DYNAMIC_CLASS(wxRegionIterator);
};

#endif // _WX_MSW_REGION_H_
