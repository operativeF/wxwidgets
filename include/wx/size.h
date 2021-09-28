#ifndef _WX_SIZE_H
#define _WX_SIZE_H

#include "wx/point.h"

#include <cmath>

struct WXDLLIMPEXP_CORE wxSize
{
    int x {0};
    int y {0};

    // constructors
    constexpr wxSize() noexcept {};
    constexpr wxSize(int xx, int yy) noexcept : x(xx), y(yy) { }

    constexpr wxSize& operator+=(const wxSize& sz) { x += sz.x; y += sz.y; return *this; }
    constexpr wxSize& operator-=(const wxSize& sz) { x -= sz.x; y -= sz.y; return *this; }
    constexpr wxSize& operator/=(int i) { x /= i; y /= i; return *this; }
    constexpr wxSize& operator*=(int i) { x *= i; y *= i; return *this; }
    constexpr wxSize& operator/=(unsigned int i) { x /= i; y /= i; return *this; }
    constexpr wxSize& operator*=(unsigned int i) { x *= i; y *= i; return *this; }
    constexpr wxSize& operator/=(long i) { x /= i; y /= i; return *this; }
    constexpr wxSize& operator*=(long i) { x *= i; y *= i; return *this; }
    constexpr wxSize& operator/=(unsigned long i) { x /= i; y /= i; return *this; }
    constexpr wxSize& operator*=(unsigned long i) { x *= i; y *= i; return *this; }
    wxSize& operator/=(double i) { x = wxRound(x/i); y = wxRound(y/i); return *this; }
    wxSize& operator*=(double i) { x = wxRound(x*i); y = wxRound(y*i); return *this; }

    void IncTo(const wxSize& sz)
        { if ( sz.x > x ) x = sz.x; if ( sz.y > y ) y = sz.y; }
    void DecTo(const wxSize& sz)
        { if ( sz.x < x ) x = sz.x; if ( sz.y < y ) y = sz.y; }
    void DecToIfSpecified(const wxSize& sz)
    {
        if ( sz.x != wxDefaultCoord && sz.x < x )
            x = sz.x;
        if ( sz.y != wxDefaultCoord && sz.y < y )
            y = sz.y;
    }

    void IncBy(int dx, int dy) { x += dx; y += dy; }
    void IncBy(const wxPoint& pt) { IncBy(pt.x, pt.y); }
    void IncBy(const wxSize& sz) { IncBy(sz.x, sz.y); }
    void IncBy(int d) { IncBy(d, d); }

    void DecBy(int dx, int dy) { IncBy(-dx, -dy); }
    void DecBy(const wxPoint& pt) { DecBy(pt.x, pt.y); }
    void DecBy(const wxSize& sz) { DecBy(sz.x, sz.y); }
    void DecBy(int d) { DecBy(d, d); }


    wxSize& Scale(double xscale, double yscale)
        { x = wxRound(x*xscale); y = wxRound(y*yscale); return *this; }

    void Set(int xx, int yy) { x = xx; y = yy; }
    void SetWidth(int w) { x = w; }
    void SetHeight(int h) { y = h; }

    int GetWidth() const { return x; }
    int GetHeight() const { return y; }

    bool IsFullySpecified() const { return x != wxDefaultCoord && y != wxDefaultCoord; }

    // compatibility
    int GetX() const { return x; }
    int GetY() const { return y; }
};

constexpr bool operator==(const wxSize& s1, const wxSize& s2)
{
    return s1.x == s2.x && s1.y == s2.y;
}

constexpr bool operator!=(const wxSize& s1, const wxSize& s2)
{
    return s1.x != s2.x || s1.y != s2.y;
}

constexpr wxSize operator+(const wxSize& s1, const wxSize& s2)
{
    return wxSize(s1.x + s2.x, s1.y + s2.y);
}

constexpr wxSize operator-(const wxSize& s1, const wxSize& s2)
{
    return wxSize(s1.x - s2.x, s1.y - s2.y);
}

constexpr wxSize operator/(const wxSize& s, int i)
{
    return wxSize(s.x / i, s.y / i);
}

constexpr wxSize operator*(const wxSize& s, int i)
{
    return wxSize(s.x * i, s.y * i);
}

constexpr wxSize operator*(int i, const wxSize& s)
{
    return wxSize(s.x * i, s.y * i);
}

constexpr wxSize operator/(const wxSize& s, unsigned int i)
{
    return wxSize(s.x / i, s.y / i);
}

constexpr wxSize operator*(const wxSize& s, unsigned int i)
{
    return wxSize(s.x * i, s.y * i);
}

constexpr wxSize operator*(unsigned int i, const wxSize& s)
{
    return wxSize(s.x * i, s.y * i);
}

constexpr wxSize operator/(const wxSize& s, long i)
{
    return wxSize(s.x / i, s.y / i);
}

constexpr wxSize operator*(const wxSize& s, long i)
{
    return wxSize(int(s.x * i), int(s.y * i));
}

constexpr wxSize operator*(long i, const wxSize& s)
{
    return wxSize(int(s.x * i), int(s.y * i));
}

constexpr wxSize operator/(const wxSize& s, unsigned long i)
{
    return wxSize(int(s.x / i), int(s.y / i));
}

constexpr wxSize operator*(const wxSize& s, unsigned long i)
{
    return wxSize(int(s.x * i), int(s.y * i));
}

constexpr wxSize operator*(unsigned long i, const wxSize& s)
{
    return wxSize(int(s.x * i), int(s.y * i));
}

inline wxSize operator*(const wxSize& s, double i)
{
    return wxSize(wxRound(s.x * i), wxRound(s.y * i));
}

inline wxSize operator*(double i, const wxSize& s)
{
    return wxSize(wxRound(s.x * i), wxRound(s.y * i));
}

#endif // _WX_SIZE_H