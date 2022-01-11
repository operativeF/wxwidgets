///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/coords.cpp
// Purpose:     Coordinates conversion unit tests
// Author:      Artur Wieczorek
// Created:     2020-09-25
// Copyright:   (c) 2020 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/bitmap.h"
#include "wx/dcgraph.h"
#include "wx/dcmemory.h"

import WX.Test.Prec;

import Utils.Geometry;

import <numbers>;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

// =====  Implementation  =====

namespace
{

constexpr wxSize s_dcSize{100, 100};
constexpr wxPoint s_posDev{24, 57};
constexpr wxSize s_dimDev{40, 15};

void InitialState(wxDC* dc)
{
    // Check initial state

    CHECK(dc->GetDeviceOrigin() == wxPoint{0, 0});

    CHECK(dc->GetLogicalOrigin() == wxPoint{0, 0});

    CHECK(dc->GetUserScale() == wxScale(1.0, 1.0));

    CHECK(dc->GetLogicalScale() == wxScale{1.0, 1.0});

#if wxUSE_DC_TRANSFORM_MATRIX
    if ( dc->CanUseTransformMatrix() )
    {
        wxAffineMatrix2D m = dc->GetTransformMatrix();
        CHECK(m.IsIdentity() == true);
    }
#endif // wxUSE_DC_TRANSFORM_MATRIX
}

void NoTransform(wxDC *dc)
{
    // No transformations

    // First convert from device to logical coordinates
    const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                            dc->DeviceToLogicalY(s_posDev.y)};
    CHECK(posLog == s_posDev);

    const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                           dc->DeviceToLogicalYRel(s_dimDev.y)};
    CHECK(dimLog == s_dimDev);

    // And next back from logical to device coordinates
    const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                            dc->LogicalToDeviceY(posLog.y)};
    CHECK(posDev == s_posDev);

   const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                          dc->LogicalToDeviceYRel(dimLog.y)};
    CHECK(dimDev == s_dimDev);
}

void NoTransformEx(wxDC * dc)
{
    // No transformations

    // First convert from device to logical coordinates
    const wxPoint posLog = dc->DeviceToLogical(s_posDev);
    CHECK(posLog == s_posDev);

    const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
    CHECK(dimLog == s_dimDev);

    // And next back from logical to device coordinates
    const wxPoint posDev = dc->LogicalToDevice(posLog);
    CHECK(posDev == s_posDev);

    const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
    CHECK(dimDev == s_dimDev);
}

void DeviceOriginChanged(wxDC* dc)
{
    // Only device origin is changed
    const wxPoint origin{10, 15};
    dc->SetDeviceOrigin(origin);

    // First convert from device to logical coordinates
    const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                            dc->DeviceToLogicalY(s_posDev.y)};
    CHECK(posLog == wxPoint{s_posDev - origin});

    const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                           dc->DeviceToLogicalYRel(s_dimDev.y)};
    CHECK(dimLog == s_dimDev);

    // And next back from logical to device coordinates
    const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                            dc->LogicalToDeviceY(posLog.y)};
    CHECK(posDev == s_posDev);

    const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                           dc->LogicalToDeviceYRel(dimLog.y)};
    CHECK(dimDev == s_dimDev);
}

void DeviceOriginChangedEx(wxDC * dc)
{
    // Only device origin is changed
    const wxPoint origin{10, 15};

    dc->SetDeviceOrigin(origin);

    // First convert from device to logical coordinates
    const wxPoint posLog = dc->DeviceToLogical(s_posDev);
    CHECK(posLog == wxPoint{s_posDev - origin});

    const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
    CHECK(dimLog == s_dimDev);

    // And next back from logical to device coordinates
    const wxPoint posDev = dc->LogicalToDevice(posLog);
    CHECK(posDev == s_posDev);

    const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
    CHECK(dimDev == s_dimDev);
}

void LogicalOriginChanged(wxDC* dc)
{
    // Only logical origin is changed
    const wxPoint origin{-15, -20};
    dc->SetLogicalOrigin(origin);

    // First convert from device to logical coordinates
    const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                            dc->DeviceToLogicalY(s_posDev.y)};
    CHECK(posLog == wxPoint{s_posDev + origin});

    const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                           dc->DeviceToLogicalYRel(s_dimDev.y)};
    CHECK(dimLog == s_dimDev);

    // And next back from logical to device coordinates
    const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                            dc->LogicalToDeviceY(posLog.y)};
    CHECK(posDev == s_posDev);

    const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                           dc->LogicalToDeviceYRel(dimLog.y)};
    CHECK(dimDev == s_dimDev);
}

