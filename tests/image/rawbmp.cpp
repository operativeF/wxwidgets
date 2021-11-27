///////////////////////////////////////////////////////////////////////////////
// Name:        tests/image/rawbmp.cpp
// Purpose:     Test for using raw bitmap access classes with wxImage
// Author:      Vadim Zeitlin
// Created:     2008-05-30
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#ifdef wxHAS_RAW_BITMAP

#include "wx/rawbmp.h"

import WX.Test.Prec;

import WX.Image;

constexpr int WIDTH = 10;
constexpr int HEIGHT = 10;

#define ASSERT_COL_EQUAL(x, y) \
    CHECK_EQ( (unsigned char)(x), (y) )

TEST_CASE("RGBImage")
{
    // create a check board image
    wxImage image(WIDTH, HEIGHT);

    wxImagePixelData data(image);
    wxImagePixelData::Iterator p(data);
    for ( int y = 0; y < HEIGHT; y++ )
    {
        const wxImagePixelData::Iterator rowStart = p;

        for ( int x = 0; x < WIDTH; x++ )
        {
            p.Red() =
            p.Green() =
            p.Blue() = (x + y) % 2 ? 0 : -1;
            ++p;
        }

        p = rowStart;
        p.OffsetY(data, 1);
    }

    // test a few pixels
    ASSERT_COL_EQUAL( 0xff, image.GetRed(0, 0) );
    ASSERT_COL_EQUAL( 0xff, image.GetBlue(1, 1) );
    ASSERT_COL_EQUAL( 0, image.GetGreen(0, 1) );
    ASSERT_COL_EQUAL( 0, image.GetGreen(1, 0) );
}

#endif // wxHAS_RAW_BITMAP
