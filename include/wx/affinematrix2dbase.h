/////////////////////////////////////////////////////////////////////////////
// Name:         wx/affinematrix2dbase.h
// Purpose:      Common interface for 2D transformation matrices.
// Author:       Catalin Raceanu
// Created:      2011-04-06
// Copyright:    (c) wxWidgets team
// Licence:      wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_AFFINEMATRIX2DBASE_H_
#define _WX_AFFINEMATRIX2DBASE_H_

#include "wx/defs.h"

#if wxUSE_GEOMETRY

#include "wx/geometry.h"

struct wxMatrix2D
{
    wxMatrix2D() = default;

    wxMatrix2D(float v11,
               float v12,
               float v21,
               float v22)
        : m_11{v11},
          m_12{v12},
          m_21{v21},
          m_22{v22}
    {
    }

    float m_11{};
    float m_12{};
    float m_21{};
    float m_22{};
};

// A 2x3 matrix representing an affine 2D transformation.
//
// This is an abstract base class implemented by wxAffineMatrix2D only so far,
// but in the future we also plan to derive wxGraphicsMatrix from it (it should
// also be documented then as currently only wxAffineMatrix2D itself is).
class WXDLLIMPEXP_CORE wxAffineMatrix2DBase
{
public:
    wxAffineMatrix2DBase() = default;
    virtual ~wxAffineMatrix2DBase() = default;

    // sets the matrix to the respective values
    virtual void Set(const wxMatrix2D& mat2D, const wxPoint2DFloat& tr) = 0;

    // gets the component values of the matrix
    virtual void Get(wxMatrix2D* mat2D, wxPoint2DFloat* tr) const = 0;

    // concatenates the matrix
    virtual void Concat(const wxAffineMatrix2DBase& t) = 0;

    // makes this the inverse matrix
    virtual bool Invert() = 0;

    // return true if this is the identity matrix
    virtual bool IsIdentity() const = 0;

    // returns true if the elements of the transformation matrix are equal ?
    virtual bool IsEqual(const wxAffineMatrix2DBase& t) const = 0;
    bool operator==(const wxAffineMatrix2DBase& t) const { return IsEqual(t); }
    bool operator!=(const wxAffineMatrix2DBase& t) const { return !IsEqual(t); }


    //
    // transformations
    //

    // add the translation to this matrix
    virtual void Translate(float dx, float dy) = 0;

    // add the scale to this matrix
    virtual void Scale(float xScale, float yScale) = 0;

    // add the rotation to this matrix (counter clockwise, radians)
    virtual void Rotate(float ccRadians) = 0;

    // add mirroring to this matrix
    void Mirror(int direction = wxHORIZONTAL)
    {
        float x = (direction & wxHORIZONTAL) ? -1 : 1;
        float y = (direction & wxVERTICAL) ? -1 : 1;
        Scale(x, y);
    }


    // applies that matrix to the point
    wxPoint2DFloat TransformPoint(const wxPoint2DFloat& src) const
    {
        return DoTransformPoint(src);
    }

    void TransformPoint(float* x, float* y) const
    {
        wxCHECK_RET( x && y, "Can't be NULL" );

        const wxPoint2DFloat dst = DoTransformPoint(wxPoint2DFloat(*x, *y));
        *x = dst.x;
        *y = dst.y;
    }

    // applies the matrix except for translations
    wxPoint2DFloat TransformDistance(const wxPoint2DFloat& src) const
    {
        return DoTransformDistance(src);
    }

    void TransformDistance(float* dx, float* dy) const
    {
        wxCHECK_RET( dx && dy, "Can't be NULL" );

        const wxPoint2DFloat
            dst = DoTransformDistance(wxPoint2DFloat(*dx, *dy));
        *dx = dst.x;
        *dy = dst.y;
    }

protected:
    virtual
        wxPoint2DFloat DoTransformPoint(const wxPoint2DFloat& p) const = 0;
    virtual
        wxPoint2DFloat DoTransformDistance(const wxPoint2DFloat& p) const = 0;
};

#endif // wxUSE_GEOMETRY

#endif // _WX_AFFINEMATRIX2DBASE_H_
