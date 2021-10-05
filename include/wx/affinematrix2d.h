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

#include "wx/affinematrix2dbase.h"

// A simple implementation of wxAffineMatrix2DBase interface done entirely in
// wxWidgets.
class WXDLLIMPEXP_CORE wxAffineMatrix2D : public wxAffineMatrix2DBase
{
public:
    // Implement base class pure virtual methods.
    void Set(const wxMatrix2D& mat2D, const wxPoint2DFloat& tr) override;
    void Get(wxMatrix2D* mat2D, wxPoint2DFloat* tr) const override;
    void Concat(const wxAffineMatrix2DBase& t) override;
    bool Invert() override;
    bool IsIdentity() const override;
    bool IsEqual(const wxAffineMatrix2DBase& t) const override;
    void Translate(float dx, float dy) override;
    void Scale(float xScale, float yScale) override;
    void Rotate(float cRadians) override;

protected:
    wxPoint2DFloat DoTransformPoint(const wxPoint2DFloat& p) const override;
    wxPoint2DFloat DoTransformDistance(const wxPoint2DFloat& p) const override;

private:
    float m_11{1.0F}, m_12{0.0F}, m_21{0.0F}, m_22{1.0F}, m_tx{0.0F}, m_ty{0.0F};
};

#endif // wxUSE_GEOMETRY

#endif // _WX_AFFINEMATRIX2D_H_
