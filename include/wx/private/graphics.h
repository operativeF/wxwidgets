/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/graphics.h
// Purpose:     private graphics context header
// Author:      Stefan Csomor
// Modified by:
// Created:
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GRAPHICS_PRIVATE_H_
#define _WX_GRAPHICS_PRIVATE_H_

#if wxUSE_GRAPHICS_CONTEXT

#include "wx/graphics.h"

class WXDLLIMPEXP_CORE wxGraphicsObjectRefData : public wxObjectRefData
{
    public :
    wxGraphicsObjectRefData( wxGraphicsRenderer* renderer );
    wxGraphicsObjectRefData( const wxGraphicsObjectRefData* data );
    wxGraphicsRenderer* GetRenderer() const ;
    virtual wxGraphicsObjectRefData* Clone() const ;

    protected :
    wxGraphicsRenderer* m_renderer;
} ;

class WXDLLIMPEXP_CORE wxGraphicsBitmapData : public wxGraphicsObjectRefData
{
public :
    wxGraphicsBitmapData( wxGraphicsRenderer* renderer) :
       wxGraphicsObjectRefData(renderer) {}

       ~wxGraphicsBitmapData() override = default;

       // returns the native representation
       virtual void * GetNativeBitmap() const = 0;
} ;

class WXDLLIMPEXP_CORE wxGraphicsMatrixData : public wxGraphicsObjectRefData
{
public :
    wxGraphicsMatrixData( wxGraphicsRenderer* renderer) :
       wxGraphicsObjectRefData(renderer) {}

       ~wxGraphicsMatrixData() override = default;

       // concatenates the matrix
       virtual void Concat( const wxGraphicsMatrixData *t ) = 0;

       // sets the matrix to the respective values
       virtual void Set(double a=1.0, double b=0.0, double c=0.0, double d=1.0,
           double tx=0.0, double ty=0.0) = 0;

       // gets the component values of the matrix
       virtual void Get(double* a=nullptr, double* b=nullptr,  double* c=nullptr,
                        double* d=nullptr, double* tx=nullptr, double* ty=nullptr) const = 0;

       // makes this the inverse matrix
       virtual void Invert() = 0;

       // returns true if the elements of the transformation matrix are equal ?
       virtual bool IsEqual( const wxGraphicsMatrixData* t) const  = 0;

       // return true if this is the identity matrix
       virtual bool IsIdentity() const = 0;

       //
       // transformation
       //

       // add the translation to this matrix
       virtual void Translate( double dx , double dy ) = 0;

       // add the scale to this matrix
       virtual void Scale( double xScale , double yScale ) = 0;

       // add the rotation to this matrix (radians)
       virtual void Rotate( double angle ) = 0;

       //
       // apply the transforms
       //

       // applies that matrix to the point
       virtual void TransformPoint( double *x, double *y ) const = 0;

       // applies the matrix except for translations
       virtual void TransformDistance( double *dx, double *dy ) const =0;

       // returns the native representation
       virtual void * GetNativeMatrix() const = 0;
} ;

class WXDLLIMPEXP_CORE wxGraphicsPathData : public wxGraphicsObjectRefData
{
public :
    wxGraphicsPathData(wxGraphicsRenderer* renderer) : wxGraphicsObjectRefData(renderer) {}
    ~wxGraphicsPathData() override = default;

    //
    // These are the path primitives from which everything else can be constructed
    //

    // begins a new subpath at (x,y)
    virtual void MoveToPoint( double x, double y ) = 0;

    // adds a straight line from the current point to (x,y)
    virtual void AddLineToPoint( double x, double y ) = 0;

    // adds a cubic Bezier curve from the current point, using two control points and an end point
    virtual void AddCurveToPoint( double cx1, double cy1, double cx2, double cy2, double x, double y ) = 0;

    // adds another path
    virtual void AddPath( const wxGraphicsPathData* path ) =0;

    // closes the current sub-path
    virtual void CloseSubpath() = 0;

    // gets the last point of the current path, (0,0) if not yet set
    virtual void GetCurrentPoint( double* x, double* y) const = 0;

    // adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
    virtual void AddArc( double x, double y, double r, double startAngle, double endAngle, bool clockwise ) = 0;

    //
    // These are convenience functions which - if not available natively will be assembled
    // using the primitives from above
    //

    // adds a quadratic Bezier curve from the current point, using a control point and an end point
    virtual void AddQuadCurveToPoint( double cx, double cy, double x, double y );

    // appends a rectangle as a new closed subpath
    virtual void AddRectangle( double x, double y, double w, double h );

    // appends an ellipsis as a new closed subpath fitting the passed rectangle
    virtual void AddCircle( double x, double y, double r );

    // appends a an arc to two tangents connecting (current) to (x1,y1) and (x1,y1) to (x2,y2), also a straight line from (current) to (x1,y1)
    virtual void AddArcToPoint( double x1, double y1 , double x2, double y2, double r ) ;

    // appends an ellipse
    virtual void AddEllipse( double x, double y, double w, double h);

    // appends a rounded rectangle
    virtual void AddRoundedRectangle( double x, double y, double w, double h, double radius);

    // returns the native path
    virtual void * GetNativePath() const = 0;

    // give the native path returned by GetNativePath() back (there might be some deallocations necessary)
    virtual void UnGetNativePath(void *p) const= 0;

    // transforms each point of this path by the matrix
    virtual void Transform( const wxGraphicsMatrixData* matrix ) =0;

    // gets the bounding box enclosing all points (possibly including control points)
    virtual void GetBox(double *x, double *y, double *w, double *h) const=0;

    virtual bool Contains( double x, double y, wxPolygonFillMode fillStyle = wxODDEVEN_RULE) const=0;
};

#endif

#endif // _WX_GRAPHICS_PRIVATE_H_
