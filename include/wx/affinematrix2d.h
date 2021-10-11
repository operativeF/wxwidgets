/////////////////////////////////////////////////////////////////////////////
// Name:         wx/affinematrix2d.h
// Purpose:      wxAffineMatrix2D class.
// Author:       Based on wxTransformMatrix by Chris Breeze, Julian Smart
// Created:      2011-04-05
// Copyright:    (c) wxWidgets team
// Licence:      wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_AFFINEMATRIX2D_H_
#define _WX_AFFINEMATRIX2D_H_

#include "wx/defs.h"

#if wxUSE_GEOMETRY

#include "wx/geometry.h"
#include "wx/geometry/point.h"

struct wxMatrix2D
{
    constexpr wxMatrix2D() {};

    constexpr wxMatrix2D(float v11,
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

// A simple implementation of 2-dimensional float affine matrix.

class WXDLLIMPEXP_CORE wxAffineMatrix2D
{
public:
    // Implement base class pure virtual methods.
    constexpr void Set(const wxMatrix2D& mat2D, const wxPoint2DFloat& tr);
    constexpr std::pair<wxMatrix2D, wxPoint2DFloat> Get() const;
    constexpr void Concat(const wxAffineMatrix2D& t);
    constexpr bool Invert();
    constexpr bool IsIdentity() const;
    constexpr void Translate(float dx, float dy);
    constexpr void Scale(float xScale, float yScale);
    void Rotate(float cRadians);

    constexpr wxPoint2DFloat DoTransformPoint(const wxPoint2DFloat& p) const;
    constexpr wxPoint2DFloat DoTransformDistance(const wxPoint2DFloat& p) const;

private:
    float m_11{1.0F};
    float m_12{0.0F};
    float m_21{0.0F};
    float m_22{1.0F};
    float m_tx{0.0F};
    float m_ty{0.0F};
};

#endif // wxUSE_GEOMETRY

#endif // _WX_AFFINEMATRIX2D_H_