void LogicalOriginChangedEx(wxDC * dc)
{
    // Only logical origin is changed
    const wxPoint origin{-15, -20};
    dc->SetLogicalOrigin(origin);

    // First convert from device to logical coordinates
    const wxPoint posLog = dc->DeviceToLogical(s_posDev);
    CHECK(posLog == wxPoint{s_posDev + origin});

    const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
    CHECK(dimLog == s_dimDev);

    // And next back from logical to device coordinates
    const wxPoint posDev = dc->LogicalToDevice(posLog);
    CHECK(posDev == s_posDev);

    const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
    CHECK(dimDev == s_dimDev);
}

void UserScaleChanged(wxDC* dc)
{
    // Only user scale is changed
    const wxScale userScale = {2.0, 3.0};
    dc->SetUserScale(userScale);

    // First convert from device to logical coordinates
    const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                            dc->DeviceToLogicalY(s_posDev.y)};
    CHECK(posLog.x == std::lround(s_posDev.x / userScale.x));
    CHECK(posLog.y == std::lround(s_posDev.y / userScale.y));

    const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                           dc->DeviceToLogicalYRel(s_dimDev.y)};
    CHECK(dimLog.x == std::lround(s_dimDev.x / userScale.x));
    CHECK(dimLog.y == std::lround(s_dimDev.y / userScale.y));

    // And next back from logical to device coordinates
    const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                            dc->LogicalToDeviceY(posLog.y)};
    CHECK(posDev == s_posDev);

    const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                           dc->LogicalToDeviceYRel(dimLog.y)};
    CHECK(dimDev == s_dimDev);
}

void UserScaleChangedEx(wxDC * dc)
{
    // Only user scale is changed
    const wxScale userScale{2.0, 3.0};
    dc->SetUserScale(userScale);

    // First convert from device to logical coordinates
    const wxPoint posLog = dc->DeviceToLogical(s_posDev);
    CHECK(posLog.x == std::lround(s_posDev.x / userScale.x));
    CHECK(posLog.y == std::lround(s_posDev.y / userScale.y));

    const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
    CHECK(dimLog.x == std::lround(s_dimDev.x / userScale.x));
    CHECK(dimLog.y == std::lround(s_dimDev.y / userScale.y));

    // And next back from logical to device coordinates
    const wxPoint posDev = dc->LogicalToDevice(posLog);
    CHECK(posDev == s_posDev);

    const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
    CHECK(dimDev == s_dimDev);
}

void LogicalScaleChanged(wxDC* dc)
{
    // Only logical scale is changed
    const wxScale logicScale{2.0, 3.0};
    dc->SetLogicalScale(logicScale);

    // First convert from device to logical coordinates
    const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                            dc->DeviceToLogicalY(s_posDev.y)};
    CHECK(posLog.x == std::lround(s_posDev.x / logicScale.x));
    CHECK(posLog.y == std::lround(s_posDev.y / logicScale.y));

    const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                           dc->DeviceToLogicalYRel(s_dimDev.y)};
    CHECK(dimLog.x == std::lround(s_dimDev.x / logicScale.x));
    CHECK(dimLog.y == std::lround(s_dimDev.y / logicScale.y));

    // And next back from logical to device coordinates
    const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                            dc->LogicalToDeviceY(posLog.y)};
    CHECK(posDev == s_posDev);

    const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                           dc->LogicalToDeviceYRel(dimLog.y)};
    CHECK(dimDev == s_dimDev);
}

void LogicalScaleChangedEx(wxDC * dc)
{
    // Only logical scale is changed
    const wxScale logicScale{2.0, 3.0};
    dc->SetLogicalScale(logicScale);

    // First convert from device to logical coordinates
    const wxPoint posLog = dc->DeviceToLogical(s_posDev);
    CHECK(posLog.x == std::lround(s_posDev.x / logicScale.x));
    CHECK(posLog.y == std::lround(s_posDev.y / logicScale.y));

    const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
    CHECK(dimLog.x == std::lround(s_dimDev.x / logicScale.x));
    CHECK(dimLog.y == std::lround(s_dimDev.y / logicScale.y));

    // And next back from logical to device coordinates
    const wxPoint posDev = dc->LogicalToDevice(posLog);
    CHECK(posDev == s_posDev);

    const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
    CHECK(dimDev == s_dimDev);
}

