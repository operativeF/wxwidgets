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

#include "wx/utils.h"

import Utils.Bitfield;
import Utils.Geometry.Point;
import Utils.Geometry.Size;
import Utils.Geometry.RectAA;

class wxTransform2D
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

#endif // _WX_GEOMETRY_H_
