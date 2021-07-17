///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/grappath.cpp
// Purpose:     graphics path unit tests
// Author:      Artur Wieczorek
// Created:     2018-07-01
// Copyright:   (c) 2018 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#if wxUSE_GRAPHICS_CONTEXT

#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "wx/dcgraph.h"


static void DoAllTests(wxGraphicsContext* gc);

// For MSW we have individual test cases for each graphics renderer
// so we don't need to execute tests with default renderer.
#ifndef __WXMSW__

TEST_CASE("GraphicsPathTestCase", "[path]")
{
    wxBitmap bmp(500, 500);
    wxMemoryDC mdc(bmp);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(mdc));
    REQUIRE(gc);
    DoAllTests(gc.get());
}

#else

#if wxUSE_GRAPHICS_GDIPLUS
TEST_CASE("GraphicsPathTestCaseGDIPlus")
{
    wxBitmap bmp(500, 500);
    wxMemoryDC mdc(bmp);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetGDIPlusRenderer()->CreateContext(mdc));
    REQUIRE(gc);
    DoAllTests(gc.get());
}
#endif // wxUSE_GRAPHICS_GDIPLUS

#if wxUSE_GRAPHICS_DIRECT2D
TEST_CASE("GraphicsPathTestCaseDirect2D")
{
    wxBitmap bmp(500, 500);
    wxMemoryDC mdc(bmp);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDirect2DRenderer()->CreateContext(mdc));
    REQUIRE(gc);
    DoAllTests(gc.get());
}
#endif // wxUSE_GRAPHICS_DIRECT2D

#endif // __WXMSW__ / !__WXMSW__

#if wxUSE_CAIRO
TEST_CASE("GraphicsPathTestCaseCairo", "[path][cairo]")
{
    wxBitmap bmp(500, 500);
    wxMemoryDC mdc(bmp);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetCairoRenderer()->CreateContext(mdc));
    REQUIRE(gc);
    DoAllTests(gc.get());
}
#endif // wxUSE_CAIRO

#define WX_CHECK_POINT(p1, p2, tolerance)      \
    CHECK(fabs(p1.m_x - p2.m_x) <= tolerance); \
    CHECK(fabs(p1.m_y - p2.m_y) <= tolerance)

