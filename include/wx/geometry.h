/////////////////////////////////////////////////////////////////////////////
// Name:        wx/geometry.h
// Purpose:     Common Geometry Classes
// Author:      Stefan Csomor
// Modified by:
// Created:     08/05/99
// Copyright:   (c) 1999 Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GEOMETRY_H_
#define _WX_GEOMETRY_H_

#include "wx/defs.h"

#if wxUSE_GEOMETRY

#include "wx/utils.h"
#include "wx/gdicmn.h"

#include <cstdint>

class WXDLLIMPEXP_FWD_BASE wxDataInputStream;
class WXDLLIMPEXP_FWD_BASE wxDataOutputStream;

// clipping from Cohen-Sutherland

enum wxOutCode
{
    wxInside = 0x00 ,
    wxOutLeft = 0x01 ,
    wxOutRight = 0x02 ,
    wxOutTop = 0x08 ,
    wxOutBottom = 0x04
};

template<typename NumericalT>
struct WXDLLIMPEXP_CORE wxPoint2D
{
    using value_type = NumericalT;

    constexpr wxPoint2D() noexcept {}

    constexpr wxPoint2D(const wxPoint& pt) noexcept : m_x(pt.x), m_y(pt.y) {}

    constexpr wxPoint2D(value_type a, value_type b) noexcept
        : m_x{a},
          m_y{b}
    {
    }

    constexpr operator wxPoint() const noexcept
    {
        return wxPoint{m_x, m_y};
    }

    constexpr bool operator==(const wxPoint2D<value_type>& b) const noexcept
    {
        return m_x == b.m_x && m_y == b.m_y;
    }

    constexpr bool operator!=(const wxPoint2D<value_type>& b) const noexcept
    {
        return !(*this == b);
    }

    template<typename ScalarT = value_type>
    constexpr wxPoint2D<value_type>& operator*=(ScalarT val) noexcept
    {
        m_x *= val;
        m_y *= val;

        return *this;
    }

    
    template<typename ScalarT = value_type>
    constexpr wxPoint2D<value_type>& operator/=(ScalarT val) noexcept
    {
        m_x /= val;
        m_y /= val;

        return *this;
    }

    template<typename NumericalU = value_type>
    constexpr wxPoint2D<value_type>& operator-=(const wxPoint2D<NumericalU>& val) noexcept
    {
        m_x -= val.m_x;
        m_y -= val.m_y;

        return *this;
    }

    template<typename NumericalU = value_type>
    constexpr wxPoint2D<value_type>& operator+=(const wxPoint2D<NumericalU>& val) noexcept
    {
        m_x += val.m_x;
        m_y += val.m_y;

        return *this;
    }

    constexpr wxPoint2D<value_type> operator-() const noexcept
    {
        return {-m_x, -m_y};
    }

    // TODO: Add fmt stream operators.

    value_type m_x{};
    value_type m_y{};
};

template<typename NumericalT, typename NumericalU = NumericalT>
constexpr auto VectorDotProduct(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return vecA.m_x * vecB.m_x + vecA.m_y * vecB.m_y;
}

template<typename NumericalT, typename NumericalU = NumericalT>
constexpr auto VectorCrossProduct(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return vecA.m_x * vecB.m_y - vecB.m_x * vecA.m_y;
}

template<typename NumericalT>
auto GetVectorLength(const wxPoint2D<NumericalT>& vec) noexcept
{
    return std::hypot(vec.m_x, vec.m_y);
}

template<typename NumericalT>
auto GetVectorAngle(const wxPoint2D<NumericalT>& vec) noexcept
{
    // FIXME: Double equality
    if ( vec.m_x == 0.0 )
    {
        if ( vec.m_y >= 0 )
            return 90.0;
        else
            return 270.0;
    }
    // FIXME: Double equality
    if ( vec.m_y == 0.0 )
    {
        if ( vec.m_x >= 0 )
            return 0.0;
        else
            return 180.0;
    }
    
    auto deg = wxRadToDeg(std::atan2( vec.m_y , vec.m_x ));

    if ( deg < 0.0 )
    {
        deg += 360.0;
    }
    
    return deg;
}

