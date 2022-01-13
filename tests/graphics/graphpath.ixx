///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/grappath.cpp
// Purpose:     graphics path unit tests
// Author:      Artur Wieczorek
// Created:     2018-07-01
// Copyright:   (c) 2018 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "wx/dcgraph.h"

#include <fmt/core.h>

export module WX.Test.GraphPath;

#if wxUSE_GRAPHICS_CONTEXT

import WX.Test.Prec;
import WX.MetaTest;

import <numbers>;

// For MSW we have individual test cases for each graphics renderer
// so we don't need to execute tests with default renderer.

#define WX_CHECK_POINT(p1, p2, tolerance)      \
    CHECK(std::fabs(p1.x - p2.x) <= tolerance); \
    CHECK(std::fabs(p1.y - p2.y) <= tolerance)

#define WX_CHECK_BOX(r1, r2, tolerance)                           \
    WX_CHECK_POINT(r1.GetLeftTop(), r2.GetLeftTop(), tolerance); \
    WX_CHECK_POINT(r1.GetRightBottom(), r2.GetRightBottom(), tolerance)


#if wxUSE_CAIRO
TEST_CASE("GraphicsPathTestCaseCairo", "[path][cairo]")
{
    wxBitmap bmp(wxSize{500, 500});
    wxMemoryDC mdc(bmp);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetCairoRenderer()->CreateContext(mdc));
    REQUIRE(gc);
}
#endif // wxUSE_CAIRO

namespace ut = boost::ut;