static void TestCurrentPoint(wxGraphicsContext* gc)
{
    // No current point
    {
        wxGraphicsPath path = gc->CreatePath();
        // Should return (0, 0) if current point is not yet set.
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(0, 0), 0);
    }
    // MoveToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        wxPoint2DDouble pt(27, 35);
        path.MoveToPoint(pt);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, pt, 1E-3);
    }
    // AddLineToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        wxPoint2DDouble pt(27, 35);
        path.AddLineToPoint(pt);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, pt, 1E-3);
    }
    // AddLineToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(10, 18);
        wxPoint2DDouble pt(37, 45);
        path.AddLineToPoint(pt);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, pt, 1E-3);
    }
    // AddArc - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double r = 40;
        path.AddArc(x, y, r, 0, M_PI/2, true);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x, y + r), 1E-3);
    }
    // AddArc
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(20, 38);
        const double x = 200;
        const double y = 50;
        const double r = 40;
        path.AddArc(x, y, r, 0, M_PI / 2, true);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x, y + r), 1E-3);
    }
    // AddArcToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x1 = 80;
        const double y1 = 80;
        const double x2 = -30;
        const double y2 = y1;
        const double r = 20;
        wxASSERT(x1 == y1 && y2 == y1); // alpha = 45 deg
        double d = r / tan(45 / 180.0 * M_PI / 2.0);
        path.AddArcToPoint(x1, y1, x2, y2, r);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x1 - d, y2), 1E-3);
    }
    // AddArcToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 20;
        const double y0 = 20;
        path.MoveToPoint(x0, y0);
        const double x1 = 80;
        const double y1 = 80;
        const double x2 = 140;
        const double y2 = y1;
        const double r = 20;
        wxASSERT(x0 == y0 && x1 == y1 && y2 == y1); // alpha = 135 deg
        double d = r / tan(135 / 180.0 * M_PI / 2.0);
        path.AddArcToPoint(x1, y1, x2, y2, r);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x1 + d, y2), 1E-3);
    }
    // AddCurveToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x1 = 102;
        const double y1 = 230;
        const double x2 = 153;
        const double y2 = 25;
        const double x3 = 230;
        const double y3 = 128;
        path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x3, y3), 1E-3);
    }
    // AddCurveToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 25;
        const double y0 = 128;
        path.MoveToPoint(x0, y0);
        const double x1 = 102;
        const double y1 = 230;
        const double x2 = 153;
        const double y2 = 25;
        const double x3 = 230;
        const double y3 = 128;
        path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x3, y3), 1E-3);
    }
    // AddQuadCurveToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x1 = 200;
        const double y1 = 200;
        const double x2 = 300;
        const double y2 = 100;
        path.AddQuadCurveToPoint(x1, y1, x2, y2);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x2, y2), 1E-3);
    }
    // AddQuadCurveToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 20;
        const double y0 = 100;
        path.MoveToPoint(x0, y0);
        const double x1 = 200;
        const double y1 = 200;
        const double x2 = 300;
        const double y2 = 100;
        path.AddQuadCurveToPoint(x1, y1, x2, y2);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x2, y2), 1E-3);
    }
    // AddCircle - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double r = 30;
        path.AddCircle(x, y, r);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x + r, y), 1E-3);
    }
    // AddCircle
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 80);
        const double x = 100;
        const double y = 140;
        const double r = 40;
        path.AddCircle(x, y, r);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x + r, y), 1E-3);
    }
    // AddEllipse - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddEllipse(x, y, w, h);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x + w, y + h / 2), 1E-3);
    }
    // AddEllipse
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddEllipse(x, y, w, h);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x + w, y + h / 2), 1E-3);
    }
    // AddRectangle - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        path.AddRectangle(x, y, 40, 20);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x, y), 1E-3);
    }
    // AddRectangle
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        path.AddRectangle(x, y, 50, 30);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x, y), 1E-3);
    }
    // AddRoundedRectangle - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 5);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x + w, y + h / 2), 1E-3);
    }
    // AddRoundedRectangle - no current point, radius = 0
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        path.AddRoundedRectangle(x, y, 40, 20, 0); // Should behave like AddRectangle
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x, y), 1E-3);
    }
    // AddRoundedRectangle
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 5);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x + w, y + h / 2), 1E-3);
    }
    // AddRoundedRectangle - radius = 0
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 50;
        const double y0 = 60;
        path.MoveToPoint(x0, y0);
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 0); // Should behave like AddRectangle
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x, y), 1E-3);
    }
    // CloseSubpath - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 50;
        const double y0 = 80;
        path.AddLineToPoint(x0, y0);
        path.AddArcToPoint(100, 160, 50, 200, 30);
        path.CloseSubpath();
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x0, y0), 1E-3);
    }
    // CloseSubpath
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 10;
        const double y0 = 20;
        path.MoveToPoint(x0, y0);
        path.AddLineToPoint(50, 80);
        path.AddArcToPoint(100, 160, 50, 200, 30);
        path.CloseSubpath();
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, wxPoint2DDouble(x0, y0), 1E-3);
    }
    // AddPath - no current point
    {
        // Path to be added
        wxGraphicsPath path2 = gc->CreatePath();
        path2.AddArcToPoint(100, 160, 50, 200, 30);
        path2.AddLineToPoint(50, 80);
        path2.CloseSubpath();
        wxPoint2DDouble cp2 = path2.GetCurrentPoint();
        // Main path
        wxGraphicsPath path = gc->CreatePath();
        path.AddLineToPoint(50, 80);
        const double x = 100;
        const double y = 140;
        path.AddRectangle(x, y, 50, 200);
        path.AddPath(path2);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, cp2, 1E-3);
    }
    // AddPath
    {
        // Path to be added
        wxGraphicsPath path2 = gc->CreatePath();
        path2.AddArcToPoint(100, 160, 50, 200, 30);
        path2.AddLineToPoint(50, 80);
        path2.CloseSubpath();
        wxPoint2DDouble cp2 = path2.GetCurrentPoint();
        // Main path
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(15, 35);
        path.AddLineToPoint(50, 80);
        const double x = 100;
        const double y = 140;
        const double r = 20;
        path.AddCircle(x, y, r);
        path.AddPath(path2);
        wxPoint2DDouble cp = path.GetCurrentPoint();
        WX_CHECK_POINT(cp, cp2, 1E-3);
    }
}

