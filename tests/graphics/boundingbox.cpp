///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/boundingbox.cpp
// Purpose:     wxGCDC bounding box unit tests
// Author:      Vadim Zeitlin / Maarten Spoek / Toni Ruža
// Created:     2011-01-36
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
//              (c) 2014 Toni Ruža <toni.ruza@gmail.com>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#if wxUSE_GRAPHICS_CONTEXT

#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "wx/dcgraph.h"
#include "wx/icon.h"
#include "wx/colour.h"
#include "wx/gdicmn.h"
#include "wx/geometry/point.h"
#include "wx/geometry/size.h"

static void AssertBox(const wxGCDC* aGCDC, int minX, int minY, int width, int height, int margin = 0)
{
    const int maxX = minX + width;
    const int maxY = minY + height;

    // Allow for a margin of error due to different implementation
    // specificities regarding drawing paths.
    if ( margin )
    {
        #define WX_ASSERT_CLOSE(expected, actual, delta) \
            CHECK_MESSAGE(std::abs(actual - expected) <= delta, \
                         ("%d != %d", actual, expected))

        WX_ASSERT_CLOSE(minX, aGCDC->MinX(), margin);
        WX_ASSERT_CLOSE(minY, aGCDC->MinY(), margin);
        WX_ASSERT_CLOSE(maxX, aGCDC->MaxX(), margin);
        WX_ASSERT_CLOSE(maxY, aGCDC->MaxY(), margin);

        #undef WX_ASSERT_CLOSE
    }
    else
    {
        CHECK_EQ(minX, aGCDC->MinX());
        CHECK_EQ(minY, aGCDC->MinY());
        CHECK_EQ(maxX, aGCDC->MaxX());
        CHECK_EQ(maxY, aGCDC->MaxY());
    }
}