ut::suite GraphPathsTest = []
{
    using namespace ut;

    wxBitmap bmp{wxSize{500, 500}};
    wxMemoryDC mdc{bmp};

    const auto TestPointProximity = [](auto p1, auto p2, auto tol)
    {
        using namespace ut;

        expect(std::fabs(p1.x - p2.x) <= tol);
        expect(std::fabs(p1.y - p2.y) <= tol);
    };

    // FIXME: make this a little more clear up front;
#ifndef __WXMSW__
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(mdc));
#else
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDirect2DRenderer()->CreateContext(mdc));
#endif

    expect(gc != nullptr);

    gc->DisableOffset();

    "Points"_test = [&]
    {
        should("No current point.") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            // Should return (0, 0) if current point is not yet set.
            wxPoint2DFloat cp = path.GetCurrentPoint();
            TestPointProximity(cp, wxPoint2DFloat{}, 1E-3);
        };

        should("Move to point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            wxPoint2DFloat pt(27.0F, 35.0F);
            path.MoveToPoint(pt);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddLineToPoint - no current point.") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            wxPoint2DFloat pt(27.0F, 35.0F);
            path.AddLineToPoint(pt);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddLineToPoint") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(10.0F, 18.0F);
            wxPoint2DFloat pt(37.0F, 45.0F);
            path.AddLineToPoint(pt);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            TestPointProximity(cp, pt, 1E-3);
        };
    
        should("AddArc - no current point.") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            const float x = 100.0F;
            const float y = 150.0F;
            const float r = 40.0F;
            path.AddArc(x, y, r, 0.0F, std::numbers::pi_v<float> / 2.0F, true);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt = {x, y + r};
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddArc") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(20.0F, 38.0F);
            const float x = 200.0F;
            const float y = 50.0F;
            const float r = 40.0F;
            path.AddArc(x, y, r, 0.0F, std::numbers::pi_v<float> / 2.0F, true);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt = {x, y + r};
            TestPointProximity(cp, pt, 1E-3);
        };
        
        should("AddArcToPoint - no current point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            const float x1 = 80.0F;
            const float y1 = 80.0F;
            const float x2 = -30.0F;
            const float y2 = y1;
            const float r = 20.0F;

            float d = r / std::tan(45.0F / 180.0F * std::numbers::pi_v<float> / 2.0F);

            path.AddArcToPoint(x1, y1, x2, y2, r);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x1 - d, y2};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddArcToPoint") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();

            const float x0 = 20.0F;
            const float y0 = 20.0F;            
            const float x1 = 80.0F;
            const float y1 = 80.0F;
            const float x2 = 140.0F;
            const float y2 = y1;
            const float r = 20.0F;

            path.MoveToPoint(x0, y0);

            float d = r / std::tan(135.0F / 180.0F * std::numbers::pi_v<float> / 2.0F);

            path.AddArcToPoint(x1, y1, x2, y2, r);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x1 + d, y2};

            TestPointProximity(cp, pt, 1E-3);
        };
    
        should("AddCurveToPoint - no current point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();

            const float x1 = 102.0F;
            const float y1 = 230.0F;
            const float x2 = 153.0F;
            const float y2 = 25.0F;
            const float x3 = 230.0F;
            const float y3 = 128.0F;

            path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x3, y3};
            
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddCurveToPoint") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();

            const float x1 = 102.0F;
            const float y1 = 230.0F;
            const float x2 = 153.0F;
            const float y2 = 25.0F;
            const float x3 = 230.0F;
            const float y3 = 128.0F;

            path.MoveToPoint(25.0F, 128.0F);

            path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x3, y3};

            TestPointProximity(cp, pt, 1E-3);
        };
    
        should("AddQuadCurveToPoint - no current point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();

            const float x1 = 200.0F;
            const float y1 = 200.0F;
            const float x2 = 300.0F;
            const float y2 = 100.0F;

            path.AddQuadCurveToPoint(x1, y1, x2, y2);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x2, y2};
            
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddQuadCurveToPoint") = [&]
        {
            const float x1 = 200.0F;
            const float y1 = 200.0F;
            const float x2 = 300.0F;
            const float y2 = 100.0F;

            wxGraphicsPath path = gc->CreatePath();

            path.MoveToPoint(20.0F, 100.0F);

            path.AddQuadCurveToPoint(x1, y1, x2, y2);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x2, y2};
            
            TestPointProximity(cp, pt, 1E-3);
        };
    
        should("AddCircle - no current point") = [&]
        {
            const float x = 100.0F;
            const float y = 150.0F;
            const float r = 30.0F;

            wxGraphicsPath path = gc->CreatePath();
            path.AddCircle(x, y, r);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x + r, y};

            TestPointProximity(cp, pt, 1E-3);
        };
    
        should("AddCircle") = [&]
        {
            const float x = 100.0F;
            const float y = 140.0F;
            const float r = 40.0F;

            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(50.0F, 80.0F);
            path.AddCircle(x, y, r);
            
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x + r, y};

            TestPointProximity(cp, pt, 1E-3);
        };
    
        should("AddEllipse - no current point") = [&]
        {
            const float x = 100.0F;
            const float y = 150.0F;
            const float w = 40.0F;
            const float h = 20.0F;

            wxGraphicsPath path = gc->CreatePath();
            path.AddEllipse(x, y, w, h);

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x + w, y + h / 2.0F};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddEllipse") = [&]
        {
            const float x = 100.0F;
            const float y = 150.0F;
            const float w = 40.0F;
            const float h = 20.0F;
            
            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(50.0F, 60.0F);
            path.AddEllipse(x, y, w, h);

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x + w, y + h / 2.0F};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddRectangle - no current point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            const float x = 100.0F;
            const float y = 150.0F;
            path.AddRectangle(x, y, 40.0F, 20.0F);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x, y};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddRectangle") = [&]
        {
            const float x = 100.0F;
            const float y = 150.0F;

            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(50.0F, 60.0F);
            path.AddRectangle(x, y, 50.0F, 30.0F);

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x, y};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddRoundedRectangle - no current point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            const float x = 100.0F;
            const float y = 150.0F;
            const float w = 40.0F;
            const float h = 20.0F;
            path.AddRoundedRectangle(x, y, w, h, 5.0F);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x + w, y + h / 2.0F};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddRoundedRectangle - no current point, radius = 0") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();

            const float x = 100.0F;
            const float y = 150.0F;
            path.AddRoundedRectangle(x, y, 40.0F, 20.0F, 0.0F); // Should behave like AddRectangle
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x, y};
            
            TestPointProximity(cp, pt, 1E-3);
        };
        
        should("AddRoundedRectangle") = [&]
        {
            const float x = 100.0F;
            const float y = 150.0F;
            const float w = 40.0F;
            const float h = 20.0F;

            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(50.0F, 60.0F);
            path.AddRoundedRectangle(x, y, w, h, 5.0F);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x + w, y + h / 2.0F};
            
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddRoundedRectangle - radius = 0") = [&]
        {
            const float x = 100;
            const float y = 150;
            const float w = 40;
            const float h = 20;

            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(50.0F, 60.0F);
            path.AddRoundedRectangle(x, y, w, h, 0.0F); // Should behave like AddRectangle

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x, y};
            
            TestPointProximity(cp, pt, 1E-3);
        };

        should("CloseSubpath - no current point") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();
            const float x0 = 50.0F;
            const float y0 = 80.0F;
            path.AddLineToPoint(x0, y0);
            path.AddArcToPoint(100.0F, 160.0F, 50.0F, 200.0F, 30.0F);
            path.CloseSubpath();

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x0, y0};

            TestPointProximity(cp, pt, 1E-3);
        };

        should("CloseSubpath") = [&]
        {
            wxGraphicsPath path = gc->CreatePath();

            const float x0 = 10.0F;
            const float y0 = 20.0F;
            path.MoveToPoint(x0, y0);
            path.AddLineToPoint(50.0F, 80.0F);
            path.AddArcToPoint(100.0F, 160.0F, 50.0F, 200.0F, 30.0F);
            path.CloseSubpath();

            wxPoint2DFloat cp = path.GetCurrentPoint();
            wxPoint2DFloat pt{x0, y0};
            
            TestPointProximity(cp, pt, 1E-3);
        };

        should("AddPath - no current point") = [&]
        {
            // Path to be added
            wxGraphicsPath path2 = gc->CreatePath();
            path2.AddArcToPoint(100.0F, 160.0F, 50.0F, 200.0F, 30.0F);
            path2.AddLineToPoint(50.0F, 80.0F);
            path2.CloseSubpath();
            wxPoint2DFloat cp2 = path2.GetCurrentPoint();
            // Main path
            wxGraphicsPath path = gc->CreatePath();
            path.AddLineToPoint(50.0F, 80.0F);
            const float x = 100.0F;
            const float y = 140.0F;
            path.AddRectangle(x, y, 50.0F, 200.0F);
            path.AddPath(path2);
            wxPoint2DFloat cp = path.GetCurrentPoint();

            TestPointProximity(cp, cp2, 1E-3);
        };

        should("AddPath") = [&]
        {
            // Path to be added
            wxGraphicsPath path2 = gc->CreatePath();
            path2.AddArcToPoint(100.0F, 160.0F, 50.0F, 200.0F, 30.0F);
            path2.AddLineToPoint(50.0F, 80.0F);
            path2.CloseSubpath();
            wxPoint2DFloat cp2 = path2.GetCurrentPoint();
            // Main path
            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(15.0F, 35.0F);
            path.AddLineToPoint(50.0F, 80.0F);
            const float x = 100.0F;
            const float y = 140.0F;
            const float r = 20.0F;
            path.AddCircle(x, y, r);
            path.AddPath(path2);
            wxPoint2DFloat cp = path.GetCurrentPoint();
            
            TestPointProximity(cp, cp2, 1E-3);
        };
    };
};

