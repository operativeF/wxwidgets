#ifndef _WX_GEOMETRY_RECT_H
#define _WX_GEOMETRY_RECT_H

#include "wx/geometry/point.h"
#include "wx/geometry/size.h"

class WXDLLIMPEXP_CORE wxRect
{
public:
    constexpr wxRect() noexcept {};
    constexpr wxRect(int xx, int yy, int ww, int hh) noexcept
        : x(xx), y(yy), width(ww), height(hh)
        { }
    constexpr wxRect(const wxPoint& topLeft, const wxPoint& bottomRight) noexcept
    {
        x = topLeft.x;
        y = topLeft.y;
        width = bottomRight.x - topLeft.x;
        height = bottomRight.y - topLeft.y;

        if (width < 0)
        {
            width = -width;
            x = bottomRight.x;
        }
        width++;

        if (height < 0)
        {
            height = -height;
            y = bottomRight.y;
        }
        height++;
    }
    constexpr wxRect(const wxPoint& pt, const wxSize& size) noexcept
        : x(pt.x), y(pt.y), width(size.x), height(size.y)
        { }
    constexpr wxRect(const wxSize& size) noexcept
        : x(0), y(0), width(size.x), height(size.y)
        { }

    // default copy ctor and assignment operators ok

    constexpr int GetX() const { return x; }
    constexpr void SetX(int xx) { x = xx; }

    constexpr int GetY() const { return y; }
    constexpr void SetY(int yy) { y = yy; }

    constexpr int GetWidth() const { return width; }
    constexpr void SetWidth(int w) { width = w; }

    constexpr int GetHeight() const { return height; }
    constexpr void SetHeight(int h) { height = h; }

    constexpr wxPoint GetPosition() const { return wxPoint(x, y); }
    constexpr void SetPosition( const wxPoint &p ) { x = p.x; y = p.y; }

    wxSize GetSize() const { return wxSize(width, height); }
    void SetSize( const wxSize &s ) { width = s.x; height = s.y; }

    constexpr bool IsEmpty() const { return (width <= 0) || (height <= 0); }

    constexpr int GetLeft()   const { return x; }
    constexpr int GetTop()    const { return y; }
    constexpr int GetBottom() const { return y + height - 1; }
    constexpr int GetRight()  const { return x + width - 1; }

    constexpr void SetLeft(int left) { x = left; }
    constexpr void SetRight(int right) { width = right - x + 1; }
    constexpr void SetTop(int top) { y = top; }
    constexpr void SetBottom(int bottom) { height = bottom - y + 1; }

    constexpr wxPoint GetTopLeft() const { return GetPosition(); }
    constexpr wxPoint GetLeftTop() const { return GetTopLeft(); }
    constexpr void SetTopLeft(const wxPoint &p) { SetPosition(p); }
    constexpr void SetLeftTop(const wxPoint &p) { SetTopLeft(p); }

    constexpr wxPoint GetBottomRight() const { return wxPoint(GetRight(), GetBottom()); }
    constexpr wxPoint GetRightBottom() const { return GetBottomRight(); }
    constexpr void SetBottomRight(const wxPoint &p) { SetRight(p.x); SetBottom(p.y); }
    constexpr void SetRightBottom(const wxPoint &p) { SetBottomRight(p); }

    constexpr wxPoint GetTopRight() const { return wxPoint(GetRight(), GetTop()); }
    constexpr wxPoint GetRightTop() const { return GetTopRight(); }
    constexpr void SetTopRight(const wxPoint &p) { SetRight(p.x); SetTop(p.y); }
    constexpr void SetRightTop(const wxPoint &p) { SetTopRight(p); }

    constexpr wxPoint GetBottomLeft() const { return wxPoint(GetLeft(), GetBottom()); }
    constexpr wxPoint GetLeftBottom() const { return GetBottomLeft(); }
    constexpr void SetBottomLeft(const wxPoint &p) { SetLeft(p.x); SetBottom(p.y); }
    constexpr void SetLeftBottom(const wxPoint &p) { SetBottomLeft(p); }

    // operations with rect
    constexpr wxRect& Inflate(wxCoord dx, wxCoord dy)
    {
        if (-2*dx>width)
        {
            // Don't allow deflate to eat more width than we have,
            // a well-defined rectangle cannot have negative width.
            x+=width/2;
            width=0;
        }
        else
        {
            // The inflate is valid.
            x-=dx;
            width+=2*dx;
        }

        if (-2*dy>height)
        {
            // Don't allow deflate to eat more height than we have,
            // a well-defined rectangle cannot have negative height.
            y+=height/2;
            height=0;
        }
        else
        {
            // The inflate is valid.
            y-=dy;
            height+=2*dy;
        }

        return *this;
    }

    constexpr wxRect& Inflate(const wxSize& d) { return Inflate(d.x, d.y); }
    constexpr wxRect& Inflate(wxCoord d) { return Inflate(d, d); }
    constexpr wxRect Inflate(wxCoord dx, wxCoord dy) const
    {
        wxRect r = *this;
        r.Inflate(dx, dy);
        return r;
    }