#define WX_CHECK_BOX(r1, r2, tolerance)                           \
    WX_CHECK_POINT(r1.GetLeftTop(), r2.GetLeftTop(), tolerance); \
    WX_CHECK_POINT(r1.GetRightBottom(), r2.GetRightBottom(), tolerance)

static void TestBox(wxGraphicsContext* gc)
{
    // No current point
    {
        wxGraphicsPath path = gc->CreatePath();
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(0, 0, 0, 0), 0);
    }
    // MoveToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(28, 38);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(0, 0, 0, 0), 0);
    }
    // AddLineToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        path.AddLineToPoint(28, 36);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(0, 0, 0, 0), 0);
    }
    // AddLineToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 10;
        const double y0 = 18;
        path.MoveToPoint(x0, y0);
        const double w = 20;
        const double h = 46;
        path.AddLineToPoint(x0 + w, y0 + h);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, w, h), 0);
    }
    // AddArc - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double r = 40;
        path.AddArc(x, y, r, 0, M_PI / 2, true);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, r, r), 1E-3);
    }
    // AddArc
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 20;
        const double y0 = 20;
        path.MoveToPoint(x0, y0);
        const double x = 200;
        const double y = 50;
        const double r = 40;
        path.AddArc(x, y, r, 0, M_PI / 2, true);
        const double x2 = x + r;
        const double y2 = y + r;
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, x2 - x0, y2 - y0), 1E-3);
    }
    // AddArcToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x1 = 80;
        const double y1 = 0;
        const double x2 = x1;
        const double y2 = 40;
        const double r = 20;
        wxASSERT(y1 == 0 && x2 == x1); // alpha = 90 deg
        path.AddArcToPoint(x1, y1, x2, y2, r);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(0, 0, x1, r), 1E-3);
    }
    // AddArcToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 20;
        const double y0 = 20;
        path.MoveToPoint(x0, y0);
        const double x1 = 80;
        const double y1 = y0;
        const double x2 = x1;
        const double y2 = 140;
        const double r = 20;
        wxASSERT(y1 == y0 && x2 == x1); // alpha = 90 deg
        path.AddArcToPoint(x1, y1, x2, y2, r);
        const double xe = x1;
        const double ye = y1 + r;
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, xe - x0, ye - y0), 1E-3);
    }
    // AddCurveToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x1 = 102;
        const double y1 = 230;
        const double x2 = 153;
        const double y2 = 25;
        const double x3 = 230;
        const double y3 = 128;
        path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
        const double xmin = wxMin(wxMin(x1, x2), x3);
        const double ymin = wxMin(wxMin(y1, y2), y3);
        const double xmax = wxMax(wxMax(x1, x2), x3);
        const double ymax = wxMax(wxMax(y1, y2), y3);
        wxRect2DDouble b = path.GetBox();
        const double tolerance = 1E-3;
        CHECK(xmin - tolerance <= b.GetLeft());
        CHECK(ymin - tolerance <= b.GetTop());
        CHECK(xmax + tolerance >= b.GetRight());
        CHECK(ymax + tolerance >= b.GetBottom());
    }
    // AddCurveToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 25;
        const double y0 = 128;
        path.MoveToPoint(x0, y0);
        const double x1 = 102;
        const double y1 = 230;
        const double x2 = 153;
        const double y2 = 25;
        const double x3 = 230;
        const double y3 = 128;
        path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
        const double xmin = wxMin(wxMin(wxMin(x0, x1), x2), x3);
        const double ymin = wxMin(wxMin(wxMin(y0, y1), y2), y3);
        const double xmax = wxMax(wxMax(wxMax(x0, x1), x2), x3);
        const double ymax = wxMax(wxMax(wxMax(y0, y1), y2), y3);
        wxRect2DDouble b = path.GetBox();
        const double tolerance = 1E-3;
        CHECK(xmin - tolerance <= b.GetLeft());
        CHECK(ymin - tolerance <= b.GetTop());
        CHECK(xmax + tolerance >= b.GetRight());
        CHECK(ymax + tolerance >= b.GetBottom());
    }
    // AddQuadCurveToPoint - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x1 = 200;
        const double y1 = 200;
        const double x2 = 300;
        const double y2 = 100;
        path.AddQuadCurveToPoint(x1, y1, x2, y2);
        //        const double xmin = wxMin(x1, x2);
        const double xmin = 133;
        const double ymin = wxMin(y1, y2);
        const double xmax = wxMax(x1, x2);
        const double ymax = wxMax(y1, y2);
        wxRect2DDouble b = path.GetBox();
        const double tolerance = 1E-3;
        CHECK(xmin - tolerance <= b.GetLeft());
        CHECK(ymin - tolerance <= b.GetTop());
        CHECK(xmax + tolerance >= b.GetRight());
        CHECK(ymax + tolerance >= b.GetBottom());
    }
    // AddQuadCurveToPoint
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 20;
        const double y0 = 100;
        path.MoveToPoint(x0, y0);
        const double x1 = 200;
        const double y1 = 200;
        const double x2 = 300;
        const double y2 = 100;
        path.AddQuadCurveToPoint(x1, y1, x2, y2);
        const double xmin = wxMin(wxMin(x0, x1), x2);
        const double ymin = wxMin(wxMin(y0, y1), y2);
        const double xmax = wxMax(wxMax(x0, x1), x2);
        const double ymax = wxMax(wxMax(y0, y1), y2);
        wxRect2DDouble b = path.GetBox();
        const double tolerance = 1E-3;
        CHECK(xmin - tolerance <= b.GetLeft());
        CHECK(ymin - tolerance <= b.GetTop());
        CHECK(xmax + tolerance >= b.GetRight());
        CHECK(ymax + tolerance >= b.GetBottom());
    }
    // AddCircle - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double r = 30;
        path.AddCircle(x, y, r);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x - r, y - r, 2 * r, 2 * r), 1E-3);
    }
    // AddCircle
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 80);
        const double x = 100;
        const double y = 140;
        const double r = 40;
        path.AddCircle(x, y, r);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x - r, y - r, 2 * r, 2 * r), 1E-3);
    }
    // AddEllipse - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddEllipse(x, y, w, h);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddEllipse
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddEllipse(x, y, w, h);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddRectangle - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRectangle(x, y, w, h);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddRectangle
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 50;
        const double h = 30;
        path.AddRectangle(x, y, w, h);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddRoundedRectangle - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 5);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddRoundedRectangle - no current point, radius = 0
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 0);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddRoundedRectangle
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 5);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // AddRoundedRectangle - radius = 0
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 40;
        const double h = 20;
        path.AddRoundedRectangle(x, y, w, h, 0);
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
    // CloseSubpath - empty path
    {
        wxGraphicsPath path = gc->CreatePath();
        path.CloseSubpath();
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(0, 0, 0, 0), 0);
    }
    // CloseSubpath - no current point
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 50;
        const double y0 = 80;
        path.AddLineToPoint(x0, y0);
        const double x1 = 100;
        const double y1 = 160;
        path.AddLineToPoint(x1, y1);
        path.CloseSubpath();
        const double w = x1 - x0;
        const double h = y1 - y0;
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, w, h), 1E-3);
    }
    // CloseSubpath
    {
        wxGraphicsPath path = gc->CreatePath();
        const double x0 = 10;
        const double y0 = 20;
        path.MoveToPoint(x0, y0);
        path.AddLineToPoint(50, 80);
        const double x = 100;
        const double y = 160;
        path.AddLineToPoint(x, y);
        path.CloseSubpath();
        const double w = x - x0;
        const double h = y - y0;
        wxRect2DDouble b = path.GetBox();
        WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, w, h), 1E-3);
    }
    // AddPath - no current point
    {
        // Path to be added
        wxGraphicsPath path2 = gc->CreatePath();
        path2.AddLineToPoint(100, 160);
        path2.AddLineToPoint(50, 80);
        path2.CloseSubpath();
        wxRect2DDouble b2 = path2.GetBox();
        // Main path
        wxGraphicsPath path = gc->CreatePath();
        path.AddLineToPoint(50, 80);
        const double x = 100;
        const double y = 140;
        path.AddRectangle(x, y, 50, 200);
        wxRect2DDouble b0 = path.GetBox();
        b0.Union(b2);
        path.AddPath(path2);
        wxRect2DDouble b1 = path.GetBox();
        WX_CHECK_BOX(b0, b1, 1E-3);
    }
    // AddPath
    {
        // Path to be added
        wxGraphicsPath path2 = gc->CreatePath();
        path2.AddArcToPoint(100, 160, 50, 200, 30);
        path2.AddLineToPoint(50, 80);
        path2.CloseSubpath();
        wxRect2DDouble b2 = path2.GetBox();
        // Main path
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(15, 35);
        path.AddLineToPoint(50, 80);
        const double x = 100;
        const double y = 140;
        const double r = 20;
        path.AddCircle(x, y, r);
        wxRect2DDouble b0 = path.GetBox();
        b0.Union(b2);
        path.AddPath(path2);
        wxRect2DDouble b1 = path.GetBox();
        WX_CHECK_BOX(b0, b1, 1E-3);
    }
    // Overlapping figures
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double xr = 100;
        const double yr = 150;
        const double wr = 80;
        const double hr = 40;
        path.AddRectangle(xr, yr, wr, hr);
        const double xe = xr + wr / 4;
        const double ye = yr + hr / 4;
        const double we = wr / 2;
        const double he = hr / 2;
        path.AddEllipse(xe, ye, we, he);
        wxRect2DDouble b = path.GetBox();
        wxRect2DDouble r;
        wxRect2DDouble::Union(wxRect2DDouble(xe, ye, we, he), wxRect2DDouble(xr, yr, wr, hr), &r);
        WX_CHECK_BOX(b, r, 1E-3);
    }
    // Partially overlapping figures
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double xe = 100;
        const double ye = 150;
        const double we = 40;
        const double he = 20;
        path.AddEllipse(xe, ye, we, he);
        const double xr = xe + he / 2;
        const double yr = ye + we / 2;
        const double wr = we + 10;
        const double hr = he + 10;
        path.AddRectangle(xr, yr, wr, hr);
        wxRect2DDouble b = path.GetBox();
        wxRect2DDouble r;
        wxRect2DDouble::Union(wxRect2DDouble(xe, ye, we, he), wxRect2DDouble(xr, yr, wr, hr), &r);
        WX_CHECK_BOX(b, r, 1E-3);
    }
    // Non-overlapping figures
    {
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double xe = 100;
        const double ye = 150;
        const double we = 40;
        const double he = 20;
        path.AddEllipse(xe, ye, we, he);
        const double xr = xe + he + 10;
        const double yr = ye + we + 20;
        const double wr = 50;
        const double hr = 30;
        path.AddRectangle(xr, yr, wr, hr);
        wxRect2DDouble b = path.GetBox();
        wxRect2DDouble r;
        wxRect2DDouble::Union(wxRect2DDouble(xe, ye, we, he), wxRect2DDouble(xr, yr, wr, hr), &r);
        WX_CHECK_BOX(b, r, 1E-3);
    }
    // Path from transformed graphics context
    {
        gc->PushState();
        gc->Translate(5, 15);
        gc->Rotate(10 * M_PI / 180);
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(50, 60);
        const double x = 100;
        const double y = 150;
        const double w = 50;
        const double h = 30;
        path.AddRectangle(x, y, w, h);
        wxRect2DDouble b = path.GetBox();
        gc->PopState();
        WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
    }
}

static void DoAllTests(wxGraphicsContext* gc)
{
    gc->DisableOffset();
    TestCurrentPoint(gc);
    TestBox(gc);
}

#endif //  wxUSE_GRAPHICS_CONTEXT