template<typename NumericalT>
void SetVectorAngle(wxPoint2D<NumericalT>& vec, auto angle) noexcept
{
    const auto length = GetLength(vec);

    vec.m_x = length * std::cos(wxDegToRad(angle));
    vec.m_y = length * std::sin(wxDegToRad(angle));
}

template<typename NumericalT>
void SetVectorLength(wxPoint2D<NumericalT>& vec, auto length) noexcept
{
    const auto priorLength = GetVectorLength(vec);

    vec.m_x = vec.m_x * length / priorLength;
    vec.m_y = vec.m_y * length / priorLength;
}

template<typename NumericalT>
constexpr void Normalize(wxPoint2D<NumericalT>& vec) noexcept
{
    SetVectorLength(vec, 1.0);
}

template<typename NumericalT, typename NumericalU = NumericalT>
auto GetVectorDistance(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return std::hypot(vecA.m_x - vecB.m_x, vecA.m_y - vecB.m_y);
}

template<typename NumericalT, typename NumericalU = NumericalT>
auto GetVectorDistanceSquare(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return std::pow(GetDistance(vecA, vecB), 2.0); 
}

template<typename NumericalT>
constexpr wxPoint2D<NumericalT> operator+(wxPoint2D<NumericalT> vecA, const wxPoint2D<NumericalT>& vecB) noexcept
{
    vecA += vecB;
    return vecA;
}

template<typename NumericalT>
constexpr wxPoint2D<NumericalT> operator-(wxPoint2D<NumericalT> vecA, const wxPoint2D<NumericalT>& vecB) noexcept
{
    vecA -= vecB;
    return vecA;
}

template<typename NumericalT, typename ScalarT = NumericalT>
constexpr wxPoint2D<NumericalT> operator*(wxPoint2D<NumericalT> vec, ScalarT val) noexcept
{
    vec *= val;
    return vec;
}

template<typename NumericalT, typename ScalarT = NumericalT>
constexpr wxPoint2D<NumericalT> operator*(ScalarT val, const wxPoint2D<NumericalT>& vec) noexcept
{
    return vec * val;
}

template<typename NumericalT, typename ScalarT = NumericalT>
constexpr wxPoint2D<NumericalT>  operator/(wxPoint2D<NumericalT> vec, ScalarT val) noexcept
{
    vec /= val;
    return vec;
}

using wxPoint2DInt = wxPoint2D<int>;
using wxPoint2DDouble = wxPoint2D<double>;

// wxRect2Ds are a axis-aligned rectangles, each side of the rect is parallel to the x- or m_y- axis. The rectangle is either defined by the
// top left and bottom right corner, or by the top left corner and size. A point is contained within the rectangle if
// left <= x < right  and top <= m_y < bottom , thus it is a half open interval.

class WXDLLIMPEXP_CORE wxRect2DDouble
{
public:
    wxRect2DDouble()
        { m_x = m_y = m_width = m_height = 0; }
    wxRect2DDouble(double x, double y, double w, double h)
        { m_x = x; m_y = y; m_width = w;  m_height = h; }
/*
    wxRect2DDouble(const wxPoint2DDouble& topLeft, const wxPoint2DDouble& bottomRight);
    wxRect2DDouble(const wxPoint2DDouble& pos, const wxSize& size);
    wxRect2DDouble(const wxRect2DDouble& rect);
*/
        // single attribute accessors

    wxPoint2DDouble GetPosition() const
        { return wxPoint2DDouble(m_x, m_y); }
    wxSize GetSize() const
        { return wxSize((int) m_width, (int) m_height); }

    // for the edge and corner accessors there are two setters counterparts, the Set.. functions keep the other corners at their
        // position whenever sensible, the Move.. functions keep the size of the rect and move the other corners appropriately

    inline double GetLeft() const { return m_x; }
    inline void SetLeft( double n ) { m_width += m_x - n; m_x = n; }
    inline void MoveLeftTo( double n ) { m_x = n; }
    inline double GetTop() const { return m_y; }
    inline void SetTop( double n ) { m_height += m_y - n; m_y = n; }
    inline void MoveTopTo( double n ) { m_y = n; }
    inline double GetBottom() const { return m_y + m_height; }
    inline void SetBottom( double n ) { m_height += n - (m_y+m_height);}
    inline void MoveBottomTo( double n ) { m_y = n - m_height; }
    inline double GetRight() const { return m_x + m_width; }
    inline void SetRight( double n ) { m_width += n - (m_x+m_width) ; }
    inline void MoveRightTo( double n ) { m_x = n - m_width; }

