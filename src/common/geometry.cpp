/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/geometry.cpp
// Purpose:     Common Geometry Classes
// Author:      Stefan Csomor
// Modified by:
// Created:     08/05/99
// Copyright:   (c) 1999 Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_GEOMETRY

#include "wx/geometry.h"
#include "wx/log.h"
#include "wx/datstrm.h"

//
// wxPoint2D
//

//
// wxRect2D
//

// double version

// for the following calculations always remember
// that the right and bottom edges are not part of a rect

bool wxRect2DDouble::Intersects( const wxRect2DDouble &rect ) const
{
    const double left = std::max( m_x , rect.m_x );
    const double right = std::min( m_x+m_width, rect.m_x + rect.m_width );
    const double top = std::max( m_y , rect.m_y );
    const double bottom = std::min( m_y+m_height, rect.m_y + rect.m_height );

    return left < right && top < bottom;
}

void wxRect2DDouble::Intersect( const wxRect2DDouble &src1 , const wxRect2DDouble &src2 , wxRect2DDouble *dest )
{
    const double left = std::max( src1.m_x , src2.m_x );
    const double right = std::min( src1.m_x+src1.m_width, src2.m_x + src2.m_width );
    const double top = std::max( src1.m_y , src2.m_y );
    const double bottom = std::min( src1.m_y+src1.m_height, src2.m_y + src2.m_height );

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

void wxRect2DDouble::Union( const wxRect2DDouble &src1 , const wxRect2DDouble &src2 , wxRect2DDouble *dest )
{
    const double left = std::min( src1.m_x , src2.m_x );
    const double right = std::max( src1.m_x+src1.m_width, src2.m_x + src2.m_width );
    const double top = std::min( src1.m_y , src2.m_y );
    const double bottom = std::max( src1.m_y+src1.m_height, src2.m_y + src2.m_height );

    dest->m_x = left;
    dest->m_y = top;
    dest->m_width = right - left;
    dest->m_height = bottom - top;
}

void wxRect2DDouble::Union( const wxPoint2DDouble &pt )
{
    const double x = pt.x;
    const double y = pt.y;

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

void wxRect2DDouble::ConstrainTo( const wxRect2DDouble &rect )
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

// integer version

// for the following calculations always remember
// that the right and bottom edges are not part of a rect

// wxRect2D

bool wxRect2DInt::Intersects( const wxRect2DInt &rect ) const
{
    const std::int32_t left = std::max( m_x , rect.m_x );
    const std::int32_t right = std::min( m_x+m_width, rect.m_x + rect.m_width );
    const std::int32_t top = std::max( m_y , rect.m_y );
    const std::int32_t bottom = std::min( m_y+m_height, rect.m_y + rect.m_height );

    return left < right && top < bottom;
}

void wxRect2DInt::Intersect( const wxRect2DInt &src1 , const wxRect2DInt &src2 , wxRect2DInt *dest )
{
    const std::int32_t left = std::max( src1.m_x , src2.m_x );
    const std::int32_t right = std::min( src1.m_x+src1.m_width, src2.m_x + src2.m_width );
    const std::int32_t top = std::max( src1.m_y , src2.m_y );
    const std::int32_t bottom = std::min( src1.m_y+src1.m_height, src2.m_y + src2.m_height );

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

void wxRect2DInt::Union( const wxRect2DInt &src1 , const wxRect2DInt &src2 , wxRect2DInt *dest )
{
    const std::int32_t left = std::min( src1.m_x , src2.m_x );
    const std::int32_t right = std::max( src1.m_x+src1.m_width, src2.m_x + src2.m_width );
    const std::int32_t top = std::min( src1.m_y , src2.m_y );
    const std::int32_t bottom = std::max( src1.m_y+src1.m_height, src2.m_y + src2.m_height );

    dest->m_x = left;
    dest->m_y = top;
    dest->m_width = right - left;
    dest->m_height = bottom - top;
}

void wxRect2DInt::Union( const wxPoint2DInt &pt )
{
    const std::int32_t x = pt.x;
    const std::int32_t y = pt.y;

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

void wxRect2DInt::ConstrainTo( const wxRect2DInt &rect )
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

#if wxUSE_STREAMS
void wxRect2DInt::WriteTo( wxDataOutputStream &stream ) const
{
    stream.Write32( m_x );
    stream.Write32( m_y );
    stream.Write32( m_width );
    stream.Write32( m_height );
}

void wxRect2DInt::ReadFrom( wxDataInputStream &stream )
{
    m_x = stream.Read32();
    m_y = stream.Read32();
    m_width = stream.Read32();
    m_height = stream.Read32();
}
#endif // wxUSE_STREAMS


// wxTransform2D

void wxTransform2D::Transform( wxRect2DInt* r ) const
{
    wxPoint2DInt a = r->GetLeftTop(), b = r->GetRightBottom();
    Transform( &a );
    Transform( &b );
    *r = wxRect2DInt( a, b );
}

wxPoint2DInt wxTransform2D::Transform( const wxPoint2DInt &pt ) const
{
    wxPoint2DInt res = pt;
    Transform( &res );
    return res;
}

wxRect2DInt wxTransform2D::Transform( const wxRect2DInt &r ) const
{
    wxRect2DInt res = r;
    Transform( &res );
    return res;
}

void wxTransform2D::InverseTransform( wxRect2DInt* r ) const
{
    wxPoint2DInt a = r->GetLeftTop(), b = r->GetRightBottom();
    InverseTransform( &a );
    InverseTransform( &b );
    *r = wxRect2DInt( a , b );
}

wxPoint2DInt wxTransform2D::InverseTransform( const wxPoint2DInt &pt ) const
{
    wxPoint2DInt res = pt;
    InverseTransform( &res );
    return res;
}

wxRect2DInt wxTransform2D::InverseTransform( const wxRect2DInt &r ) const
{
    wxRect2DInt res = r;
    InverseTransform( &res );
    return res;
}

#endif // wxUSE_GEOMETRY