// FIXME: Templatize for running Cairo / other renderers.
TEST_SUITE("Graphing path tests.")
{
    TEST_CASE("Boxes")
    {
        wxBitmap bmp{ wxSize{500, 500} };
        wxMemoryDC mdc{ bmp };

        // FIXME: make this a little more clear up front;
#ifndef __WXMSW__
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(mdc));
#else
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsRenderer::GetDirect2DRenderer()->CreateContext(mdc));
#endif

        REQUIRE(gc.get());

        gc->DisableOffset();

        /*
        SUBCASE("No current point")
        {
            wxGraphicsPath path = gc->CreatePath();
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble{0.0, 0.0, 0.0, 0.0}, 0.0);
        }

        SUBCASE("MoveToPoint")
        {
            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(28, 38);
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble{0.0, 0.0, 0.0, 0.0}, 0.0);
        }

        SUBCASE("AddLineToPoint - no current point")
        {
            wxGraphicsPath path = gc->CreatePath();
            path.AddLineToPoint(28, 36);
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble{0.0, 0.0, 0.0, 0.0}, 0.0);
        }

        */

        SUBCASE("AddLineToPoint")
        {
            wxGraphicsPath path = gc->CreatePath();
            const double x0 = 10;
            const double y0 = 18;
            path.MoveToPoint(x0, y0);
            const double w = 20;
            const double h = 46;
            path.AddLineToPoint(x0 + w, y0 + h);
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, w, h), 0.0);
        }

        SUBCASE("AddArc - no current point")
        {
            wxGraphicsPath path = gc->CreatePath();
            const double x = 100;
            const double y = 150;
            const double r = 40;
            path.AddArc(x, y, r, 0, std::numbers::pi_v<float> / 2.0, true);
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble(x, y, r, r), 1E-3);
        }

        SUBCASE("AddArc")
        {
            wxGraphicsPath path = gc->CreatePath();
            const double x0 = 20;
            const double y0 = 20;
            path.MoveToPoint(x0, y0);
            const double x = 200;
            const double y = 50;
            const double r = 40;
            path.AddArc(x, y, r, 0, std::numbers::pi_v<float> / 2.0, true);
            const double x2 = x + r;
            const double y2 = y + r;
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble(x0, y0, x2 - x0, y2 - y0), 1E-3);
        }

        SUBCASE("AddArcToPoint - no current point")
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
            WX_CHECK_BOX(b, wxRect2DDouble(0.0, 0.0, x1, r), 1E-3);
        }

        SUBCASE("AddArcToPoint")
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

        SUBCASE("AddCurveToPoint - no current point")
        {
            wxGraphicsPath path = gc->CreatePath();
            const double x1 = 102;
            const double y1 = 230;
            const double x2 = 153;
            const double y2 = 25;
            const double x3 = 230;
            const double y3 = 128;
            path.AddCurveToPoint(x1, y1, x2, y2, x3, y3);
            const double xmin = std::min(std::min(x1, x2), x3);
            const double ymin = std::min(std::min(y1, y2), y3);
            const double xmax = std::max(std::max(x1, x2), x3);
            const double ymax = std::max(std::max(y1, y2), y3);
            wxRect2DDouble b = path.GetBox();
            const double tolerance = 1E-3;
            CHECK(xmin - tolerance <= b.GetLeft());
            CHECK(ymin - tolerance <= b.GetTop());
            CHECK(xmax + tolerance >= b.GetRight());
            CHECK(ymax + tolerance >= b.GetBottom());
        }

        SUBCASE("AddCurveToPoint")
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
            const double xmin = std::min(std::min(std::min(x0, x1), x2), x3);
            const double ymin = std::min(std::min(std::min(y0, y1), y2), y3);
            const double xmax = std::max(std::max(std::max(x0, x1), x2), x3);
            const double ymax = std::max(std::max(std::max(y0, y1), y2), y3);
            wxRect2DDouble b = path.GetBox();
            const double tolerance = 1E-3;
            CHECK(xmin - tolerance <= b.GetLeft());
            CHECK(ymin - tolerance <= b.GetTop());
            CHECK(xmax + tolerance >= b.GetRight());
            CHECK(ymax + tolerance >= b.GetBottom());
        }

        SUBCASE("AddQuadCurveToPoint - no current point")
        {
            wxGraphicsPath path = gc->CreatePath();
            const double x1 = 200;
            const double y1 = 200;
            const double x2 = 300;
            const double y2 = 100;
            path.AddQuadCurveToPoint(x1, y1, x2, y2);
            //        const double xmin = std::min(x1, x2);
            const double xmin = 133;
            const double ymin = std::min(y1, y2);
            const double xmax = std::max(x1, x2);
            const double ymax = std::max(y1, y2);
            wxRect2DDouble b = path.GetBox();
            const double tolerance = 1E-3;
            CHECK(xmin - tolerance <= b.GetLeft());
            CHECK(ymin - tolerance <= b.GetTop());
            CHECK(xmax + tolerance >= b.GetRight());
            CHECK(ymax + tolerance >= b.GetBottom());
        }

        SUBCASE("AddQuadCurveToPoint")
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
            const double xmin = std::min(std::min(x0, x1), x2);
            const double ymin = std::min(std::min(y0, y1), y2);
            const double xmax = std::max(std::max(x0, x1), x2);
            const double ymax = std::max(std::max(y0, y1), y2);
            wxRect2DDouble b = path.GetBox();
            const double tolerance = 1E-3;
            CHECK(xmin - tolerance <= b.GetLeft());
            CHECK(ymin - tolerance <= b.GetTop());
            CHECK(xmax + tolerance >= b.GetRight());
            CHECK(ymax + tolerance >= b.GetBottom());
        }

        SUBCASE("AddCircle - no current point")
        {
            wxGraphicsPath path = gc->CreatePath();
            const double x = 100;
            const double y = 150;
            const double r = 30;
            path.AddCircle(x, y, r);
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble(x - r, y - r, 2 * r, 2 * r), 1E-3);
        }

        SUBCASE("AddCircle")
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

        SUBCASE("AddEllipse - no current point")
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

        SUBCASE("AddEllipse")
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

        SUBCASE("AddRectangle - no current point")
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

        SUBCASE("AddRectangle")
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

        SUBCASE("AddRoundedRectangle - no current point")
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

        SUBCASE("AddRoundedRectangle - no current point, radius = 0")
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

        SUBCASE("AddRoundedRectangle")
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

        SUBCASE("AddRoundedRectangle - radius = 0")
        {
            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(50, 60);
            const double x = 100;
            const double y = 150;
            const double w = 40;
            const double h = 20;
            path.AddRoundedRectangle(x, y, w, h, 0.0);
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble(x, y, w, h), 1E-3);
        }
        /*
        SUBCASE("CloseSubpath - empty path")
        {
            wxGraphicsPath path = gc->CreatePath();
            path.CloseSubpath();
            wxRect2DDouble b = path.GetBox();
            WX_CHECK_BOX(b, wxRect2DDouble{0.0, 0.0, 0.0, 0.0}, 0.0);
        }
        */
        SUBCASE("CloseSubpath - no current point")
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

        SUBCASE("CloseSubpath")
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

        SUBCASE("AddPath - no current point")
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

        SUBCASE("AddPath")
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

        SUBCASE("Overlapping figures")
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

        SUBCASE("Partially overlapping figures")
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

        SUBCASE("Non-overlapping figures")
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

        SUBCASE("Path from transformed graphics context")
        {
            gc->PushState();
            gc->Translate(5, 15);
            gc->Rotate(10.0F * std::numbers::pi_v<float> / 180.0F);
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
}

#endif //  wxUSE_GRAPHICS_CONTEXT
