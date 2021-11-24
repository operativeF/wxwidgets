module;

#include "wx/defs.h"

export module Utils.Geometry.Rect;

import WX.Cfg.Flags;

import Utils.Geometry.Point;
import Utils.Geometry.Size;

export
{

template<typename T>
struct wxRect2D
{
    using value_type = T;

    constexpr wxRect2D() noexcept {}

    constexpr wxRect2D(T xx, T yy, T ww, T hh) noexcept
        : x{xx}, y{yy}, width{ww}, height{hh}
    {}

    constexpr wxRect2D(const wxPoint2D<T>& topLeft, const wxPoint2D<T>& bottomRight) noexcept
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

    constexpr wxRect2D(const wxPoint2D<T>& pt, const wxSize& size) noexcept
        : x{pt.x}, y{pt.y}, width{size.x}, height{size.y}
    {}

    constexpr wxRect2D(const wxSize& size) noexcept
        : x{0}, y{0}, width{size.x}, height{size.y}
    {}

    // like Union() but don't ignore empty rectangles
    constexpr wxRect2D& operator+=(const wxRect2D& otherRect) noexcept
    {
        const T x1 = std::min(x, otherRect.x);
        const T y1 = std::min(y, otherRect.y);
        const T y2 = std::max(y + height, otherRect.height + otherRect.y);
        const T x2 = std::max(x + width, otherRect.width + otherRect.x);

        x = x1;
        y = y1;
        width = x2 - x1;
        height = y2 - y1;

        return *this;
    }

    // intersections of two rectrangles not testing for empty rectangles
    constexpr wxRect2D& operator*=(const wxRect2D& otherRect) noexcept
    {
        const T x1 = std::max(x, otherRect.x);
        const T y1 = std::max(y, otherRect.y);
        const T y2 = std::min(y + height, otherRect.height + otherRect.y);
        const T x2 = std::min(x + width, otherRect.width + otherRect.x);

        x = x1;
        y = y1;
        width = x2 - x1;
        height = y2 - y1;

        return *this;
    }

    constexpr T GetX() const noexcept { return x; }
    constexpr void SetX(T xx) noexcept { x = xx; }

    constexpr T GetY() const noexcept { return y; }
    constexpr void SetY(T yy) noexcept { y = yy; }

    constexpr int GetWidth() const noexcept { return width; }
    constexpr void SetWidth(int w) noexcept { width = w; }

    constexpr int GetHeight() const noexcept { return height; }
    constexpr void SetHeight(T h) noexcept { height = h; }

    constexpr wxPoint GetPosition() const noexcept { return wxPoint{x, y}; }
    constexpr void SetPosition( const wxPoint &p ) noexcept { x = p.x; y = p.y; }

    constexpr wxSize GetSize() const noexcept { return wxSize{width, height}; }
    constexpr void SetSize( const wxSize &s ) noexcept { width = s.x; height = s.y; }

    constexpr bool IsEmpty() const noexcept { return (width <= value_type{0}) || (height <= value_type{0}); }

    constexpr T GetLeft()   const noexcept { return x; }
    constexpr T GetTop()    const noexcept { return y; }
    constexpr T GetBottom() const noexcept { return y + height - 1; }
    constexpr T GetRight()  const noexcept { return x + width - 1; }

    constexpr void SetLeft(T left) noexcept { x = left; }
    constexpr void SetRight(T right) noexcept { width = right - x + 1; }
    constexpr void SetTop(T top) noexcept { y = top; }
    constexpr void SetBottom(T bottom) noexcept { height = bottom - y + 1; }

    constexpr wxPoint2D<T> GetTopLeft() const noexcept { return GetPosition(); }
    constexpr wxPoint2D<T> GetLeftTop() const noexcept { return GetTopLeft(); }
    constexpr void SetTopLeft(const wxPoint2D<T>& p) noexcept { SetPosition(p); }
    constexpr void SetLeftTop(const wxPoint2D<T>& p) noexcept { SetTopLeft(p); }

    constexpr wxPoint2D<T> GetBottomRight() const { return {GetRight(), GetBottom()}; }
    constexpr wxPoint2D<T> GetRightBottom() const { return GetBottomRight(); }
    constexpr void SetBottomRight(const wxPoint2D<T>& p) { SetRight(p.x); SetBottom(p.y); }
    constexpr void SetRightBottom(const wxPoint2D<T>& p) { SetBottomRight(p); }

    constexpr wxPoint2D<T> GetTopRight() const noexcept { return {GetRight(), GetTop()}; }
    constexpr wxPoint2D<T> GetRightTop() const noexcept { return GetTopRight(); }
    constexpr void SetTopRight(const wxPoint2D<T>& p) noexcept { SetRight(p.x); SetTop(p.y); }
    constexpr void SetRightTop(const wxPoint2D<T>& p) noexcept { SetTopRight(p); }

    constexpr wxPoint2D<T> GetBottomLeft() const noexcept { return {GetLeft(), GetBottom()}; }
    constexpr wxPoint2D<T> GetLeftBottom() const noexcept { return GetBottomLeft(); }
    constexpr void SetBottomLeft(const wxPoint2D<T>& p) noexcept { SetLeft(p.x); SetBottom(p.y); }
    constexpr void SetLeftBottom(const wxPoint2D<T>& p) noexcept { SetBottomLeft(p); }

    // operations with rect
    constexpr wxRect2D& Inflate(T dx, T dy) noexcept
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

    constexpr wxRect2D& Inflate(const wxSize& d) noexcept { return Inflate(d.x, d.y); }
    constexpr wxRect2D& Inflate(T d) noexcept { return Inflate(d, d); }
    constexpr wxRect2D Inflate(T dx, T dy) const noexcept 
    {
        wxRect2D r = *this;
        r.Inflate(dx, dy);
        return r;
    }

    constexpr wxRect2D& Deflate(T dx, T dy) noexcept { return Inflate(-dx, -dy); }
    constexpr wxRect2D& Deflate(const wxSize& d) noexcept { return Inflate(value_type{-d.x}, value_type{-d.y}); }
    constexpr wxRect2D& Deflate(T d) noexcept { return Inflate(-d); }
    constexpr wxRect2D Deflate(T dx, T dy) const noexcept
    {
        wxRect2D r = *this;
        r.Deflate(dx, dy);
        return r;
    }

    constexpr void Offset(T dx, T dy) noexcept { x += dx; y += dy; }
    constexpr void Offset(const wxPoint2D<T>& pt) noexcept { Offset(pt.x, pt.y); }

    constexpr wxRect2D& Intersect(const wxRect2D& rect) noexcept
    {
        T x2 = GetRight();
        T y2 = GetBottom();

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

        if ( width <= value_type{0} || height <= value_type{0} )
        {
            width = value_type{0};
            height = value_type{0};
        }

        return *this;
    }

    constexpr wxRect2D Intersect(const wxRect2D& rect) const noexcept
    {
        wxRect2D r = *this;
        r.Intersect(rect);
        return r;
    }

    constexpr wxRect2D& Union(const wxRect2D& rect) noexcept
    {
        // ignore empty rectangles: union with an empty rectangle shouldn't extend
        // this one to (0, 0)
        if ( width == value_type{0} || height == value_type{0} )
        {
            *this = rect;
        }
        else if ( rect.width && rect.height )
        {
            const T x1 = std::min(x, rect.x);
            const T y1 = std::min(y, rect.y);
            const T y2 = std::max(y + height, rect.height + rect.y);
            const T x2 = std::max(x + width, rect.width + rect.x);

            x = x1;
            y = y1;
            width = x2 - x1;
            height = y2 - y1;
        }
        //else: we're not empty and rect is empty

        return *this;
    }

    constexpr wxRect2D Union(const wxRect2D& rect) const noexcept
    {
        wxRect2D r = *this;
        r.Union(rect);
        return r;
    }

    // return true if the point is (not strictly) inside the rect
    constexpr bool Contains(T cx, T cy) const noexcept
    {
        return ( (cx >= x) && (cy >= y)
              && ((cy - y) < height)
              && ((cx - x) < width)
              );
    }

    constexpr bool Contains(const wxPoint2D<T>& pt) const noexcept { return Contains(pt.x, pt.y); }
    // return true if the rectangle 'rect' is (not strictly) inside this rect
    constexpr bool Contains(const wxRect2D& rect) const noexcept
    {
        return Contains(rect.GetTopLeft()) && Contains(rect.GetBottomRight());
    }

    // return true if the rectangles have a non empty intersection
    constexpr bool Intersects(const wxRect2D& rect) const noexcept
    {
        const auto r = Intersect(rect);

        // if there is no intersection, both width and height are 0
        return r.width != value_type{0};
    }

    // centre this rectangle in the given (usually, but not necessarily,
    // larger) one
    constexpr wxRect2D CentreIn(const wxRect2D& r, int dir = wxBOTH) const noexcept
    {
        return {dir & wxHORIZONTAL ? r.x + (r.width - width) / value_type{2} : x,
                dir & wxVERTICAL ? r.y + (r.height - height) / value_type{2} : y,
                width,
                height};
    }

    constexpr wxRect2D CenterIn(const wxRect2D& r, int dir = wxBOTH) const noexcept
    {
        return CentreIn(r, dir);
    }

    T x{};
    T y{};
    T width{};
    T height{};
};

template<typename T>
constexpr wxRect2D<T> operator+(wxRect2D<T> rect, const wxRect2D<T>& otherRect) noexcept
{
    rect += otherRect;
    return rect;
}

template<typename T>
constexpr wxRect2D<T> operator*(wxRect2D<T> rect, const wxRect2D<T>& otherRect) noexcept
{
    rect *= otherRect;
    return rect;
}

// compare rectangles
template<typename T>
constexpr bool operator==(const wxRect2D<T>& r1, const wxRect2D<T>& r2) noexcept
{
    return (r1.x == r2.x) && (r1.y == r2.y) &&
           (r1.width == r2.width) && (r1.height == r2.height);
}

template<typename T>
constexpr bool operator!=(const wxRect2D<T>& r1, const wxRect2D<T>& r2) noexcept
{
    return !(r1 == r2);
}

using wxRect       = wxRect2D<int>;
using wxRectDouble = wxRect2D<double>;
using wxRectFloat  = wxRect2D<float>;

} // export