void TransformedStd(wxDC* dc)
{
    // Apply all standard transformations
    dc->SetDeviceOrigin({ 10, 15 });
    dc->SetUserScale({ 0.5, 2.0 });
    dc->SetLogicalScale({ 4.0, 1.5 });
    dc->SetLogicalOrigin({ -15, -20 });

    // First convert from device to logical coordinates
    const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                            dc->DeviceToLogicalY(s_posDev.y)};
    CHECK(posLog == wxPoint{-8, -6});

    const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                           dc->DeviceToLogicalYRel(s_dimDev.y)};
    CHECK(dimLog == wxSize{20, 5});

    // And next back from logical to device coordinates
    const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                            dc->LogicalToDeviceY(posLog.y)};
    CHECK(posDev == s_posDev);

    const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                           dc->LogicalToDeviceYRel(dimLog.y)};
    CHECK(dimDev == s_dimDev);
}

void TransformedStdEx(wxDC * dc)
{
    // Apply all standardd transformations
    dc->SetDeviceOrigin({ 10, 15 });
    dc->SetUserScale({ 0.5, 2.0 });
    dc->SetLogicalScale({ 4.0, 1.5 });
    dc->SetLogicalOrigin({ -15, -20 });

    // First convert from device to logical coordinates
    const wxPoint posLog = dc->DeviceToLogical(s_posDev);
    CHECK(posLog == wxPoint{-8, -6});

    const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
    CHECK(dimLog == wxSize{20, 5});

    // And next back from logical to device coordinates
    const wxPoint posDev = dc->LogicalToDevice(posLog);
    CHECK(posDev == s_posDev);

    const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
    CHECK(dimDev == s_dimDev);
}

#if wxUSE_DC_TRANSFORM_MATRIX

void TransformedWithMatrix(wxDC* dc)
{
    // Apply transformation matrix only
    if ( dc->CanUseTransformMatrix() )
    {
        // Apply translation and scaling only
        wxAffineMatrix2D m = dc->GetTransformMatrix();
        m.Translate(10, 15);
        m.Scale(2.0, 3.0);
        dc->SetTransformMatrix(m);

        // First convert from device to logical coordinates
        // Results should be nagative because legacy functions
        // don't take affine transformation into account.
        m.Invert();
        wxPoint2DFloat posLogRef = m.TransformPoint(wxPoint2DFloat(s_posDev.x, s_posDev.y));
        const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                                dc->DeviceToLogicalY(s_posDev.y)};

        CHECK_FALSE(posLog.x == std::lround(posLogRef.x));
        CHECK_FALSE(posLog.y == std::lround(posLogRef.y));

        CHECK(posLog == s_posDev);

        wxPoint2DFloat dimLogRef = m.TransformDistance(wxPoint2DFloat(s_dimDev.x, s_dimDev.y));
        const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                               dc->DeviceToLogicalYRel(s_dimDev.y)};
        CHECK_FALSE(dimLog.x == std::lround(dimLogRef.x));
        CHECK_FALSE(dimLog.y == std::lround(dimLogRef.y));
        CHECK(dimLog == s_dimDev);

        // And next back from logical to device coordinates
        const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                                dc->LogicalToDeviceY(posLog.y)};
        CHECK(posDev == s_posDev);

        const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                               dc->LogicalToDeviceYRel(dimLog.y)};
        CHECK(dimDev == s_dimDev);
    }
}

void TransformedWithMatrixEx(wxDC * dc)
{
    // Apply transformation matrix only
    if ( dc->CanUseTransformMatrix() )
    {
        // Apply translation and scaling only
        wxAffineMatrix2D m = dc->GetTransformMatrix();
        m.Translate(10, 15);
        m.Scale(2.0, 3.0);
        dc->SetTransformMatrix(m);

        // First convert from device to logical coordinates
        m.Invert();
        wxPoint2DFloat posLogRef = m.TransformPoint(wxPoint2DFloat(s_posDev.x, s_posDev.y));
        const wxPoint posLog = dc->DeviceToLogical(s_posDev);
        CHECK(posLog.x == std::lround(posLogRef.x));
        CHECK(posLog.y == std::lround(posLogRef.y));

        wxPoint2DFloat dimLogRef = m.TransformDistance(wxPoint2DFloat(s_dimDev.x, s_dimDev.y));
        const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
        CHECK(dimLog.x == std::lround(dimLogRef.x));
        CHECK(dimLog.y == std::lround(dimLogRef.y));

        // And next back from logical to device coordinates
        const wxPoint posDev = dc->LogicalToDevice(posLog);
        CHECK(posDev == s_posDev);

        const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
        CHECK(dimDev == s_dimDev);
     }
}

