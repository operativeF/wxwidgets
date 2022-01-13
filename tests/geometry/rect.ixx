///////////////////////////////////////////////////////////////////////////////
// Name:        tests/geometry/rect.cpp
// Purpose:     wxRect unit test
// Author:      Vadim Zeitlin
// Created:     2004-12-11
// Copyright:   (c) 2004 wxWindows
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"

export module WX.Test.Rect;

import WX.Test.Prec;
import WX.MetaTest;

import <algorithm>;
import <array>;
import <ranges>;

namespace ut = boost::ut;

namespace
{

struct RectData
{
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    int x, y, w, h;

    wxRect GetFirst() const { return wxRect(x1, y1, w1, h1); }
    wxRect GetSecond() const { return wxRect(x2, y2, w2, h2); }
    wxRect GetResult() const { return wxRect(x, y, w, h); }
};

constexpr std::array<RectData, 7> s_rects =
{{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 4, 4, 1, 1, 1, 1, 4, 4 },
    { 1, 1, 2, 2, 4, 4, 1, 1, 1, 1, 4, 4 },
    { 2, 2, 2, 2, 4, 4, 4, 4, 2, 2, 6, 6 },
    { 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 4, 4 }
}};

} // namespace anonymous

ut::suite RectOperations = []
{
    using namespace ut;

    "Centre In"_test = []
    {
        using R = wxRect;

        expect( R{45, 45, 10, 10} == R(0, 0, 10, 10).CentreIn(R{0, 0, 100, 100}));

        expect( R{-5, -5, 20, 20} == R(0, 0, 20, 20).CentreIn(R{0, 0, 10, 10}));
    };

    // FIXME: Need to do items that return references for expect.
    // Currently compiles, and runs, but won't work.
    // "Inflate / Deflate"_test = []
    // {
    //     // This is the rectangle from the example in the documentation of wxRect::Inflate().
    //     wxRect r1{10, 10, 20, 40};

    //     r1.Inflate(10, 10);
    //     expect(r1 == wxRect{0, 0, 40, 60});

    //     r1.Inflate(20, 30);
    //     expect(r1 == wxRect{-10, -20, 60, 100});

    //     expect(r1.Inflate(-10, -10) == wxRect{ 20,  20,  0,  20});
    //     expect(r1.Inflate(-15, -15) == wxRect{ 20,  25,  0,  10});

    //     expect(r1.Inflate( 10,  10) == r1.Deflate(-10, -10));
    //     expect(r1.Inflate( 20,  30) == r1.Deflate(-20, -30));
    //     expect(r1.Inflate(-10, -10) == r1.Deflate( 10,  10));
    //     expect(r1.Inflate(-15, -15) == r1.Deflate( 15,  15));
    // };

    // test operator*() which returns the intersection of two rectangles
    "Intersection"_test = []
    {
        wxRect r1{0, 2, 3, 4};
        wxRect r2{1, 2, 7, 8};
        r1 *= r2;

        expect(wxRect{1, 2, 2, 4} == r1);
        expect( (r1 * wxRect()).IsEmpty() );
    };

    "Union"_test = []
    {
        std::ranges::for_each(s_rects,
            [](const auto& rect) {
                expect(rect.GetFirst().Union(rect.GetSecond()) == rect.GetResult());
                expect(rect.GetSecond().Union(rect.GetFirst()) == rect.GetResult());
            });
    };

    // test + operator which works like Union but does not ignore empty rectangles
    // FIXME: Need a way of determining what rect we failed on.
    // FIXME: Fails, but could be because of the compound operation required.
    // "+ operator test"_test = []
    // {
    //     std::ranges::for_each(s_rects,
    //         [](const auto& rect) {
    //             expect(rect.GetFirst() + rect.GetSecond() == rect.GetResult());
    //             expect(rect.GetSecond() + rect.GetFirst() == rect.GetResult());
    //         });
    // };
};
