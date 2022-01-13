///////////////////////////////////////////////////////////////////////////////
// Name:        tests/geometry/size.cpp
// Purpose:     wxSize unit test
// Author:      Wlodzimierz ABX Skiba
// Created:     2004-12-14
// Copyright:   (c) 2004 wxWindows
///////////////////////////////////////////////////////////////////////////////

export module WX.Test.Size;

import WX.MetaTest;
import Utils.Geometry;

namespace ut = boost::ut;

ut::suite SizeOperators = []
{
    using namespace ut;

    wxSize s1{1, 2};
    wxSize s2{3, 4};
    wxSize s3 = s1 + s2;

    expect( s3.GetWidth() == 4 );
    expect( s3.GetHeight() == 6 );
    s3 = s2 - s1;
    expect( s3.GetWidth() == 2 );
    expect( s3.GetHeight()==2 );
    s3 = s1 * 2;
    expect( s3.GetWidth()==2 );
    expect( s3.GetHeight()==4 );
    s3 = 2 * s1;
    expect( s3.GetWidth()==2 );
    expect( s3.GetHeight()==4 );
    s3 = s3 / 2;
    expect( s3.GetWidth()==1 );
    expect( s3.GetHeight()==2 );

    s3 = s2;
    expect( s3 != s1 );
    s3 = s1;
    expect( s3 == s1 );
    s3 += s2;
    expect( s3.GetWidth()==4 );
    expect( s3.GetHeight()==6 );
    s3 -= s2;
    expect( s3 == s1 );
    s3 *= 2;
    expect( s3.GetWidth()==2 );
    expect( s3.GetHeight()==4 );
    s3 /= 2;
    expect( s3 == s1 );
};
