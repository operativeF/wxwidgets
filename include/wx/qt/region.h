/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/region.h
// Purpose:     header for wxRegion
// Author:      Peter Most, Javier Torres
// Copyright:   (c) Peter Most, Javier Torres
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_REGION_H_
#define _WX_QT_REGION_H_

class QRegion;
class QRect;
template<class T> class QVector;

class wxRegion : public wxRegionBase
{
public:
    wxRegion();
    wxRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h);
    wxRegion(const wxPoint& topLeft, const wxPoint& bottomRight);
    wxRegion(const wxRect& rect);
    wxRegion(size_t n, const wxPoint *points, wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven);
    wxRegion(const wxBitmap& bmp);
    wxRegion(const wxBitmap& bmp, const wxColour& transp, int tolerance = 0);

    bool IsEmpty() const override;
    void Clear() override;

    virtual const QRegion &GetHandle() const;
    virtual void QtSetRegion(QRegion region); // Hangs on to this region

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

    bool DoIsEqual(const wxRegion& region) const override;
    bool DoGetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const override;
    wxRegionContain DoContainsPoint(wxCoord x, wxCoord y) const override;
    wxRegionContain DoContainsRect(const wxRect& rect) const override;

    bool DoOffset(wxCoord x, wxCoord y) override;

    bool DoUnionWithRect(const wxRect& rect) override;
    bool DoUnionWithRegion(const wxRegion& region) override;

    bool DoIntersect(const wxRegion& region) override;
    bool DoSubtract(const wxRegion& region) override;
    bool DoXor(const wxRegion& region) override;

    virtual bool DoCombine(const wxRegion& rgn, wxRegionOp op);

private:
    wxDECLARE_DYNAMIC_CLASS(wxRegion);
};



class wxRegionIterator: public wxObject
{
public:
    wxRegionIterator();
    wxRegionIterator(const wxRegion& region);
    wxRegionIterator(const wxRegionIterator& ri);
    ~wxRegionIterator();

    wxRegionIterator& operator=(const wxRegionIterator& ri);

    void Reset();
    void Reset(const wxRegion& region);

    bool HaveRects() const;
    operator bool () const;

    wxRegionIterator& operator ++ ();
    wxRegionIterator operator ++ (int);

    wxCoord GetX() const;
    wxCoord GetY() const;
    wxCoord GetW() const;
    wxCoord GetWidth() const;
    wxCoord GetH() const;
    wxCoord GetHeight() const;
    wxRect GetRect() const;

private:
    QVector < QRect > *m_qtRects;
    int m_pos;

    wxDECLARE_DYNAMIC_CLASS(wxRegionIterator);
};

#endif // _WX_QT_REGION_H_