    inline wxPoint2DDouble GetLeftTop() const
        { return wxPoint2DDouble( m_x , m_y ); }
    inline void SetLeftTop( const wxPoint2DDouble &pt )
        { m_width += m_x - pt.m_x; m_height += m_y - pt.m_y; m_x = pt.m_x; m_y = pt.m_y; }
    inline void MoveLeftTopTo( const wxPoint2DDouble &pt )
        { m_x = pt.m_x; m_y = pt.m_y; }
    inline wxPoint2DDouble GetLeftBottom() const
        { return wxPoint2DDouble( m_x , m_y + m_height ); }
    inline void SetLeftBottom( const wxPoint2DDouble &pt )
        { m_width += m_x - pt.m_x; m_height += pt.m_y - (m_y+m_height) ; m_x = pt.m_x; }
    inline void MoveLeftBottomTo( const wxPoint2DDouble &pt )
        { m_x = pt.m_x; m_y = pt.m_y - m_height; }
    inline wxPoint2DDouble GetRightTop() const
        { return wxPoint2DDouble( m_x+m_width , m_y ); }
    inline void SetRightTop( const wxPoint2DDouble &pt )
        { m_width += pt.m_x - ( m_x + m_width ); m_height += m_y - pt.m_y; m_y = pt.m_y; }
    inline void MoveRightTopTo( const wxPoint2DDouble &pt )
        { m_x = pt.m_x - m_width; m_y = pt.m_y; }
    inline wxPoint2DDouble GetRightBottom() const
        { return wxPoint2DDouble( m_x+m_width , m_y + m_height ); }
    inline void SetRightBottom( const wxPoint2DDouble &pt )
        { m_width += pt.m_x - ( m_x + m_width ); m_height += pt.m_y - (m_y+m_height);}
    inline void MoveRightBottomTo( const wxPoint2DDouble &pt )
        { m_x = pt.m_x - m_width; m_y = pt.m_y - m_height; }
    inline wxPoint2DDouble GetCentre() const
        { return wxPoint2DDouble( m_x+m_width/2 , m_y+m_height/2 ); }
    inline void SetCentre( const wxPoint2DDouble &pt )
        { MoveCentreTo( pt ); }    // since this is impossible without moving...
    inline void MoveCentreTo( const wxPoint2DDouble &pt )
        { m_x += pt.m_x - (m_x+m_width/2); m_y += pt.m_y -(m_y+m_height/2); }
    inline wxOutCode GetOutCode( const wxPoint2DDouble &pt ) const
        { return (wxOutCode) (( ( pt.m_x < m_x ) ? wxOutLeft : 0 ) +
                     ( ( pt.m_x > m_x + m_width ) ? wxOutRight : 0 ) +
                     ( ( pt.m_y < m_y ) ? wxOutTop : 0 )  +
                     ( ( pt.m_y > m_y + m_height ) ? wxOutBottom : 0 )); }
    inline wxOutCode GetOutcode(const wxPoint2DDouble &pt) const
        { return GetOutCode(pt) ; }
    inline bool Contains( const wxPoint2DDouble &pt ) const
        { return  GetOutCode( pt ) == wxInside; }
    inline bool Contains( const wxRect2DDouble &rect ) const
        { return ( ( ( m_x <= rect.m_x ) && ( rect.m_x + rect.m_width <= m_x + m_width ) ) &&
                ( ( m_y <= rect.m_y ) && ( rect.m_y + rect.m_height <= m_y + m_height ) ) ); }
    inline bool IsEmpty() const
        { return m_width <= 0 || m_height <= 0; }
    // FIXME: Double equality
    inline bool HaveEqualSize( const wxRect2DDouble &rect ) const
        { return (rect.m_width == m_width) && (rect.m_height == m_height); }

