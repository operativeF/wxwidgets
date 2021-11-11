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