TEST_CASE("Bounding box tests.")
{

    wxBitmap m_bmp{wxSize{100, 100}};
    wxMemoryDC m_dc;

    m_dc.SelectObject(m_bmp);
    auto m_gcdc = std::make_unique<wxGCDC>(m_dc);

    m_gcdc->ResetBoundingBox();

    SUBCASE("DrawBitmap")
    {
        wxBitmap bitmap{wxSize{12, 12}};

        m_gcdc->DrawBitmap(bitmap, 5, 5);
        AssertBox(m_gcdc.get(), 5, 5, 12, 12);
    }

    SUBCASE("DrawIcon")
    {
        wxBitmap bitmap{wxSize{16, 16}};
        wxIcon icon;
        icon.CopyFromBitmap(bitmap);

        m_gcdc->DrawIcon(icon, 42, 42);
        AssertBox(m_gcdc.get(), 42, 42, 16, 16);
    }

    SUBCASE("DrawLine")
    {
        m_gcdc->DrawLine(10, 10, 20, 15);
        AssertBox(m_gcdc.get(), 10, 10, 10, 5);
    }

    SUBCASE("Crosshair")
    {
        wxSize gcdcSize = m_gcdc->GetSize();

        m_gcdc->CrossHair(33, 33);
        AssertBox(m_gcdc.get(), 0, 0, gcdcSize.x, gcdcSize.y);
    }

    SUBCASE("DrawArc")
    {
        m_gcdc->DrawArc(25, 30, 15, 40, 25, 40);  // quarter circle
        AssertBox(m_gcdc.get(), 15, 30, 10, 10, 3);
    }

    SUBCASE("DrawEllipticArc")
    {
        m_gcdc->DrawEllipticArc(40, 50, 30, 20, 0, 180);  // half circle
        AssertBox(m_gcdc.get(), 40, 50, 30, 10, 3);
    }

    SUBCASE("DrawPoint")
    {
        m_gcdc->DrawPoint(20, 20);
        AssertBox(m_gcdc.get(), 20, 20, 0, 0);
    }

    SUBCASE("DrawLines")
    {
        static const wxPoint points[4] = {
            {10, 20},
            {20, 10},
            {30, 20},
            {20, 30}
        };

        m_gcdc->DrawLines(4, points, 7, 8);
        AssertBox(m_gcdc.get(), 17, 18, 20, 20);
    }

#if wxUSE_SPLINES
    SUBCASE("DrawSpline")
    {
        static const wxPoint points[3] = {
            {10, 30},
            {20, 20},
            {40, 50}
        };

        m_gcdc->DrawSpline(3, points);
        AssertBox(m_gcdc.get(), 10, 20, 30, 30, 5);
    }
#endif  // wxUSE_SPLINES

    SUBCASE("DrawPolygon")
    {
        static const wxPoint points[3] = {
            {10, 30},
            {20, 10},
            {30, 30}
        };

        m_gcdc->DrawPolygon(3, points, -5, -7);
        AssertBox(m_gcdc.get(), 5, 3, 20, 20);
    }

    SUBCASE("DrawPolyPolygon")
    {
        static const int lengths[2] = {3, 3};

        static const wxPoint points[6] = {
            {10, 30},
            {20, 10},
            {30, 30},
            {20, 60},
            {30, 40},
            {40, 60}
        };

        m_gcdc->DrawPolyPolygon(2, lengths, points, 12, 5);
        AssertBox(m_gcdc.get(), 22, 15, 30, 50, 4);
    }

    SUBCASE("DrawRectangle")
    {
        m_gcdc->DrawRectangle(2, 2, 12, 12);
        AssertBox(m_gcdc.get(), 2, 2, 12, 12);
    }

    SUBCASE("DrawRoundedRectangle")
    {
        m_gcdc->DrawRoundedRectangle(27, 27, 12, 12, 2);
        AssertBox(m_gcdc.get(), 27, 27, 12, 12);
    }

    SUBCASE("DrawEllipse")
    {
        m_gcdc->DrawEllipse(54, 45, 23, 12);
        AssertBox(m_gcdc.get(), 54, 45, 23, 12);
    }

    SUBCASE("Blit")
    {
        wxBitmap bitmap{wxSize{20, 20}};

        wxMemoryDC dc(bitmap);

        m_gcdc->Blit(wxPoint{20, 10}, wxSize{12, 7}, &dc, wxPoint{0, 0});
        AssertBox(m_gcdc.get(), 20, 10, 12, 7);

        dc.SelectObject(wxNullBitmap);
    }

    SUBCASE("StretchBlit")
    {
        wxBitmap bitmap{wxSize{20, 20}};

        wxMemoryDC dc(bitmap);

        m_gcdc->StretchBlit(wxPoint{30, 50}, wxSize{5, 5}, &dc, wxPoint{0, 0}, wxSize{12, 4});
        AssertBox(m_gcdc.get(), 30, 50, 5, 5);

        dc.SelectObject(wxNullBitmap);
    }

    SUBCASE("DrawRotatedText")
    {
        std::string text("vertical");
        auto textExtent = m_gcdc->GetTextExtent(text);

        m_gcdc->DrawRotatedText(text, wxPoint{43, 22}, -90);
        AssertBox(m_gcdc.get(), 43 - textExtent.y, 22,
                                textExtent.y, textExtent.x, 3);
    }

    SUBCASE("wxDrawText")
    {
        std::string text("H");
        auto textExtent = m_gcdc->GetTextExtent(text);

        m_gcdc->wxDrawText(text, wxPoint{3, 3});
        AssertBox(m_gcdc.get(), 3, 3, textExtent.x, textExtent.y, 3);
    }

    SUBCASE("GradientFillLinear")
    {
        wxRect rect(16, 16, 30, 40);
        m_gcdc->GradientFillLinear(rect, *wxWHITE, *wxBLACK, wxNORTH);
        AssertBox(m_gcdc.get(), 16, 16, 30, 40);
    }

    SUBCASE("GradientFillConcentric")
    {
        wxRect rect(6, 6, 30, 40);
        m_gcdc->GradientFillConcentric(rect, *wxWHITE, *wxBLACK, wxPoint(10, 10));
        AssertBox(m_gcdc.get(), 6, 6, 30, 40);
    }

    SUBCASE("DrawCheckMark")
    {
        m_gcdc->DrawCheckMark(32, 24, 16, 16);
        AssertBox(m_gcdc.get(), 32, 24, 16, 16);
    }

    SUBCASE("DrawRectangleAndReset")
    {
        m_gcdc->DrawRectangle(2, 2, 12, 12);
        m_gcdc->ResetBoundingBox();
        AssertBox(m_gcdc.get(), 0, 0, 0, 0);
    }

    SUBCASE("DrawTwoRectangles")
    {
        m_gcdc->DrawRectangle(10, 15, 50, 30);
        m_gcdc->DrawRectangle(15, 20, 55, 35);
        AssertBox(m_gcdc.get(), 10, 15, 60, 40);
    }

    SUBCASE("DrawRectsOnTransformedDC")
    {
        m_gcdc->DrawRectangle(10, 15, 50, 30);
        m_gcdc->SetDeviceOrigin({15, 20});
        m_gcdc->DrawRectangle(15, 20, 45, 35);
        m_gcdc->SetDeviceOrigin({5, 10});
        AssertBox(m_gcdc.get(), 5, 5, 65, 60);
    }

    m_dc.SelectObject(wxNullBitmap);
    m_bmp = wxNullBitmap;

} // end TEST_CASE("Bounding box tests.")
#endif // wxUSE_GRAPHICS_CONTEXT