    inline void Inset( double x , double y )
        { m_x += x; m_y += y; m_width -= 2 * x; m_height -= 2 * y; }
    inline void Inset( double left , double top ,double right , double bottom  )
        { m_x += left; m_y += top; m_width -= left + right; m_height -= top + bottom;}
    inline void Offset( const wxPoint2DDouble &pt )
        { m_x += pt.m_x; m_y += pt.m_y; }

    void ConstrainTo( const wxRect2DDouble &rect );

    wxPoint2DDouble Interpolate( std::int32_t widthfactor, std::int32_t heightfactor ) const
        { return wxPoint2DDouble( m_x + m_width * widthfactor , m_y + m_height * heightfactor ); }

    static void Intersect( const wxRect2DDouble &src1 , const wxRect2DDouble &src2 , wxRect2DDouble *dest );
    inline void Intersect( const wxRect2DDouble &otherRect )
        { Intersect( *this , otherRect , this ); }
    inline wxRect2DDouble CreateIntersection( const wxRect2DDouble &otherRect ) const
        { wxRect2DDouble result; Intersect( *this , otherRect , &result); return result; }
    bool Intersects( const wxRect2DDouble &rect ) const;

    static void Union( const wxRect2DDouble &src1 , const wxRect2DDouble &src2 , wxRect2DDouble *dest );
    void Union( const wxRect2DDouble &otherRect )
        { Union( *this , otherRect , this ); }
    void Union( const wxPoint2DDouble &pt );
    inline wxRect2DDouble CreateUnion( const wxRect2DDouble &otherRect ) const
        { wxRect2DDouble result; Union( *this , otherRect , &result); return result; }

    inline void Scale( double f )
        { m_x *= f; m_y *= f; m_width *= f; m_height *= f;}
    inline void Scale( std::int32_t num , std::int32_t denum )
        { m_x *= ((double)num)/((double)denum); m_y *= ((double)num)/((double)denum);
                m_width *= ((double)num)/((double)denum); m_height *= ((double)num)/((double)denum);}

    // FIXME: Double equality
    inline bool operator == (const wxRect2DDouble& rect) const
        { return (m_x == rect.m_x) && (m_y == rect.m_y) && HaveEqualSize(rect); }
    inline bool operator != (const wxRect2DDouble& rect) const
        { return !(*this == rect); }

    double  m_x;
    double  m_y;
    double  m_width;
    double m_height;
};


// wxRect2Ds are a axis-aligned rectangles, each side of the rect is parallel to the x- or m_y- axis. The rectangle is either defined by the
// top left and bottom right corner, or by the top left corner and size. A point is contained within the rectangle if
// left <= x < right  and top <= m_y < bottom , thus it is a half open interval.

class WXDLLIMPEXP_CORE wxRect2DInt
{
public:
       wxRect2DInt() { m_x = m_y = m_width = m_height = 0; }
       wxRect2DInt( const wxRect& r ) { m_x = r.x ; m_y = r.y ; m_width = r.width ; m_height = r.height ; }
       wxRect2DInt(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h) { m_x = x; m_y = y; m_width = w;  m_height = h; }
       wxRect2DInt(const wxPoint2DInt& topLeft, const wxPoint2DInt& bottomRight);
       inline wxRect2DInt(const wxPoint2DInt& pos, const wxSize& size);
    // default copy ctor and copy-assign operator are OK

        // single attribute accessors

       wxPoint2DInt GetPosition() const { return wxPoint2DInt(m_x, m_y); }
       wxSize GetSize() const { return wxSize(m_width, m_height); }

        // for the edge and corner accessors there are two setters counterparts, the Set.. functions keep the other corners at their
        // position whenever sensible, the Move.. functions keep the size of the rect and move the other corners appropriately

      inline std::int32_t GetLeft() const { return m_x; }
       inline void SetLeft( std::int32_t n ) { m_width += m_x - n; m_x = n; }
       inline void MoveLeftTo( std::int32_t n ) { m_x = n; }
       inline std::int32_t GetTop() const { return m_y; }
       inline void SetTop( std::int32_t n ) { m_height += m_y - n; m_y = n; }
       inline void MoveTopTo( std::int32_t n ) { m_y = n; }
       inline std::int32_t GetBottom() const { return m_y + m_height; }
       inline void SetBottom( std::int32_t n ) { m_height += n - (m_y+m_height);}
       inline void MoveBottomTo( std::int32_t n ) { m_y = n - m_height; }
       inline std::int32_t GetRight() const { return m_x + m_width; }
       inline void SetRight( std::int32_t n ) { m_width += n - (m_x+m_width) ; }
       inline void MoveRightTo( std::int32_t n ) { m_x = n - m_width; }

