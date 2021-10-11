///////////////////////////////////////////////////////////////////////////////
// Name:        affinematrix2d.cpp
// Purpose:     implementation of wxAffineMatrix2D
// Author:      Based on wxTransformMatrix by Chris Breeze, Julian Smart
// Created:     2011-04-05
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"


#if wxUSE_GEOMETRY

#include "wx/affinematrix2d.h"

// sets the matrix to the respective values
constexpr void wxAffineMatrix2D::Set(const wxMatrix2D &mat2D, const wxPoint2DFloat &tr)
{
    m_11 = mat2D.m_11;
    m_12 = mat2D.m_12;
    m_21 = mat2D.m_21;
    m_22 = mat2D.m_22;
    m_tx = tr.x;
    m_ty = tr.y;
}

// gets the component valuess of the matrix
constexpr std::pair<wxMatrix2D, wxPoint2DFloat> wxAffineMatrix2D::Get() const
{
    return {wxMatrix2D{m_11, m_12, m_21, m_22}, wxPoint2DFloat{m_tx, m_ty}};
}

// concatenates the matrix
// | t.m_11  t.m_12  0 |   | m_11  m_12   0 |
// | t.m_21  t.m_22  0 | x | m_21  m_22   0 |
// | t.m_tx  t.m_ty  1 |   | m_tx  m_ty   1 |
constexpr void wxAffineMatrix2D::Concat(const wxAffineMatrix2D &t)
{
    auto [mat, tr] = t.Get();

    m_tx += tr.x*m_11 + tr.y*m_21;
    m_ty += tr.x*m_12 + tr.y*m_22;

    float e11 = mat.m_11*m_11 + mat.m_12*m_21;
    float e12 = mat.m_11*m_12 + mat.m_12*m_22;
    float e21 = mat.m_21*m_11 + mat.m_22*m_21;

    m_22 = mat.m_21*m_12 + mat.m_22*m_22;
    m_11 = e11;
    m_12 = e12;
    m_21 = e21;
}

// makes this its inverse matrix.
// Invert
// | m_11  m_12   0 |
// | m_21  m_22   0 |
// | m_tx  m_ty   1 |
constexpr bool wxAffineMatrix2D::Invert()
{
    const float det = m_11*m_22 - m_12*m_21;

    if ( !det )
        return false;

    float ex = (m_21*m_ty - m_22*m_tx) / det;
    m_ty = (-m_11*m_ty + m_12*m_tx) / det;
    m_tx = ex;
    float e11 = m_22 / det;
    m_12 = -m_12 / det;
    m_21 = -m_21 / det;
    m_22 = m_11 / det;
    m_11 = e11;

    return true;
}

//
// transformations
//

// add the translation to this matrix
// |  1   0   0 |   | m_11  m_12   0 |
// |  0   1   0 | x | m_21  m_22   0 |
// | dx  dy   1 |   | m_tx  m_ty   1 |
constexpr void wxAffineMatrix2D::Translate(float dx, float dy)
{
    m_tx += m_11 * dx + m_21 * dy;
    m_ty += m_12 * dx + m_22 * dy;
}

// add the scale to this matrix
// | xScale   0      0 |   | m_11  m_12   0 |
// |   0    yScale   0 | x | m_21  m_22   0 |
// |   0      0      1 |   | m_tx  m_ty   1 |
constexpr void wxAffineMatrix2D::Scale(float xScale, float yScale)
{
    m_11 *= xScale;
    m_12 *= xScale;
    m_21 *= yScale;
    m_22 *= yScale;
}

// add the rotation to this matrix (clockwise, radians)
// | cos    sin   0 |   | m_11  m_12   0 |
// | -sin   cos   0 | x | m_21  m_22   0 |
// |  0      0    1 |   | m_tx  m_ty   1 |
void wxAffineMatrix2D::Rotate(float cRadians)
{
    const float c = std::cos(cRadians);
    const float s = std::sin(cRadians);

    const float e11 = c * m_11 + s * m_21;
    const float e12 = c * m_12 + s * m_22;

    m_21 = c * m_21 - s * m_11;
    m_22 = c * m_22 - s * m_12;
    m_11 = e11;
    m_12 = e12;
}

//
// apply the transforms
//

// applies that matrix to the point
//                           | m_11  m_12   0 |
// | src.m_x  src._my  1 | x | m_21  m_22   0 |
//                           | m_tx  m_ty   1 |
constexpr wxPoint2DFloat wxAffineMatrix2D::DoTransformPoint(const wxPoint2DFloat& src) const
{
    if ( IsIdentity() )
        return src;

    return {src.x * m_11 + src.y * m_21 + m_tx,
            src.x * m_12 + src.y * m_22 + m_ty};
}

// applies the matrix except for translations
//                           | m_11  m_12   0 |
// | src.m_x  src._my  0 | x | m_21  m_22   0 |
//                           | m_tx  m_ty   1 |
constexpr wxPoint2DFloat wxAffineMatrix2D::DoTransformDistance(const wxPoint2DFloat& src) const
{
    if ( IsIdentity() )
        return src;

    return {src.x * m_11 + src.y * m_21,
            src.x * m_12 + src.y * m_22};
}

constexpr bool wxAffineMatrix2D::IsIdentity() const
{
    return m_11 == 1 && m_12 == 0 &&
           m_21 == 0 && m_22 == 1 &&
           m_tx == 0 && m_ty == 0;
}

#endif // wxUSE_GEOMETRY
