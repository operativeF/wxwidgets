///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/boundingbox.cpp
// Purpose:     wxGCDC bounding box unit tests
// Author:      Vadim Zeitlin / Maarten Spoek / Toni Ruža
// Created:     2011-01-36
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
//              (c) 2014 Toni Ruža <toni.ruza@gmail.com>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "wx/dcgraph.h"
#include "wx/icon.h"
#include "wx/gdicmn.h"

#include <fmt/core.h>

export module WX.Test.BoundingBox;

import WX.Test.Prec;
import WX.MetaTest;

import Utils.Geometry;

import <cstdlib>;

#if wxUSE_GRAPHICS_CONTEXT

namespace ut = boost::ut;

void AssertBox(wxGCDC* aGCDC, int minX, int minY, int width, int height, int margin = 0)
{
    using namespace ut;

    const int maxX = minX + width;
    const int maxY = minY + height;

    // Allow for a margin of error due to different implementation
    // specificities regarding drawing paths.

    const auto AssertClose = [](auto expected, auto actual, auto delta)
    {
        expect(std::abs(actual - expected) <= delta) << fmt::format("{} != {}", actual, expected); 
    };

    if ( margin )
    {
        AssertClose(minX, aGCDC->MinX(), margin);
        AssertClose(minY, aGCDC->MinY(), margin);
        AssertClose(maxX, aGCDC->MaxX(), margin);
        AssertClose(maxY, aGCDC->MaxY(), margin);
    }
    else
    {
        expect(minX == aGCDC->MinX());
        expect(minY == aGCDC->MinY());
        expect(maxX == aGCDC->MaxX());
        expect(maxY == aGCDC->MaxY());
    }

    aGCDC->ResetBoundingBox();
}