void TransformedWithMatrixAndStd(wxDC* dc)
{
    // Apply combination of standard and matrix transformations
    if ( dc->CanUseTransformMatrix() )
    {
        dc->SetDeviceOrigin({ 10, 15 });

        dc->SetUserScale({ 0.5, 1.5 });
        dc->SetLogicalScale({ 4.0, 2.0 });
        dc->SetLogicalOrigin({ -15, -20 });

        wxAffineMatrix2D m = dc->GetTransformMatrix();
        m.Translate(10, 18);
        m.Scale(2.0, 0.5);
        dc->SetTransformMatrix(m);

        // First convert from device to logical coordinates
        // Results should be nagative because legacy functions
        // don't take affine transformation into account.
        wxAffineMatrix2D m1;
        m1.Translate(10 - (-15) * (0.5 * 4.0), 15 - (-20) * (1.5 * 2.0));
        m1.Scale(0.5 * 4.0, 1.5 * 2.0);
        m1.Concat(m);
        m1.Invert();

        wxPoint2DFloat posLogRef = m1.TransformPoint(wxPoint2DFloat(s_posDev.x, s_posDev.y));
        const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                                dc->DeviceToLogicalY(s_posDev.y)};
        CHECK_FALSE(posLog.x == std::lround(posLogRef.x));
        CHECK_FALSE(posLog.y == std::lround(posLogRef.y));

        wxPoint2DFloat dimLogRef = m1.TransformDistance(wxPoint2DFloat(s_dimDev.x, s_dimDev.y));
        const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                               dc->DeviceToLogicalYRel(s_dimDev.y)};
        CHECK_FALSE(dimLog.x == std::lround(dimLogRef.x));
        CHECK_FALSE(dimLog.y == std::lround(dimLogRef.y));

        // And next back from logical to device coordinates
        const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                                dc->LogicalToDeviceY(posLog.y)};
        CHECK(posDev == s_posDev);

        const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                               dc->LogicalToDeviceYRel(dimLog.y)};
        CHECK(dimDev == s_dimDev);
    }
}

void TransformedWithMatrixAndStdEx(wxDC * dc)
{
    // Apply combination of standard and matrix transformations
    if ( dc->CanUseTransformMatrix() )
    {
        dc->SetDeviceOrigin({ 10, 15 });

        dc->SetUserScale({ 0.5, 1.5 });
        dc->SetLogicalScale({ 4.0, 2.0 });
        dc->SetLogicalOrigin({ -15, -20 });

        wxAffineMatrix2D m = dc->GetTransformMatrix();
        m.Translate(10, 18);
        m.Scale(2.0, 0.5);
        dc->SetTransformMatrix(m);

        // First convert from device to logical coordinates
        wxAffineMatrix2D m1;
        m1.Translate(10 - (-15) * (0.5 * 4.0), 15 - (-20) * (1.5 * 2.0));
        m1.Scale(0.5 * 4.0, 1.5 * 2.0);
        m1.Concat(m);
        m1.Invert();

        wxPoint2DFloat posLogRef = m1.TransformPoint(wxPoint2DFloat(s_posDev.x, s_posDev.y));
        const wxPoint posLog = dc->DeviceToLogical(s_posDev);
        CHECK(posLog.x == std::lround(posLogRef.x));
        CHECK(posLog.y == std::lround(posLogRef.y));

        wxPoint2DFloat dimLogRef = m1.TransformDistance(wxPoint2DFloat(s_dimDev.x, s_dimDev.y));
        const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
        CHECK(dimLog.x == std::lround(dimLogRef.x));
        CHECK(dimLog.y == std::lround(dimLogRef.y));

        // And next back from logical to device coordinates
        const wxPoint posDev = dc->LogicalToDevice(posLog);
        CHECK(posDev == s_posDev);

        const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
        CHECK(dimDev == s_dimDev);
    }
}

