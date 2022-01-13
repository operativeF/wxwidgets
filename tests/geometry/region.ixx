// Original by:
///////////////////////////////////////////////////////////////////////////////
// Name:        tests/geometry/region.ixx
// Purpose:     wxRegion unit test
// Author:      Vadim Zeitlin
// Created:     2011-10-12
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/region.h"

export module WX.Test.Region;

import WX.MetaTest;
import WX.Test.Prec;

import <ostream>;

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

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

namespace ut = boost::ut;

ut::suite RegionTests = []
{
    using namespace ut;

    "Validity"_test = []
    {
        wxRegion r{};

        expect(!r.IsOk()) <<
            "Default constructed region must be invalid";

        expect(r.IsEmpty()) <<
            "Invalid region should be empty";

        expect(throws([&r] { r.Offset(1, 1); })) <<
            "Should throw; offsetting an invalid region doesn't make sense.";

        expect(r.Union(0, 0, 10, 10)) <<
            "Combining with a valid region should create a valid region";

        expect(wxRegion(0, 0, 10, 10) == r)
             << "Union() with invalid region should give the same region";
    };

    "Intersect"_test = []
    {
        static constexpr wxPoint points1[] = {
            wxPoint{310, 392},
            wxPoint{270, 392},
            wxPoint{270, 421},
            wxPoint{310, 421}
        };

        wxRegion region1{4, points1};

        static constexpr wxPoint points2[] = {
            wxPoint{54, 104},
            wxPoint{85,  82},
            wxPoint{68,  58},
            wxPoint{37,  80}
        };

        wxRegion region2{4, points2};

        expect( region1.Intersect(region2) );
        expect( region1.IsEmpty() );
    };
};