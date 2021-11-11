module;

#include "wx/datstrm.h"
#include "wx/utils.h"

export module Utils.Geometry.RectAA;

import Utils.Bitfield;
import Utils.Geometry.Point;
import Utils.Geometry.Size;
import Utils.Geometry.Rect;

import <algorithm>;
import <concepts>;
import <cstdint>;

export
{

// clipping from Cohen-Sutherland

enum class wxOutCode
{
    Inside,
    Left,
    Right,
    Top,
    Bottom,
    _max_size
};

using OutCodeFlags = InclBitfield<wxOutCode>;

// Axis-aligned rectangle template class
template<typename T>
class wxRectAA
{
public:
    using ValueType = T;
    using PointType = wxPoint2D<T>;
    using RectType  = wxRect2D<T>;

    constexpr wxRectAA() {}
    constexpr wxRectAA(const RectType& r)
        : m_x{r.x},
          m_y{r.y},
          m_width{r.width},
          m_height{r.height}
    { 
    }

    constexpr wxRectAA(T x, T y, T w, T h)
        : m_x{x},
          m_y{y},
          m_width{w},
          m_height{h}
    {
    }

    wxRectAA(const PointType& topLeft, const PointType& bottomRight)
    {
        m_x = std::min( topLeft.x, bottomRight.x );
        m_y = std::min( topLeft.y, bottomRight.y );
        m_width = std::abs( topLeft.x - bottomRight.x );
        m_height = std::abs( topLeft.y - bottomRight.y );
    }

    constexpr wxRectAA(const PointType& pos, const wxSize& size)
        : m_x{pos.x},
          m_y{pos.y},
          m_width(size.x),
          m_height(size.y)
    {
    }

    // single attribute accessors
    constexpr PointType GetPosition() const { return {m_x, m_y}; }
    constexpr wxSize GetSize() const { return wxSize(m_width, m_height); }

    // for the edge and corner accessors there are two setters counterparts, the Set.. functions keep the other corners at their
    // position whenever sensible, the Move.. functions keep the size of the rect and move the other corners appropriately

    constexpr ValueType GetLeft() const { return m_x; }
    constexpr void SetLeft( ValueType n ) { m_width += m_x - n; m_x = n; }
    constexpr void MoveLeftTo( ValueType n ) { m_x = n; }
    constexpr ValueType GetTop() const { return m_y; }
    constexpr void SetTop( ValueType n ) { m_height += m_y - n; m_y = n; }
    constexpr void MoveTopTo( ValueType n ) { m_y = n; }
    constexpr ValueType GetBottom() const { return m_y + m_height; }
    constexpr void SetBottom( ValueType n ) { m_height += n - (m_y+m_height);}
    constexpr void MoveBottomTo( ValueType n ) { m_y = n - m_height; }
    constexpr ValueType GetRight() const { return m_x + m_width; }
    constexpr void SetRight( ValueType n ) { m_width += n - (m_x+m_width) ; }
    constexpr void MoveRightTo( ValueType n ) { m_x = n - m_width; }

    constexpr PointType GetLeftTop() const { return PointType( m_x , m_y ); }
    constexpr void SetLeftTop( const PointType &pt ) { m_width += m_x - pt.x; m_height += m_y - pt.y; m_x = pt.x; m_y = pt.y; }
    constexpr void MoveLeftTopTo( const PointType &pt ) { m_x = pt.x; m_y = pt.y; }
    constexpr PointType GetLeftBottom() const { return PointType( m_x , m_y + m_height ); }
    constexpr void SetLeftBottom( const PointType &pt ) { m_width += m_x - pt.x; m_height += pt.y - (m_y+m_height) ; m_x = pt.x; }
    constexpr void MoveLeftBottomTo( const PointType &pt ) { m_x = pt.x; m_y = pt.y - m_height; }
    constexpr PointType GetRightTop() const { return PointType( m_x+m_width , m_y ); }
    constexpr void SetRightTop( const PointType &pt ) { m_width += pt.x - ( m_x + m_width ); m_height += m_y - pt.y; m_y = pt.y; }
    constexpr void MoveRightTopTo( const PointType &pt ) { m_x = pt.x - m_width; m_y = pt.y; }
    constexpr PointType GetRightBottom() const { return PointType( m_x+m_width , m_y + m_height ); }
    constexpr void SetRightBottom( const PointType &pt ) { m_width += pt.x - ( m_x + m_width ); m_height += pt.y - (m_y+m_height);}
    constexpr void MoveRightBottomTo( const PointType &pt ) { m_x = pt.x - m_width; m_y = pt.y - m_height; }
    constexpr PointType GetCentre() const { return PointType( m_x+m_width/2 , m_y+m_height/2 ); }
    constexpr void SetCentre( const PointType &pt ) { MoveCentreTo( pt ); }    // since this is impossible without moving...
    constexpr void MoveCentreTo( const PointType &pt ) { m_x += pt.x - (m_x+m_width/2); m_y += pt.y -(m_y+m_height/2); }
    constexpr OutCodeFlags GetOutCode( const PointType &pt ) const
    {
        OutCodeFlags flags{};
        if(pt.x < m_x)
            flags |= wxOutCode::Left;
        if(pt.x >= m_x + m_width)
            flags |= wxOutCode::Right;
        if(pt.y < m_y)
            flags |= wxOutCode::Top;
        if(pt.y >= m_y + m_height)
            flags |= wxOutCode::Bottom;

        return flags;
    }
    constexpr OutCodeFlags GetOutcode( const PointType &pt ) const { return GetOutCode( pt ) ; }
    constexpr bool Contains( const PointType &pt ) const { return  GetOutCode( pt ) == wxOutCode::Inside; }
    constexpr bool Contains( const wxRectAA& rect ) const
        { return ( ( ( m_x <= rect.m_x ) && ( rect.m_x + rect.m_width <= m_x + m_width ) ) &&
                 ( ( m_y <= rect.m_y ) && ( rect.m_y + rect.m_height <= m_y + m_height ) ) ); }
    constexpr bool IsEmpty() const { return ( m_width <= 0 || m_height <= 0 ); }
    constexpr bool HaveEqualSize( const wxRectAA& rect ) const { return ( rect.m_width == m_width && rect.m_height == m_height ); }
    constexpr void Inset( T x , T y ) { m_x += x; m_y += y; m_width -= 2 * x; m_height -= 2 * y; }
    constexpr void Inset( T left , T top ,T right , T bottom  )
        { m_x += left; m_y += top; m_width -= left + right; m_height -= top + bottom;}
    constexpr void Offset( const PointType &pt ) { m_x += pt.x; m_y += pt.y; }
    constexpr void ConstrainTo( const wxRectAA& rect )
    {
        if ( GetLeft() < rect.GetLeft() )
            SetLeft( rect.GetLeft() );

        if ( GetRight() > rect.GetRight() )
            SetRight( rect.GetRight() );

        if ( GetBottom() > rect.GetBottom() )
            SetBottom( rect.GetBottom() );

        if ( GetTop() < rect.GetTop() )
            SetTop( rect.GetTop() );
    }
    constexpr PointType Interpolate( T widthfactor, T heightfactor ) const { return { m_x + m_width * widthfactor, m_y + m_height * heightfactor }; }

    static constexpr void Intersect( const wxRectAA& src1, const wxRectAA& src2, wxRectAA *dest )
    {
        const auto left = std::max( src1.m_x, src2.m_x );
        const auto right = std::min( src1.m_x + src1.m_width, src2.m_x + src2.m_width );
        const auto top = std::max( src1.m_y, src2.m_y );
        const auto bottom = std::min( src1.m_y + src1.m_height, src2.m_y + src2.m_height );

        if ( left < right && top < bottom )
        {
            dest->m_x = left;
            dest->m_y = top;
            dest->m_width = right - left;
            dest->m_height = bottom - top;
        }
        else
        {
            dest->m_width = dest->m_height = 0;
        }
    }
    constexpr void Intersect( const wxRectAA& otherRect ) { Intersect( *this , otherRect , this ); }
    constexpr wxRectAA CreateIntersection( const wxRectAA& otherRect ) const { wxRectAA result; Intersect( *this , otherRect , &result); return result; }
    constexpr bool Intersects( const wxRectAA& rect ) const
    {
        const auto left = std::max( m_x , rect.m_x );
        const auto right = std::min( m_x + m_width, rect.m_x + rect.m_width );
        const auto top = std::max( m_y , rect.m_y );
        const auto bottom = std::min( m_y + m_height, rect.m_y + rect.m_height );

        return left < right && top < bottom;
    }

    static constexpr void Union( const wxRectAA& src1, const wxRectAA& src2 , wxRectAA *dest )
    {
        const auto left = std::min( src1.m_x , src2.m_x );
        const auto right = std::max( src1.m_x+src1.m_width, src2.m_x + src2.m_width );
        const auto top = std::min( src1.m_y , src2.m_y );
        const auto bottom = std::max( src1.m_y+src1.m_height, src2.m_y + src2.m_height );

        dest->m_x = left;
        dest->m_y = top;
        dest->m_width = right - left;
        dest->m_height = bottom - top;
    }
    constexpr void Union( const wxRectAA& otherRect )  { Union( *this , otherRect , this ); }
    constexpr void Union( const PointType &pt )
    {
        const auto x = pt.x;
        const auto y = pt.y;

        if ( x < m_x )
        {
            SetLeft( x );
        }
        else if ( x < m_x + m_width )
        {
            // contained
        }
        else
        {
            SetRight( x );
        }

        if ( y < m_y )
        {
            SetTop( y );
        }
        else if ( y < m_y + m_height )
        {
            // contained
        }
        else
        {
            SetBottom( y );
        }
    }
    constexpr wxRectAA CreateUnion( const wxRectAA& otherRect ) const { wxRectAA result; Union( *this , otherRect , &result); return result; }

    constexpr void Scale( T f ) { m_x *= f; m_y *= f; m_width *= f; m_height *= f;}
    constexpr void Scale( T num , T denum )
        { m_x *= ((T)num)/((T)denum); m_y *= ((T)num)/((T)denum);
            m_width *= ((T)num)/((T)denum); m_height *= ((T)num)/((T)denum);}

    constexpr bool operator==(const wxRectAA& rect) const
    {
        return m_x == rect.m_x &&
               m_y == rect.m_y &&
               m_width == rect.m_width &&
               m_height == rect.m_height;
    }
    constexpr bool operator!=(const wxRectAA& rect) const
    {
        return !(*this == rect);
    }

#if wxUSE_STREAMS
       void WriteTo( wxDataOutputStream &stream ) const
       {
            stream.Write32( m_x );
            stream.Write32( m_y );
            stream.Write32( m_width );
            stream.Write32( m_height );
       }
       void ReadFrom( wxDataInputStream &stream )
       {
            m_x = stream.Read32();
            m_y = stream.Read32();
            m_width = stream.Read32();
            m_height = stream.Read32();
       }
#endif // wxUSE_STREAMS
    T m_x;
    T m_y;
    T m_width;
    T m_height;
};

using wxRect2DInt    = wxRectAA<int>;
using wxRect2DDouble = wxRectAA<double>;

} // export