        inline wxPoint2DInt GetLeftTop() const { return wxPoint2DInt( m_x , m_y ); }
        inline void SetLeftTop( const wxPoint2DInt &pt ) { m_width += m_x - pt.m_x; m_height += m_y - pt.m_y; m_x = pt.m_x; m_y = pt.m_y; }
        inline void MoveLeftTopTo( const wxPoint2DInt &pt ) { m_x = pt.m_x; m_y = pt.m_y; }
        inline wxPoint2DInt GetLeftBottom() const { return wxPoint2DInt( m_x , m_y + m_height ); }
        inline void SetLeftBottom( const wxPoint2DInt &pt ) { m_width += m_x - pt.m_x; m_height += pt.m_y - (m_y+m_height) ; m_x = pt.m_x; }
        inline void MoveLeftBottomTo( const wxPoint2DInt &pt ) { m_x = pt.m_x; m_y = pt.m_y - m_height; }
        inline wxPoint2DInt GetRightTop() const { return wxPoint2DInt( m_x+m_width , m_y ); }
        inline void SetRightTop( const wxPoint2DInt &pt ) { m_width += pt.m_x - ( m_x + m_width ); m_height += m_y - pt.m_y; m_y = pt.m_y; }
        inline void MoveRightTopTo( const wxPoint2DInt &pt ) { m_x = pt.m_x - m_width; m_y = pt.m_y; }
        inline wxPoint2DInt GetRightBottom() const { return wxPoint2DInt( m_x+m_width , m_y + m_height ); }
        inline void SetRightBottom( const wxPoint2DInt &pt ) { m_width += pt.m_x - ( m_x + m_width ); m_height += pt.m_y - (m_y+m_height);}
        inline void MoveRightBottomTo( const wxPoint2DInt &pt ) { m_x = pt.m_x - m_width; m_y = pt.m_y - m_height; }
        inline wxPoint2DInt GetCentre() const { return wxPoint2DInt( m_x+m_width/2 , m_y+m_height/2 ); }
        inline void SetCentre( const wxPoint2DInt &pt ) { MoveCentreTo( pt ); }    // since this is impossible without moving...
        inline void MoveCentreTo( const wxPoint2DInt &pt ) { m_x += pt.m_x - (m_x+m_width/2); m_y += pt.m_y -(m_y+m_height/2); }
        inline wxOutCode GetOutCode( const wxPoint2DInt &pt ) const
            { return (wxOutCode) (( ( pt.m_x < m_x ) ? wxOutLeft : 0 ) +
                     ( ( pt.m_x >= m_x + m_width ) ? wxOutRight : 0 ) +
                     ( ( pt.m_y < m_y ) ? wxOutTop : 0 )  +
                     ( ( pt.m_y >= m_y + m_height ) ? wxOutBottom : 0 )); }
        inline wxOutCode GetOutcode( const wxPoint2DInt &pt ) const
            { return GetOutCode( pt ) ; }
        inline bool Contains( const wxPoint2DInt &pt ) const
            { return  GetOutCode( pt ) == wxInside; }
        inline bool Contains( const wxRect2DInt &rect ) const
            { return ( ( ( m_x <= rect.m_x ) && ( rect.m_x + rect.m_width <= m_x + m_width ) ) &&
                ( ( m_y <= rect.m_y ) && ( rect.m_y + rect.m_height <= m_y + m_height ) ) ); }
        inline bool IsEmpty() const
            { return ( m_width <= 0 || m_height <= 0 ); }
        inline bool HaveEqualSize( const wxRect2DInt &rect ) const
            { return ( rect.m_width == m_width && rect.m_height == m_height ); }

