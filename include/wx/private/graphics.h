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

import WX.GDI.Flags;

class wxGraphicsObjectRefData : public wxObjectRefData
{
public :
    wxGraphicsObjectRefData( wxGraphicsRenderer* renderer );
    wxGraphicsObjectRefData( const wxGraphicsObjectRefData* data );
    wxGraphicsRenderer* GetRenderer() const ;
    virtual wxGraphicsObjectRefData* Clone() const ;

protected :
    wxGraphicsRenderer* m_renderer;
} ;

class wxGraphicsBitmapData : public wxGraphicsObjectRefData
{
public :
    wxGraphicsBitmapData( wxGraphicsRenderer* renderer) :
       wxGraphicsObjectRefData(renderer) {}

       // returns the native representation
       virtual void * GetNativeBitmap() const = 0;
} ;

class wxGraphicsMatrixData : public wxGraphicsObjectRefData
{
public:
    wxGraphicsMatrixData( wxGraphicsRenderer* renderer) :
       wxGraphicsObjectRefData(renderer) {}

       // concatenates the matrix
       virtual void Concat( const wxGraphicsMatrixData *t ) = 0;

       // sets the matrix to the respective values
       virtual void Set(float a, float b,
                        float c, float d,
                        float tx, float ty) = 0;

       // gets the component values of the matrix
       virtual void Get(float* a, float* b,  float* c,
                        float* d, float* tx, float* ty) const = 0;

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
       virtual void Translate( float dx , float dy ) = 0;

       // add the scale to this matrix
       virtual void Scale( float xScale , float yScale ) = 0;

       // add the rotation to this matrix (radians)
       virtual void Rotate( float angle ) = 0;

       //
       // apply the transforms
       //

       // applies that matrix to the point
       virtual void TransformPoint( float *x, float *y ) const = 0;

       // applies the matrix except for translations
       virtual void TransformDistance( float *dx, float *dy ) const = 0;

       // returns the native representation
       virtual void* GetNativeMatrix() const = 0;
} ;

class wxGraphicsPathData : public wxGraphicsObjectRefData
{
public :
    wxGraphicsPathData(wxGraphicsRenderer* renderer) : wxGraphicsObjectRefData(renderer) {}

    //
    // These are the path primitives from which everything else can be constructed
    //

    // begins a new subpath at (x,y)
    virtual void MoveToPoint( float x, float y ) = 0;

    // adds a straight line from the current point to (x,y)
    virtual void AddLineToPoint( float x, float y ) = 0;

    // adds a cubic Bezier curve from the current point, using two control points and an end point
    virtual void AddCurveToPoint( float cx1, float cy1, float cx2, float cy2, float x, float y ) = 0;

    // adds another path
    virtual void AddPath( const wxGraphicsPathData* path ) =0;

    // closes the current sub-path
    virtual void CloseSubpath() = 0;

    // gets the last point of the current path, (0,0) if not yet set
    virtual void GetCurrentPoint( float* x, float* y) const = 0;

    // adds an arc of a circle centering at (x,y) with radius (r) from startAngle to endAngle
    virtual void AddArc( float x, float y, float r, float startAngle, float endAngle, bool clockwise ) = 0;

    //
    // These are convenience functions which - if not available natively will be assembled
    // using the primitives from above
    //

    // adds a quadratic Bezier curve from the current point, using a control point and an end point
    virtual void AddQuadCurveToPoint( float cx, float cy, float x, float y );

    // appends a rectangle as a new closed subpath
    virtual void AddRectangle( float x, float y, float w, float h );

    // appends an ellipsis as a new closed subpath fitting the passed rectangle
    virtual void AddCircle( float x, float y, float r );

    // appends a an arc to two tangents connecting (current) to (x1,y1) and (x1,y1) to (x2,y2), also a straight line from (current) to (x1,y1)
    virtual void AddArcToPoint( float x1, float y1 , float x2, float y2, float r ) ;

    // appends an ellipse
    virtual void AddEllipse( float x, float y, float w, float h);

    // appends a rounded rectangle
    virtual void AddRoundedRectangle( float x, float y, float w, float h, float radius);

    // returns the native path
    virtual void * GetNativePath() const = 0;

    // give the native path returned by GetNativePath() back (there might be some deallocations necessary)
    virtual void UnGetNativePath(void *p) const= 0;

    // transforms each point of this path by the matrix
    virtual void Transform( const wxGraphicsMatrixData* matrix ) =0;

    // gets the bounding box enclosing all points (possibly including control points)
    virtual void GetBox(float *x, float *y, float *w, float *h) const=0;

    virtual bool Contains( float x, float y, wxPolygonFillMode fillStyle = wxPolygonFillMode::OddEven) const=0;
};

#endif

#endif // _WX_GRAPHICS_PRIVATE_H_
