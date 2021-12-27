///////////////////////////////////////////////////////////////////////////////
// Name:        tests/geometry/region.cpp
// Purpose:     wxRegion unit test
// Author:      Vadim Zeitlin
// Created:     2011-10-12
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

import WX.Test.Prec;

#include "wx/region.h"

import <ostream>;

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

namespace
{

// This function could be easily added to wxRegionIterator itself, where it
// could be implemented far more efficiently as all major platforms store the
// number of rectangles anyhow, but as we only use it for debugging purposes,
// just keep it here for now.
unsigned GetRectsCount(const wxRegion& rgn)
{
    unsigned count = 0;
    for ( wxRegionIterator iter(rgn); iter.HaveRects(); ++iter )
        count++;
    return count;
}

} // anonymous namespace

// this operator is needed to use CHECK_EQ with wxRegions
std::ostream& operator<<(std::ostream& os, const wxRegion& rgn)
{
    wxRect r = rgn.GetBox();
    os << "# rects = " << GetRectsCount(rgn)
       << "; bounding box {"
       << r.x << ", " << r.y << ", " << r.width << ", " << r.height
       << "}";
    return os;
}

TEST_CASE("Validity")
{
    wxRegion r;

    CHECK_MESSAGE
    (
        !r.IsOk(),
        "Default constructed region must be invalid"
    );

    CHECK_MESSAGE
    (
        r.IsEmpty(),
        "Invalid region should be empty"
    );

    // Offsetting an invalid region doesn't make sense.
    CHECK_THROWS( r.Offset(1, 1) );

    CHECK_MESSAGE
    (
        r.Union(0, 0, 10, 10),
        "Combining with a valid region should create a valid region"
    );

    CHECK_MESSAGE
    (
        wxRegion(0, 0, 10, 10) == r,
        "Union() with invalid region should give the same region"
    );
}

TEST_CASE("Intersect")
{
    static constexpr wxPoint points1[] = {
        wxPoint(310, 392),
        wxPoint(270, 392),
        wxPoint(270, 421),
        wxPoint(310, 421)
    };

    wxRegion region1(4, points1);

    static constexpr wxPoint points2[] = {
        wxPoint(54, 104),
        wxPoint(85,  82),
        wxPoint(68,  58),
        wxPoint(37,  80)
    };

    wxRegion region2(4, points2);

    CHECK( region1.Intersect(region2) );
    CHECK( region1.IsEmpty() );
}