ut::suite BoundingBoxTests = []
{
    using namespace ut;

    wxBitmap m_bmp{wxSize{100, 100}};
    wxMemoryDC m_dc;

    m_dc.SelectObject(m_bmp);
    auto m_gcdc = std::make_unique<wxGCDC>(m_dc);

    m_gcdc->ResetBoundingBox();

    "DrawBitmap"_test = [&]
    {
        wxBitmap bitmap{wxSize{12, 12}};

        m_gcdc->DrawBitmap(bitmap, 5, 5);
        AssertBox(m_gcdc.get(), 5, 5, 12, 12);
    };

    "DrawIcon"_test = [&]
    {
        wxBitmap bitmap{wxSize{16, 16}};
        wxIcon icon;
        icon.CopyFromBitmap(bitmap);

        m_gcdc->DrawIcon(icon, 42, 42);
        AssertBox(m_gcdc.get(), 42, 42, 16, 16);
    };

    "DrawLine"_test = [&]
    {
        m_gcdc->DrawLine(10, 10, 20, 15);
        AssertBox(m_gcdc.get(), 10, 10, 10, 5);
    };

    "Crosshair"_test = [&]
    {
        wxSize gcdcSize = m_gcdc->GetSize();

        m_gcdc->CrossHair(33, 33);
        AssertBox(m_gcdc.get(), 0, 0, gcdcSize.x, gcdcSize.y);
    };

    "DrawArc"_test = [&]
    {
        m_gcdc->DrawArc(25, 30, 15, 40, 25, 40);  // quarter circle
        AssertBox(m_gcdc.get(), 15, 30, 10, 10, 3);
    };

    "DrawEllipticArc"_test = [&]
    {
        m_gcdc->DrawEllipticArc(40, 50, 30, 20, 0, 180);  // half circle
        AssertBox(m_gcdc.get(), 40, 50, 30, 10, 3);
    };

    "DrawPoint"_test = [&]
    {
        m_gcdc->DrawPoint(20, 20);
        AssertBox(m_gcdc.get(), 20, 20, 0, 0);
    };

    "DrawLines"_test = [&]
    {
        static const wxPoint points[4] = {
            {10, 20},
            {20, 10},
            {30, 20},
            {20, 30}
        };

        m_gcdc->DrawLines(4, points, 7, 8);
        AssertBox(m_gcdc.get(), 17, 18, 20, 20);
    };

#if wxUSE_SPLINES
    "DrawSpline"_test = [&]
    {
        static const wxPoint points[3] = {
            {10, 30},
            {20, 20},
            {40, 50}
        };

        m_gcdc->DrawSpline(3, points);
        AssertBox(m_gcdc.get(), 10, 20, 30, 30, 5);
    };
#endif  // wxUSE_SPLINES

    "DrawPolygon"_test = [&]
    {
        static const wxPoint points[3] = {
            {10, 30},
            {20, 10},
            {30, 30}
        };

        m_gcdc->DrawPolygon(3, points, -5, -7);
        AssertBox(m_gcdc.get(), 5, 3, 20, 20);
    };

    "DrawPolyPolygon"_test = [&]
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
    };

    "DrawRectangle"_test = [&]
    {
        m_gcdc->DrawRectangle(2, 2, 12, 12);
        AssertBox(m_gcdc.get(), 2, 2, 12, 12);
    };

    "DrawRoundedRectangle"_test = [&]
    {
        m_gcdc->DrawRoundedRectangle(27, 27, 12, 12, 2);
        AssertBox(m_gcdc.get(), 27, 27, 12, 12);
    };

    "DrawEllipse"_test = [&]
    {
        m_gcdc->DrawEllipse(54, 45, 23, 12);
        AssertBox(m_gcdc.get(), 54, 45, 23, 12);
    };

    "Blit"_test = [&]
    {
        wxBitmap bitmap{wxSize{20, 20}};

        wxMemoryDC dc(bitmap);

        m_gcdc->Blit(wxPoint{20, 10}, wxSize{12, 7}, &dc, wxPoint{0, 0});
        AssertBox(m_gcdc.get(), 20, 10, 12, 7);

        dc.SelectObject(wxNullBitmap);
    };

    "StretchBlit"_test = [&]
    {
        wxBitmap bitmap{wxSize{20, 20}};

        wxMemoryDC dc(bitmap);

        m_gcdc->StretchBlit(wxPoint{30, 50}, wxSize{5, 5}, &dc, wxPoint{0, 0}, wxSize{12, 4});
        AssertBox(m_gcdc.get(), 30, 50, 5, 5);

        dc.SelectObject(wxNullBitmap);
    };

    "DrawRotatedText"_test = [&]
    {
        std::string text("vertical");
        auto textExtent = m_gcdc->GetTextExtent(text);

        m_gcdc->DrawRotatedText(text, wxPoint{43, 22}, -90);
        AssertBox(m_gcdc.get(), 43 - textExtent.y, 22,
                                textExtent.y, textExtent.x, 3);
    };

    "wxDrawText"_test = [&]
    {
        std::string text("H");
        auto textExtent = m_gcdc->GetTextExtent(text);

        m_gcdc->wxDrawText(text, wxPoint{3, 3});
        AssertBox(m_gcdc.get(), 3, 3, textExtent.x, textExtent.y, 3);
    };

    "GradientFillLinear"_test = [&]
    {
        wxRect rect(16, 16, 30, 40);
        m_gcdc->GradientFillLinear(rect, *wxWHITE, *wxBLACK, wxNORTH);
        AssertBox(m_gcdc.get(), 16, 16, 30, 40);
    };

    "GradientFillConcentric"_test = [&]
    {
        wxRect rect(6, 6, 30, 40);
        m_gcdc->GradientFillConcentric(rect, *wxWHITE, *wxBLACK, wxPoint(10, 10));
        AssertBox(m_gcdc.get(), 6, 6, 30, 40);
    };

    "DrawCheckMark"_test = [&]
    {
        m_gcdc->DrawCheckMark(32, 24, 16, 16);
        AssertBox(m_gcdc.get(), 32, 24, 16, 16);
    };

    "DrawRectangleAndReset"_test = [&]
    {
        m_gcdc->DrawRectangle(2, 2, 12, 12);
        m_gcdc->ResetBoundingBox();
        AssertBox(m_gcdc.get(), 0, 0, 0, 0);
    };

    "DrawTwoRectangles"_test = [&]
    {
        m_gcdc->DrawRectangle(10, 15, 50, 30);
        m_gcdc->DrawRectangle(15, 20, 55, 35);
        AssertBox(m_gcdc.get(), 10, 15, 60, 40);
    };

    "DrawRectsOnTransformedDC"_test = [&]
    {
        m_gcdc->DrawRectangle(10, 15, 50, 30);
        m_gcdc->SetDeviceOrigin({15, 20});
        m_gcdc->DrawRectangle(15, 20, 45, 35);
        m_gcdc->SetDeviceOrigin({5, 10});
        AssertBox(m_gcdc.get(), 5, 5, 65, 60);
    };

    m_dc.SelectObject(wxNullBitmap);
    m_bmp = wxNullBitmap;

};

#endif // wxUSE_GRAPHICS_CONTEXT