    constexpr wxRect& Deflate(wxCoord dx, wxCoord dy) { return Inflate(-dx, -dy); }
    constexpr wxRect& Deflate(const wxSize& d) { return Inflate(-d.x, -d.y); }
    constexpr wxRect& Deflate(wxCoord d) { return Inflate(-d); }
    constexpr wxRect Deflate(wxCoord dx, wxCoord dy) const
    {
        wxRect r = *this;
        r.Deflate(dx, dy);
        return r;
    }

    constexpr void Offset(wxCoord dx, wxCoord dy) { x += dx; y += dy; }
    constexpr void Offset(const wxPoint& pt) { Offset(pt.x, pt.y); }

    constexpr wxRect& Intersect(const wxRect& rect)
    {
        int x2 = GetRight(),
            y2 = GetBottom();

        if ( x < rect.x )
            x = rect.x;
        if ( y < rect.y )
            y = rect.y;
        if ( x2 > rect.GetRight() )
            x2 = rect.GetRight();
        if ( y2 > rect.GetBottom() )
            y2 = rect.GetBottom();

        width = x2 - x + 1;
        height = y2 - y + 1;

        if ( width <= 0 || height <= 0 )
        {
            width =
            height = 0;
        }

        return *this;
    }

    constexpr wxRect Intersect(const wxRect& rect) const
    {
        wxRect r = *this;
        r.Intersect(rect);
        return r;
    }

    constexpr wxRect& Union(const wxRect& rect)
    {
        // ignore empty rectangles: union with an empty rectangle shouldn't extend
        // this one to (0, 0)
        if ( !width || !height )
        {
            *this = rect;
        }
        else if ( rect.width && rect.height )
        {
            const int x1 = std::min(x, rect.x);
            const int y1 = std::min(y, rect.y);
            const int y2 = std::max(y + height, rect.height + rect.y);
            const int x2 = std::max(x + width, rect.width + rect.x);

            x = x1;
            y = y1;
            width = x2 - x1;
            height = y2 - y1;
        }
        //else: we're not empty and rect is empty

        return *this;
    }

    constexpr wxRect Union(const wxRect& rect) const
    {
        wxRect r = *this;
        r.Union(rect);
        return r;
    }

    // return true if the point is (not strictly) inside the rect
    constexpr bool Contains(int cx, int cy) const
    {
        return ( (cx >= x) && (cy >= y)
          && ((cy - y) < height)
          && ((cx - x) < width)
          );
    }

    constexpr bool Contains(const wxPoint& pt) const { return Contains(pt.x, pt.y); }
    // return true if the rectangle 'rect' is (not strictly) inside this rect
    constexpr bool Contains(const wxRect& rect) const
    {
        return Contains(rect.GetTopLeft()) && Contains(rect.GetBottomRight());
    }

    // return true if the rectangles have a non empty intersection
    constexpr bool Intersects(const wxRect& rect) const
    {
        const wxRect r = Intersect(rect);

        // if there is no intersection, both width and height are 0
        return r.width != 0;
    }

    // like Union() but don't ignore empty rectangles
    constexpr wxRect& operator+=(const wxRect& rect);

    // intersections of two rectrangles not testing for empty rectangles
    constexpr wxRect& operator*=(const wxRect& rect);

    // centre this rectangle in the given (usually, but not necessarily,
    // larger) one
    constexpr wxRect CentreIn(const wxRect& r, int dir = wxBOTH) const
    {
        return wxRect(dir & wxHORIZONTAL ? r.x + (r.width - width)/2 : x,
                      dir & wxVERTICAL ? r.y + (r.height - height)/2 : y,
                      width, height);
    }

    constexpr wxRect CenterIn(const wxRect& r, int dir = wxBOTH) const
    {
        return CentreIn(r, dir);
    }

public:
    int x{0}, y{0}, width{0}, height{0};
};


// compare rectangles
constexpr bool operator==(const wxRect& r1, const wxRect& r2)
{
    return (r1.x == r2.x) && (r1.y == r2.y) &&
           (r1.width == r2.width) && (r1.height == r2.height);
}

constexpr bool operator!=(const wxRect& r1, const wxRect& r2)
{
    return !(r1 == r2);
}

// like Union() but don't treat empty rectangles specifically
constexpr WXDLLIMPEXP_CORE wxRect operator+(const wxRect& r1, const wxRect& r2)
{
    const int x1 = std::min(r1.x, r2.x);
    const int y1 = std::min(r1.y, r2.y);
    const int y2 = std::max(r1.y+r1.height, r2.height+r2.y);
    const int x2 = std::max(r1.x+r1.width, r2.width+r2.x);

    return {x1, y1, x2 - x1, y2 - y1};
}

// intersections of two rectangles
constexpr WXDLLIMPEXP_CORE wxRect operator*(const wxRect& r1, const wxRect& r2)
{
    const int x1 = std::max(r1.x, r2.x);
    const int y1 = std::max(r1.y, r2.y);
    const int y2 = std::min(r1.y+r1.height, r2.height+r2.y);
    const int x2 = std::min(r1.x+r1.width, r2.width+r2.x);

    return {x1, y1, x2 - x1, y2 - y1};
}

constexpr wxRect& wxRect::operator+=(const wxRect& rect)
{
    *this = *this + rect;
    return *this;
}

constexpr wxRect& wxRect::operator*=(const wxRect& rect)
{
    *this = *this * rect;
    return *this;
}

#endif // _WX_GEOMETRY_RECT_H
