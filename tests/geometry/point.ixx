///////////////////////////////////////////////////////////////////////////////
// Name:        tests/geometry/point.cpp
// Purpose:     wxPoint unit test
// Author:      Wlodzimierz ABX Skiba
// Created:     2004-12-14
// Copyright:   (c) 2004 wxWindows
///////////////////////////////////////////////////////////////////////////////

export module WX.Test.Point;

import WX.Test.Prec;
import WX.MetaTest;

import Utils.Geometry;
import Utils.Strings;

import <cmath>;
import <numeric>;

namespace boost::ext::ut::v1_1_8::literals
{

template<StrLit TStr>
consteval auto operator""_pt()
{
    wxPoint result{};

    if(TStr.value[0] == '{' &&
       TStr.value[sizeof(TStr.value) - 2] == '}')
    {
        bool nocomma{false};
        bool negativex{false};
        bool negativey{false};

        for(const auto& c : TStr.value)
        {
            if(c == '-' && nocomma)
            {
                negativey = true;
            }

            if(c == '-' && !nocomma)
            {
                negativex = true;
            }

            if (c >= '0' && c <= '9' && !nocomma)
            {
                result.x = result.x * int(10) + int(c - '0');
            }

            if (c >= '0' && c <= '9' && nocomma)
            {
                result.y = result.y * int(10) + int(c - '0');
            }

            if(c == ',')
            {
                nocomma = true;
            }
        }

        if(negativex)
        {
            result.x *= int{-1};
        }

        if(negativey)
        {
            result.y *= int{-1};
        }

        return result;
    }

    return wxPoint{};
}

} // namespace literals

namespace ut = boost::ut;
using namespace ut::literals;

ut::suite PointOperations = []
{
    using namespace ut;

    "Equality"_test = []
    {
        wxPoint p1{1, 2};
        wxPoint p2{1, 2};

        expect(p1 == p2);
        expect(p1.x == p2.x);
        expect(p2.y == p2.y);
    };

    "Inequality"_test = []
    {
        wxPoint p1{2, 3};
        wxPoint p2{3, 4};

        expect(p1 != p2);
        expect(p1.x != p2.x);
        expect(p1.y != p2.y);
    };

    "Addition"_test = []
    {
        wxPoint p1{1, 1};
        wxPoint p2{2, -3};

        wxPoint p3 = p1 + p2;
        
        expect(p3 == "{3, -2}"_pt);
        expect(p3.x == 3);
        expect(p3.y == (-2));
    };

    "Subtraction"_test = []
    {
        wxPoint p1{0, 4};
        wxPoint p2{2, 1};

        wxPoint p3 = p1 - p2;

        expect(p3 == "{-2, 3}"_pt);
        expect(p3.x == (-2));
        expect(p3.y == 3);
    };
};

ut::suite RealPointOperations = []
{
    using namespace ut;

    "Equality"_test = []
    {
        wxRealPoint rp1{1.0, 2.2};
        wxRealPoint rp2{2.4, 3.3};

        auto rp3 = rp1 + rp2;

        expect(rp3 == wxRealPoint{3.4, 5.5});
        expect(std::abs(3.4 - rp3.x) <= std::numeric_limits<double>::epsilon());
        expect(std::abs(5.5 - rp3.y) <= std::numeric_limits<double>::epsilon());
    };
};

// TEST_CASE("Operators")
// {
//     wxPoint p1(1,2);
//     wxPoint p2(6,3);
//     wxPoint p3(7,5);
//     wxPoint p4(5,1);
//     wxPoint p5 = p2 + p1;
//     wxPoint p6 = p2 - p1;
//     CHECK( p3.x == p5.x );
//     CHECK( p3.y == p5.y );
//     CHECK( p4.x == p6.x );
//     CHECK( p4.y == p6.y );
//     CHECK( p3 == p5 );
//     CHECK( p4 == p6 );
//     CHECK( p3 != p4 );
//     p5 = p2; p5 += p1;
//     p6 = p2; p6 -= p1;
//     CHECK( p3 == p5 );
//     CHECK( p4 == p6 );
//     wxSize s(p1.x,p1.y);
//     p5 = p2; p5 = p2 + s;
//     p6 = p2; p6 = p2 - s;
//     CHECK( p3 == p5 );
//     CHECK( p4 == p6 );
//     p5 = p2; p5 = s + p2;
//     p6 = p2; p6 = s - p2;
//     CHECK( p3 == p5 );
//     CHECK( p4 == -p6 );
//     p5 = p2; p5 += s;
//     p6 = p2; p6 -= s;
//     CHECK( p3 == p5 );
//     CHECK( p4 == p6 );
// }

// TEST_CASE("Operators")
// {
//     const double EPSILON = 0.00001;
//     wxRealPoint p1(1.2,3.4);
//     wxRealPoint p2(8.7,5.4);
//     wxRealPoint p3(9.9,8.8);
//     wxRealPoint p4(7.5,2.0);
//     wxRealPoint p5 = p2 + p1;
//     wxRealPoint p6 = p2 - p1;
//     /*
//     CHECK( p3 == p5 );
//     CHECK( p4 == p6 );
//     CHECK( p3 != p4 );
//     */
//     CHECK( std::fabs( p3.x - p5.x ) < EPSILON );
//     CHECK( std::fabs( p3.y - p5.y ) < EPSILON );
//     CHECK( std::fabs( p4.x - p6.x ) < EPSILON );
//     CHECK( std::fabs( p4.y - p6.y ) < EPSILON );
// }