void RotatedWithMatrix(wxDC* dc)
{
    // Apply matrix transformations with rotation component
    if ( dc->CanUseTransformMatrix() )
    {
        wxAffineMatrix2D m = dc->GetTransformMatrix();
        m.Rotate(6.0F * std::numbers::pi_v<float> / 180.0F);
        m.Translate(10.0F, 15.0F);
        m.Scale(2.0F, 3.0F);
        dc->SetTransformMatrix(m);

        // First convert from device to logical coordinates
        // Results should be nagative because legacy functions
        // don't take affine transformation into account.
        m.Invert();
        wxPoint2DFloat posLogRef = m.TransformPoint(wxPoint2DFloat(s_posDev.x, s_posDev.y));
        const wxPoint posLog = {dc->DeviceToLogicalX(s_posDev.x),
                                dc->DeviceToLogicalY(s_posDev.y)};
        CHECK_FALSE(posLog.x == std::lround(posLogRef.x));
        CHECK_FALSE(posLog.y == std::lround(posLogRef.y));
        CHECK(posLog == s_posDev);

        wxPoint2DFloat dimLogRef = m.TransformDistance(wxPoint2DFloat(s_dimDev.x, s_dimDev.y));
        const wxSize dimLog = {dc->DeviceToLogicalXRel(s_dimDev.x),
                               dc->DeviceToLogicalYRel(s_dimDev.y)};
        CHECK_FALSE(dimLog.x == std::lround(dimLogRef.x));
        CHECK_FALSE(dimLog.y == std::lround(dimLogRef.y));
        CHECK(dimLog == s_dimDev);

        // And next back from logical to device coordinates
        const wxPoint posDev = {dc->LogicalToDeviceX(posLog.x),
                                dc->LogicalToDeviceY(posLog.y)};
        CHECK(posDev == s_posDev);

        const wxSize dimDev = {dc->LogicalToDeviceXRel(dimLog.x),
                               dc->LogicalToDeviceYRel(dimLog.y)};
        CHECK(dimDev == s_dimDev);
    }
}

void RotatedWithMatrixEx(wxDC * dc)
{
    // Apply matrix transformations with rotation component
    if ( dc->CanUseTransformMatrix() )
    {
        wxAffineMatrix2D m = dc->GetTransformMatrix();
        m.Rotate(6.0F * std::numbers::pi_v<float> / 180.0F);
        m.Translate(10.0F, 15.0F);
        m.Scale(2.0F, 3.0F);
        dc->SetTransformMatrix(m);

        // First convert from device to logical coordinates
        m.Invert();
        wxPoint2DFloat posLogRef = m.TransformPoint(wxPoint2DFloat(s_posDev.x, s_posDev.y));
        const wxPoint posLog = dc->DeviceToLogical(s_posDev);
        CHECK(posLog.x == std::lround(posLogRef.x));
        CHECK(posLog.y == std::lround(posLogRef.y));

        wxPoint2DFloat dimLogRef = m.TransformDistance(wxPoint2DFloat(s_dimDev.x, s_dimDev.y));
        const wxSize dimLog = dc->DeviceToLogicalRel(s_dimDev);
        CHECK(dimLog.x == std::lround(dimLogRef.x));
        CHECK(dimLog.y == std::lround(dimLogRef.y));

        // And next back from logical to device coordinates
        const wxPoint posDev = dc->LogicalToDevice(posLog);
        CHECK(doctest::Approx(posDev.x).epsilon(1) == s_posDev.x);
        CHECK(doctest::Approx(posDev.y).epsilon(1) == s_posDev.y);

        const wxSize dimDev = dc->LogicalToDeviceRel(dimLog);
        CHECK(doctest::Approx(dimDev.x).epsilon(1) == s_dimDev.x);
        CHECK(doctest::Approx(dimDev.y).epsilon(1) == s_dimDev.y);
    }
}

#endif // wxUSE_DC_TRANSFORM_MATRIX

} // namespace anonymous

// ====================
// wxDC / wxGCDC tests
// ====================

class CoordinatesDCTestCaseBase
{
public:
    CoordinatesDCTestCaseBase()
    {
        m_bmp.Create(s_dcSize);
    }

    virtual ~CoordinatesDCTestCaseBase()
    {
        m_bmp = wxNullBitmap;
    }

protected:
    wxBitmap m_bmp;
    wxDC* m_dc{ nullptr };
};

// ===========
// wxDC tests
// ===========

class CoordinatesDCTestCase : public CoordinatesDCTestCaseBase
{
public:
    CoordinatesDCTestCase()
    {
        m_mdc.SelectObject(m_bmp);
        m_dc = &m_mdc;
    }