        inline void Inset( std::int32_t x , std::int32_t y ) { m_x += x; m_y += y; m_width -= 2 * x; m_height -= 2 * y; }
        inline void Inset( std::int32_t left , std::int32_t top ,std::int32_t right , std::int32_t bottom  )
            { m_x += left; m_y += top; m_width -= left + right; m_height -= top + bottom;}
        inline void Offset( const wxPoint2DInt &pt ) { m_x += pt.m_x; m_y += pt.m_y; }
        void ConstrainTo( const wxRect2DInt &rect );
        wxPoint2DInt Interpolate( std::int32_t widthfactor, std::int32_t heightfactor ) const
            { return wxPoint2DInt( m_x + m_width * widthfactor, m_y + m_height * heightfactor ); }

        static void Intersect( const wxRect2DInt &src1 , const wxRect2DInt &src2 , wxRect2DInt *dest );
        inline void Intersect( const wxRect2DInt &otherRect ) { Intersect( *this , otherRect , this ); }
        inline wxRect2DInt CreateIntersection( const wxRect2DInt &otherRect ) const { wxRect2DInt result; Intersect( *this , otherRect , &result); return result; }
        bool Intersects( const wxRect2DInt &rect ) const;

        static void Union( const wxRect2DInt &src1 , const wxRect2DInt &src2 , wxRect2DInt *dest );
        void Union( const wxRect2DInt &otherRect )  { Union( *this , otherRect , this ); }
        void Union( const wxPoint2DInt &pt );
        inline wxRect2DInt CreateUnion( const wxRect2DInt &otherRect ) const { wxRect2DInt result; Union( *this , otherRect , &result); return result; }

        inline void Scale( std::int32_t f ) { m_x *= f; m_y *= f; m_width *= f; m_height *= f;}
        inline void Scale( std::int32_t num , std::int32_t denum )
            { m_x *= ((std::int32_t)num)/((std::int32_t)denum); m_y *= ((std::int32_t)num)/((std::int32_t)denum);
                m_width *= ((std::int32_t)num)/((std::int32_t)denum); m_height *= ((std::int32_t)num)/((std::int32_t)denum);}

       bool operator == (const wxRect2DInt& rect) const;
       bool operator != (const wxRect2DInt& rect) const;

#if wxUSE_STREAMS
       void WriteTo( wxDataOutputStream &stream ) const;
       void ReadFrom( wxDataInputStream &stream );
#endif // wxUSE_STREAMS

       std::int32_t m_x;
       std::int32_t m_y;
       std::int32_t m_width;
       std::int32_t m_height;
};

inline wxRect2DInt::wxRect2DInt( const wxPoint2DInt &a , const wxPoint2DInt &b)
{
    m_x = std::min( a.m_x , b.m_x );
    m_y = std::min( a.m_y , b.m_y );
    m_width = std::abs( a.m_x - b.m_x );
    m_height = std::abs( a.m_y - b.m_y );
}

inline wxRect2DInt::wxRect2DInt( const wxPoint2DInt& pos, const wxSize& size)
{
    m_x = pos.m_x;
    m_y = pos.m_y;
    m_width = size.x;
    m_height = size.y;
}

inline bool wxRect2DInt::operator == (const wxRect2DInt& rect) const
{
    return (m_x==rect.m_x && m_y==rect.m_y &&
            m_width==rect.m_width && m_height==rect.m_height);
}

inline bool wxRect2DInt::operator != (const wxRect2DInt& rect) const
{
    return !(*this == rect);
}

class WXDLLIMPEXP_CORE wxTransform2D
{
public :
    virtual ~wxTransform2D() = default;
    virtual void                    Transform( wxPoint2DInt* pt )const  = 0;
    virtual void                    Transform( wxRect2DInt* r ) const;
    virtual wxPoint2DInt    Transform( const wxPoint2DInt &pt ) const;
    virtual wxRect2DInt        Transform( const wxRect2DInt &r ) const ;

    virtual void                    InverseTransform( wxPoint2DInt* pt ) const  = 0;
    virtual void                    InverseTransform( wxRect2DInt* r ) const ;
    virtual wxPoint2DInt    InverseTransform( const wxPoint2DInt &pt ) const ;
    virtual wxRect2DInt        InverseTransform( const wxRect2DInt &r ) const ;
};


#endif // wxUSE_GEOMETRY

#endif // _WX_GEOMETRY_H_