    virtual ~CoordinatesDCTestCase()
    {
        m_mdc.SelectObject(wxNullBitmap);
    }

protected:
    wxMemoryDC m_mdc;
};

#if wxUSE_GRAPHICS_CONTEXT
// =============
// wxGCDC tests
// =============

class CoordinatesGCDCTestCase : public CoordinatesDCTestCase
{
public:
    CoordinatesGCDCTestCase()
    {
        m_gcdc = std::make_unique<wxGCDC>(m_mdc);

        wxGraphicsContext* ctx = m_gcdc->GetGraphicsContext();
        ctx->SetAntialiasMode(wxAntialiasMode::None);
        ctx->DisableOffset();

        m_dc = m_gcdc.get();
    }

protected:
    std::unique_ptr<wxGCDC> m_gcdc;
};
#endif //  wxUSE_GRAPHICS_CONTEXT

// For GTK+ 3 and OSX wxDC is equivalent to wxGCDC
// so it doesn't need to be tested individually.
#if !defined(__WXGTK3__) && !defined(__WXOSX__)
TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::InitialState")
{
    // Check initial state
    InitialState(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::NoTransform")
{
    // No transformations
    NoTransform(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::NoTransformEx")
{
    // No transformations
    NoTransformEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::DeviceOriginChanged")
{
    // Only device origin is changed
    DeviceOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::DeviceOriginChangedEx")
{
    // Only device origin is changed
    DeviceOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::LogicalOriginChanged")
{
    // Only logical origin is changed
    LogicalOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::LogicalOriginChangedEx")
{
    // Only logical origin is changed
    LogicalOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::UserScaleChanged")
{
    // Only user scale is changed
    UserScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::UserScaleChangedEx")
{
    // Only user scale is changed
    UserScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::LogicalScaleChanged")
{
    // Only logical scale is changed
    LogicalScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::LogicalScaleChangedEx")
{
    // Only logical scale is changed
    LogicalScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::TransformedStd")
{
    // Apply all standardd transformations
    TransformedStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::TransformedStdEx")
{
    // Apply all standardd transformations
    TransformedStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::TransformedWithMatrix")
{
    // Apply transformation matrix only
    TransformedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::TransformedWithMatrixEx")
{
    // Apply transformation matrix only
    TransformedWithMatrixEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::TransformedWithMatrixAndStd")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::TransformedWithMatrixAndStdEx")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::RotatedWithMatrix")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesDCTestCase, "CoordinatesDC::RotatedWithMatrixEx")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrixEx(m_dc);
}
#endif // !__WXGTK3__ && !_WXOSX__

#if wxUSE_GRAPHICS_CONTEXT
// For MSW we have individual test cases for each graphics renderer
// so we don't need to test wxGCDC with default renderer.
#ifndef __WXMSW__
TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::InitialState")
{
    // Check initial state
    InitialState(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::NoTransform")
{
    // No transformations
    NoTransform(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::NoTransformEx")
{
    // No transformations
    NoTransformEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::DeviceOriginChanged")
{
    // Only device origin is changed
    DeviceOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::DeviceOriginChangedEx")
{
    // Only device origin is changed
    DeviceOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::LogicalOriginChanged")
{
    // Only logical origin is changed
    LogicalOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::LogicalOriginChangedEx")
{
    // Only logical origin is changed
    LogicalOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::UserScaleChanged")
{
    // Only user scale is changed
    UserScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::UserScaleChangedEx")
{
    // Only user scale is changed
    UserScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::LogicalScaleChanged")
{
    // Only logical scale is changed
    LogicalScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::LogicalScaleChangedEx")
{
    // Only logical scale is changed
    LogicalScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::TransformedStd")
{
    // Apply all standardd transformations
    TransformedStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::TransformedStdEx")
{
    // Apply all standardd transformations
    TransformedStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::TransformedWithMatrix")
{
    // Apply transformation matrix only
    TransformedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::TransformedWithMatrixEx")
{
    // Apply transformation matrix only
    TransformedWithMatrixEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::TransformedWithMatrixAndStd")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::TransformedWithMatrixAndStdEx")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::RotatedWithMatrix")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCTestCase, "CoordinatesGCDC::RotatedWithMatrixEx")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrixEx(m_dc);
}
#else // Direct2D is available only under MSW.

class CoordinatesGCDCDirect2DTestCase : public CoordinatesGCDCTestCase
{
public:
    CoordinatesGCDCDirect2DTestCase()
    {
        wxGraphicsRenderer* rend = wxGraphicsRenderer::GetDirect2DRenderer();
        std::unique_ptr<wxGraphicsContext> ctx = rend->CreateContext(m_mdc);
        REQUIRE(ctx != nullptr);
        m_gcdc->SetGraphicsContext(std::move(ctx));
    }
};

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::InitialState")
{
    // Check initial state
    InitialState(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::NoTransform")
{
    // No transformations
    NoTransform(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::NoTransformEx")
{
    // No transformations
    NoTransformEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::DeviceOriginChanged")
{
    // Only device origin is changed
    DeviceOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::DeviceOriginChangedEx")
{
    // Only device origin is changed
    DeviceOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::LogicalOriginChanged")
{
    // Only logical origin is changed
    LogicalOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::LogicalOriginChangedEx")
{
    // Only logical origin is changed
    LogicalOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::UserScaleChanged")
{
    // Only user scale is changed
    UserScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::UserScaleChangedEx")
{
    // Only user scale is changed
    UserScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::LogicalScaleChanged")
{
    // Only logical scale is changed
    LogicalScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::LogicalScaleChangedEx")
{
    // Only logical scale is changed
    LogicalScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::TransformedStd")
{
    // Apply all standardd transformations
    TransformedStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::TransformedStdEx")
{
    // Apply all standardd transformations
    TransformedStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::TransformedWithMatrix")
{
    // Apply transformation matrix only
    TransformedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::TransformedWithMatrixEx")
{
    // Apply transformation matrix only
    TransformedWithMatrixEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::TransformedWithMatrixAndStd")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::TransformedWithMatrixAndStdEx")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::RotatedWithMatrix")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCDirect2DTestCase, "CoordinatesGCDCDirect2D::RotatedWithMatrixEx")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrixEx(m_dc);
}

#endif // __WXMSW__/!__WXMSW__

#if wxUSE_CAIRO
class CoordinatesGCDCCairoTestCase : public CoordinatesGCDCTestCase
{
public:
    CoordinatesGCDCCairoTestCase()
    {
        wxGraphicsRenderer* rend = wxGraphicsRenderer::GetCairoRenderer();
        wxGraphicsContext* ctx = rend->CreateContext(m_mdc);
        REQUIRE(ctx != nullptr);
        m_gcdc->SetGraphicsContext(ctx);
    }
};

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::InitialState")
{
    // Check initial state
    InitialState(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::NoTransform")
{
    // No transformations
    NoTransform(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::NoTransformEx")
{
    // No transformations
    NoTransformEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::DeviceOriginChanged")
{
    // Only device origin is changed
    DeviceOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::DeviceOriginChangedEx")
{
    // Only device origin is changed
    DeviceOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::LogicalOriginChanged")
{
    // Only logical origin is changed
    LogicalOriginChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::LogicalOriginChangedEx")
{
    // Only logical origin is changed
    LogicalOriginChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::UserScaleChanged")
{
    // Only user scale is changed
    UserScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::UserScaleChangedEx")
{
    // Only user scale is changed
    UserScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::LogicalScaleChanged")
{
    // Only logical scale is changed
    LogicalScaleChanged(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::LogicalScaleChangedEx")
{
    // Only logical scale is changed
    LogicalScaleChangedEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::TransformedStd")
{
    // Apply all standardd transformations
    TransformedStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::TransformedStdEx")
{
    // Apply all standardd transformations
    TransformedStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::TransformedWithMatrix")
{
    // Apply transformation matrix only
    TransformedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::TransformedWithMatrixEx")
{
    // Apply transformation matrix only
    TransformedWithMatrixEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::TransformedWithMatrixAndStd")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStd(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::TransformedWithMatrixAndStdEx")
{
    // Apply combination of standard and matrix transformations
    TransformedWithMatrixAndStdEx(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::RotatedWithMatrix")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrix(m_dc);
}

TEST_CASE_FIXTURE(CoordinatesGCDCCairoTestCase, "CoordinatesGCDCCairo::RotatedWithMatrixEx")
{
    // Apply matrix transformations with rotation component
    RotatedWithMatrixEx(m_dc);
}
#endif // wxUSE_CAIRO
#endif // wxUSE_GRAPHICS_CONTEXT
