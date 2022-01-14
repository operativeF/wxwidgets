///////////////////////////////////////////////////////////////////////////////
// Name:        tests/image/image.cpp
// Purpose:     Test wxImage
// Author:      Francesco Montorsi
// Created:     2009-05-31
// Copyright:   (c) 2009 Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/colour.h"
#include "wx/bitmap.h"
#include "wx/cursor.h"
#include "wx/icon.h"
#include "wx/palette.h"
#include "wx/log.h"
#include "wx/utils.h"

#include "wx/url.h"
#include "wx/clipbrd.h"
#include "wx/dataobj.h"

#include <fmt/core.h>

export module WX.Test.Image;

import WX.Image;
import WX.Test.Prec;

import WX.Cmn.WFStream;
import WX.Cmn.MemStream;
import WX.Cmn.ZStream;

import WX.MetaTest;

import <algorithm>;
import <iostream>;
import <numeric>;
import <string>;

#if wxUSE_IMAGE

namespace ut = boost::ut;

// FIXME: Define equality operator for colors.
#define CHECK_EQUAL_COLOUR_RGB(c1, c2) \
    expect( (int)c1.Red()   == (int)c2.Red() ); \
    expect( (int)c1.Green() == (int)c2.Green() ); \
    expect( (int)c1.Blue()  == (int)c2.Blue() )

#define CHECK_EQUAL_COLOUR_RGBA(c1, c2) \
    expect( (int)c1.Red()   == (int)c2.Red() ); \
    expect( (int)c1.Green() == (int)c2.Green() ); \
    expect( (int)c1.Blue()  == (int)c2.Blue() ); \
    expect( (int)c1.Alpha() == (int)c2.Alpha() )

enum
{
    wxIMAGE_HAVE_ALPHA = (1 << 0),
    wxIMAGE_HAVE_PALETTE = (1 << 1)
};

struct testData {
    const char* file;
    wxBitmapType type;
    unsigned bitDepth;
};

constexpr auto g_testfiles = std::to_array<testData>
({
    { "image/data/horse.ico", wxBitmapType::ICO, 4 },
    { "image/data/horse.xpm", wxBitmapType::XPM, 8 },
    { "image/data/horse.png", wxBitmapType::PNG, 24 },
    { "image/data/horse.ani", wxBitmapType::ANI, 24 },
    { "image/data/horse.bmp", wxBitmapType::BMP, 8 },
    { "image/data/horse.cur", wxBitmapType::CUR, 1 },
    { "image/data/horse.gif", wxBitmapType::GIF, 8 },
#if wxUSE_JPEG
    { "image/data/horse.jpg", wxBitmapType::JPEG, 24 },
#endif // wxUSE_JPEG
    { "image/data/horse.pcx", wxBitmapType::PCX, 8 },
    { "image/data/horse.pnm", wxBitmapType::PNM, 24 },
    { "image/data/horse.tga", wxBitmapType::TGA, 8 },
    { "image/data/horse.tif", wxBitmapType::TIFF, 8 }
});

void SetAlpha(wxImage* image)
{
    image->SetAlpha();

    unsigned char* ptr = image->GetAlpha();
    const int width = image->GetWidth();
    const int height = image->GetHeight();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            ptr[y * width + x] = (x * y) & wxIMAGE_ALPHA_OPAQUE;
        }
    }
}

void CompareImage(const wxImageHandler& handler, const wxImage& image,
    int properties = 0, const wxImage* compareTo = nullptr)
{
    using namespace ut;

    wxBitmapType type = handler.GetType();

    const bool testPalette = (properties & wxIMAGE_HAVE_PALETTE) != 0;
    /*
    This is getting messy and should probably be transformed into a table
    with image format features before it gets hairier.
    */
    if (testPalette
        && (!(type == wxBitmapType::BMP
            || type == wxBitmapType::GIF
            || type == wxBitmapType::ICO
            || type == wxBitmapType::PNG)
            || type == wxBitmapType::XPM))
    {
        return;
    }

    const bool testAlpha = (properties & wxIMAGE_HAVE_ALPHA) != 0;
    if (testAlpha
        && !(type == wxBitmapType::PNG || type == wxBitmapType::TGA
            || type == wxBitmapType::TIFF))
    {
        // don't test images with alpha if this handler doesn't support alpha
        return;
    }

    if (type == wxBitmapType::JPEG /* skip lossy JPEG */)
    {
        return;
    }

    wxMemoryOutputStream memOut;
    if (!image.SaveFile(memOut, type))
    {
        // Unfortunately we can't know if the handler just doesn't support
        // saving images, or if it failed to save.
        return;
    }

    wxMemoryInputStream memIn(memOut);
    expect(memIn.IsOk());

    wxImage actual(memIn);
    expect(actual.IsOk());

    // FIXME: RGBSameAs
    //const wxImage* expected = compareTo ? compareTo : &image;

    unsigned bitsPerPixel = testPalette ? 8 : (testAlpha ? 32 : 24);
    //INFO("Compare test '%s (%d-bit)' for saving",
    //    handler.GetExtension(), bitsPerPixel);
    //CHECK_THAT(actual, RGBSameAs(*expected));

#if wxUSE_PALETTE
    expect(actual.HasPalette()
        == (testPalette || type == wxBitmapType::XPM));
#endif

    expect(actual.HasAlpha() == testAlpha);

    if (!testAlpha)
    {
        return;
    }

    //INFO("Compare alpha test '%s' for saving", handler.GetExtension());
    // FIXME: Find better way.
    //CHECK_THAT(actual, RGBSameAs(*expected));
}

#if wxUSE_LIBTIFF
void TestTIFFImage(std::string_view option, int value,
    const wxImage* compareImage = nullptr)
{
    using namespace ut;

    wxImage image;
    if (compareImage)
    {
        image = *compareImage;
    }
    else
    {
        (void)image.LoadFile("image/data/horse.png");
    }
    expect(image.IsOk());

    wxMemoryOutputStream memOut;
    image.SetOption(option, value);

    expect(image.SaveFile(memOut, wxBitmapType::TIFF));

    wxMemoryInputStream memIn(memOut);
    expect(memIn.IsOk());

    wxImage savedImage(memIn);
    expect(savedImage.IsOk());

    expect(savedImage.HasOption(option) == true) << fmt::format("While checking for option %s", option);

    expect(value == savedImage.GetOptionInt(option)) << fmt::format("While testing for %s", option);

    expect(image.HasAlpha() == savedImage.HasAlpha()) << "HasAlpha() not equal";
}
#endif // wxUSE_LIBTIFF

#if wxUSE_GIF

void TestGIFComment(std::string_view comment)
{
    using namespace ut;

    wxImage image("image/data/horse.gif");

    image.SetOption(wxIMAGE_OPTION_GIF_COMMENT, comment);
    wxMemoryOutputStream memOut;
    expect(image.SaveFile(memOut, wxBitmapType::GIF));

    wxMemoryInputStream memIn(memOut);
    expect(image.LoadFile(memIn));

    expect(comment ==
        image.GetOption(wxIMAGE_OPTION_GIF_COMMENT));
}

#endif // wxUSE_GIF

void CompareBMPImage(const std::string& file1, const std::string& file2)
{
    using namespace ut;

    wxImage image1(file1);
    expect(image1.IsOk());

    wxImage image2(file2);
    expect(image2.IsOk());

    CompareImage(*wxImage::FindHandler(wxBitmapType::BMP), image1, 0, &image2);
}

int FindMaxChannelDiff(const wxImage& i1, const wxImage& i2)
{
    if (i1.GetWidth() != i2.GetWidth())
        return false;

    if (i1.GetHeight() != i2.GetHeight())
        return false;

    const unsigned char* p1 = i1.GetData();
    const unsigned char* p2 = i2.GetData();
    const int numBytes = i1.GetWidth() * i1.GetHeight() * 3;
    int maxDiff = 0;
    for (int n = 0; n < numBytes; n++, p1++, p2++)
    {
        const int diff = std::abs(*p1 - *p2);
        if (diff > maxDiff)
            maxDiff = diff;
    }

    return maxDiff;
}

// Note that we accept up to one pixel difference, this happens because of
// different rounding behaviours in different compiler versions
// even under the same architecture, see the example in
// http://thread.gmane.org/gmane.comp.lib.wxwidgets.devel/151149/focus=151154

// The 0 below can be replaced with 1 to generate, instead of comparing with,
// the test files.
#define ASSERT_IMAGE_EQUAL_TO_FILE(image, file) \
    if ( 0 ) \
    { \
        CHECK_MESSAGE( image.SaveFile(file), "Failed to save " file ); \
    } \
    else \
    { \
        const wxImage imageFromFile(file); \
        if ( imageFromFile.IsOk() ) \
        { \
            INFO("Wrong scaled \"" << file << "\" " << doctest::toString(image)); \
            CHECK(FindMaxChannelDiff(imageFromFile, image) <= 1); \
        } \
        else \
        { \
            FAIL("Failed to load \"" << file << "\""); \
        } \
    }

ut::suite BaseImageTest = []
{
    using namespace ut;

    // the formats we're going to test:
    wxImage::AddHandler(new wxICOHandler);
    wxImage::AddHandler(new wxXPMHandler);
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxANIHandler);
    wxImage::AddHandler(new wxBMPHandler);
    wxImage::AddHandler(new wxCURHandler);
#if wxUSE_GIF
    wxImage::AddHandler(new wxGIFHandler);
#endif // wxUSE_GIF
#if wxUSE_JPEG
    wxImage::AddHandler(new wxJPEGHandler);
#endif
    wxImage::AddHandler(new wxPCXHandler);
    wxImage::AddHandler(new wxPNMHandler);
    wxImage::AddHandler(new wxTGAHandler);
#if wxUSE_LIBTIFF
    wxImage::AddHandler(new wxTIFFHandler);
#endif // wxUSE_LIBTIFF

    "LoadFromFile"_test = []
    {
        wxImage img;
        for (const auto& testFile : g_testfiles)
            expect(img.LoadFile(testFile.file)) << fmt::format("{}", testFile.file);
    };

    "SizeImage"_test = []
    {
       // Test the wxImage::Size() function which takes a rectangle from source and
       // places it in a new image at a given position. This test checks, if the
       // correct areas are chosen, and clipping is done correctly.

       // our test image:
       static const char * xpm_orig[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "     .....",
          " ++++@@@@.",
          " +...   @.",
          " +.@@++ @.",
          " +.@ .+ @.",
          ".@ +. @.+ ",
          ".@ ++@@.+ ",
          ".@   ...+ ",
          ".@@@@++++ ",
          ".....     "
       };
       // the expected results for all tests:
       static const char * xpm_l_t[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "...   @.BB",
          ".@@++ @.BB",
          ".@ .+ @.BB",
          " +. @.+ BB",
          " ++@@.+ BB",
          "   ...+ BB",
          "@@@++++ BB",
          "...     BB",
          "BBBBBBBBBB",
          "BBBBBBBBBB"
       };
       static const char * xpm_t[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          " +...   @.",
          " +.@@++ @.",
          " +.@ .+ @.",
          ".@ +. @.+ ",
          ".@ ++@@.+ ",
          ".@   ...+ ",
          ".@@@@++++ ",
          ".....     ",
          "BBBBBBBBBB",
          "BBBBBBBBBB"
       };
       static const char * xpm_r_t[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB +...   ",
          "BB +.@@++ ",
          "BB +.@ .+ ",
          "BB.@ +. @.",
          "BB.@ ++@@.",
          "BB.@   ...",
          "BB.@@@@+++",
          "BB.....   ",
          "BBBBBBBBBB",
          "BBBBBBBBBB"
       };
       static const char * xpm_l[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "   .....BB",
          "+++@@@@.BB",
          "...   @.BB",
          ".@@++ @.BB",
          ".@ .+ @.BB",
          " +. @.+ BB",
          " ++@@.+ BB",
          "   ...+ BB",
          "@@@++++ BB",
          "...     BB"
       };
       static const char * xpm_r[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB     ...",
          "BB ++++@@@",
          "BB +...   ",
          "BB +.@@++ ",
          "BB +.@ .+ ",
          "BB.@ +. @.",
          "BB.@ ++@@.",
          "BB.@   ...",
          "BB.@@@@+++",
          "BB.....   "
       };
       static const char * xpm_l_b[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBB",
          "BBBBBBBBBB",
          "   .....BB",
          "+++@@@@.BB",
          "...   @.BB",
          ".@@++ @.BB",
          ".@ .+ @.BB",
          " +. @.+ BB",
          " ++@@.+ BB",
          "   ...+ BB"
       };
       static const char * xpm_b[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBB",
          "BBBBBBBBBB",
          "     .....",
          " ++++@@@@.",
          " +...   @.",
          " +.@@++ @.",
          " +.@ .+ @.",
          ".@ +. @.+ ",
          ".@ ++@@.+ ",
          ".@   ...+ "
       };
       static const char * xpm_r_b[] = {
          "10 10 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBB",
          "BBBBBBBBBB",
          "BB     ...",
          "BB ++++@@@",
          "BB +...   ",
          "BB +.@@++ ",
          "BB +.@ .+ ",
          "BB.@ +. @.",
          "BB.@ ++@@.",
          "BB.@   ..."
       };
       static const char * xpm_sm[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "     .....",
          " ++++@@@",
          " +...   ",
          " +.@@++ ",
          " +.@ .+ ",
          ".@ +. @.",
          ".@ ++@@.",
          ".@   ..."
       };
       static const char * xpm_gt[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "     .....BB",
          " ++++@@@@.BB",
          " +...   @.BB",
          " +.@@++ @.BB",
          " +.@ .+ @.BB",
          ".@ +. @.+ BB",
          ".@ ++@@.+ BB",
          ".@   ...+ BB",
          ".@@@@++++ BB",
          ".....     BB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_l_t[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "...   @.BBBB",
          ".@@++ @.BBBB",
          ".@ .+ @.BBBB",
          " +. @.+ BBBB",
          " ++@@.+ BBBB",
          "   ...+ BBBB",
          "@@@++++ BBBB",
          "...     BBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_l[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "   .....BBBB",
          "+++@@@@.BBBB",
          "...   @.BBBB",
          ".@@++ @.BBBB",
          ".@ .+ @.BBBB",
          " +. @.+ BBBB",
          " ++@@.+ BBBB",
          "   ...+ BBBB",
          "@@@++++ BBBB",
          "...     BBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_l_b[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "   .....BBBB",
          "+++@@@@.BBBB",
          "...   @.BBBB",
          ".@@++ @.BBBB",
          ".@ .+ @.BBBB",
          " +. @.+ BBBB",
          " ++@@.+ BBBB",
          "   ...+ BBBB",
          "@@@++++ BBBB",
          "...     BBBB"
       };
       static const char * xpm_gt_l_bb[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "   .....BBBB",
          "+++@@@@.BBBB",
          "...   @.BBBB",
          ".@@++ @.BBBB",
          ".@ .+ @.BBBB",
          " +. @.+ BBBB",
          " ++@@.+ BBBB",
          "   ...+ BBBB"
       };
       static const char * xpm_gt_t[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          " +...   @.BB",
          " +.@@++ @.BB",
          " +.@ .+ @.BB",
          ".@ +. @.+ BB",
          ".@ ++@@.+ BB",
          ".@   ...+ BB",
          ".@@@@++++ BB",
          ".....     BB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_b[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "     .....BB",
          " ++++@@@@.BB",
          " +...   @.BB",
          " +.@@++ @.BB",
          " +.@ .+ @.BB",
          ".@ +. @.+ BB",
          ".@ ++@@.+ BB",
          ".@   ...+ BB",
          ".@@@@++++ BB",
          ".....     BB"
       };
       static const char * xpm_gt_bb[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "     .....BB",
          " ++++@@@@.BB",
          " +...   @.BB",
          " +.@@++ @.BB",
          " +.@ .+ @.BB",
          ".@ +. @.+ BB",
          ".@ ++@@.+ BB",
          ".@   ...+ BB"
       };
       static const char * xpm_gt_r_t[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB +...   @.",
          "BB +.@@++ @.",
          "BB +.@ .+ @.",
          "BB.@ +. @.+ ",
          "BB.@ ++@@.+ ",
          "BB.@   ...+ ",
          "BB.@@@@++++ ",
          "BB.....     ",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_r[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB     .....",
          "BB ++++@@@@.",
          "BB +...   @.",
          "BB +.@@++ @.",
          "BB +.@ .+ @.",
          "BB.@ +. @.+ ",
          "BB.@ ++@@.+ ",
          "BB.@   ...+ ",
          "BB.@@@@++++ ",
          "BB.....     ",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_r_b[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BB     .....",
          "BB ++++@@@@.",
          "BB +...   @.",
          "BB +.@@++ @.",
          "BB +.@ .+ @.",
          "BB.@ +. @.+ ",
          "BB.@ ++@@.+ ",
          "BB.@   ...+ ",
          "BB.@@@@++++ ",
          "BB.....     "
       };
       static const char * xpm_gt_r_bb[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BB     .....",
          "BB ++++@@@@.",
          "BB +...   @.",
          "BB +.@@++ @.",
          "BB +.@ .+ @.",
          "BB.@ +. @.+ ",
          "BB.@ ++@@.+ ",
          "BB.@   ...+ "
       };
       static const char * xpm_gt_rr_t[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBB +...   ",
          "BBBB +.@@++ ",
          "BBBB +.@ .+ ",
          "BBBB.@ +. @.",
          "BBBB.@ ++@@.",
          "BBBB.@   ...",
          "BBBB.@@@@+++",
          "BBBB.....   ",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_rr[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBB     ...",
          "BBBB ++++@@@",
          "BBBB +...   ",
          "BBBB +.@@++ ",
          "BBBB +.@ .+ ",
          "BBBB.@ +. @.",
          "BBBB.@ ++@@.",
          "BBBB.@   ...",
          "BBBB.@@@@+++",
          "BBBB.....   ",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB"
       };
       static const char * xpm_gt_rr_b[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBB     ...",
          "BBBB ++++@@@",
          "BBBB +...   ",
          "BBBB +.@@++ ",
          "BBBB +.@ .+ ",
          "BBBB.@ +. @.",
          "BBBB.@ ++@@.",
          "BBBB.@   ...",
          "BBBB.@@@@+++",
          "BBBB.....   "
       };
       static const char * xpm_gt_rr_bb[] = {
          "12 12 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBBBBBBBBBB",
          "BBBB     ...",
          "BBBB ++++@@@",
          "BBBB +...   ",
          "BBBB +.@@++ ",
          "BBBB +.@ .+ ",
          "BBBB.@ +. @.",
          "BBBB.@ ++@@.",
          "BBBB.@   ..."
       };
       static const char * xpm_sm_ll_tt[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          " .+ @.BB",
          ". @.+ BB",
          "+@@.+ BB",
          " ...+ BB",
          "@++++ BB",
          ".     BB",
          "BBBBBBBB",
          "BBBBBBBB"
       };
       static const char * xpm_sm_ll_t[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          ".   @.BB",
          "@++ @.BB",
          " .+ @.BB",
          ". @.+ BB",
          "+@@.+ BB",
          " ...+ BB",
          "@++++ BB",
          ".     BB"
       };
       static const char * xpm_sm_ll[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          " .....BB",
          "+@@@@.BB",
          ".   @.BB",
          "@++ @.BB",
          " .+ @.BB",
          ". @.+ BB",
          "+@@.+ BB",
          " ...+ BB"
       };
       static const char * xpm_sm_ll_b[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBB",
          "BBBBBBBB",
          " .....BB",
          "+@@@@.BB",
          ".   @.BB",
          "@++ @.BB",
          " .+ @.BB",
          ". @.+ BB"
       };
       static const char * xpm_sm_l_tt[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          ".@ .+ @.",
          " +. @.+ ",
          " ++@@.+ ",
          "   ...+ ",
          "@@@++++ ",
          "...     ",
          "BBBBBBBB",
          "BBBBBBBB"
       };
       static const char * xpm_sm_l_t[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "...   @.",
          ".@@++ @.",
          ".@ .+ @.",
          " +. @.+ ",
          " ++@@.+ ",
          "   ...+ ",
          "@@@++++ ",
          "...     "
       };
       static const char * xpm_sm_l[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "   .....",
          "+++@@@@.",
          "...   @.",
          ".@@++ @.",
          ".@ .+ @.",
          " +. @.+ ",
          " ++@@.+ ",
          "   ...+ "
       };
       static const char * xpm_sm_l_b[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBB",
          "BBBBBBBB",
          "   .....",
          "+++@@@@.",
          "...   @.",
          ".@@++ @.",
          ".@ .+ @.",
          " +. @.+ "
       };
       static const char * xpm_sm_tt[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          " +.@ .+ ",
          ".@ +. @.",
          ".@ ++@@.",
          ".@   ...",
          ".@@@@+++",
          ".....   ",
          "BBBBBBBB",
          "BBBBBBBB"
       };
       static const char * xpm_sm_t[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          " +...   ",
          " +.@@++ ",
          " +.@ .+ ",
          ".@ +. @.",
          ".@ ++@@.",
          ".@   ...",
          ".@@@@+++",
          ".....   "
       };
       static const char * xpm_sm_b[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBB",
          "BBBBBBBB",
          "     ...",
          " ++++@@@",
          " +...   ",
          " +.@@++ ",
          " +.@ .+ ",
          ".@ +. @."
       };
       static const char * xpm_sm_r_tt[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB +.@ .",
          "BB.@ +. ",
          "BB.@ ++@",
          "BB.@   .",
          "BB.@@@@+",
          "BB..... ",
          "BBBBBBBB",
          "BBBBBBBB"
       };
       static const char * xpm_sm_r_t[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB +... ",
          "BB +.@@+",
          "BB +.@ .",
          "BB.@ +. ",
          "BB.@ ++@",
          "BB.@   .",
          "BB.@@@@+",
          "BB..... "
       };
       static const char * xpm_sm_r[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BB     .",
          "BB ++++@",
          "BB +... ",
          "BB +.@@+",
          "BB +.@ .",
          "BB.@ +. ",
          "BB.@ ++@",
          "BB.@   ."
       };
       static const char * xpm_sm_r_b[] = {
          "8 8 5 1", "B c Black", "  c #00ff00", ". c #0000ff", "+ c #7f7f7f", "@ c #FF0000",
          "BBBBBBBB",
          "BBBBBBBB",
          "BB     .",
          "BB ++++@",
          "BB +... ",
          "BB +.@@+",
          "BB +.@ .",
          "BB.@ +. "
       };

       // this table defines all tests
       struct SizeTestData
       {
          int w, h, dx, dy;                // first parameters for Size()
          const char **ref_xpm;            // expected result
       } sizeTestData[] =
       {
          { 10, 10,  0,  0, xpm_orig},      // same size, same position
          { 12, 12,  0,  0, xpm_gt},       // target larger, same position
          {  8,  8,  0,  0, xpm_sm},       // target smaller, same position
          { 10, 10, -2, -2, xpm_l_t},      // same size, move left up
          { 10, 10, -2,  0, xpm_l},        // same size, move left
          { 10, 10, -2,  2, xpm_l_b},      // same size, move left down
          { 10, 10,  0, -2, xpm_t},        // same size, move up
          { 10, 10,  0,  2, xpm_b},        // same size, move down
          { 10, 10,  2, -2, xpm_r_t},      // same size, move right up
          { 10, 10,  2,  0, xpm_r},        // same size, move right
          { 10, 10,  2,  2, xpm_r_b},      // same size, move right down
          { 12, 12, -2, -2, xpm_gt_l_t},   // target larger, move left up
          { 12, 12, -2,  0, xpm_gt_l},     // target larger, move left
          { 12, 12, -2,  2, xpm_gt_l_b},   // target larger, move left down
          { 12, 12, -2,  4, xpm_gt_l_bb},  // target larger, move left down
          { 12, 12,  0, -2, xpm_gt_t},     // target larger, move up
          { 12, 12,  0,  2, xpm_gt_b},     // target larger, move down
          { 12, 12,  0,  4, xpm_gt_bb},    // target larger, move down
          { 12, 12,  2, -2, xpm_gt_r_t},   // target larger, move right up
          { 12, 12,  2,  0, xpm_gt_r},     // target larger, move right
          { 12, 12,  2,  2, xpm_gt_r_b},   // target larger, move right down
          { 12, 12,  2,  4, xpm_gt_r_bb},  // target larger, move right down
          { 12, 12,  4, -2, xpm_gt_rr_t},  // target larger, move right up
          { 12, 12,  4,  0, xpm_gt_rr},    // target larger, move right
          { 12, 12,  4,  2, xpm_gt_rr_b},  // target larger, move right down
          { 12, 12,  4,  4, xpm_gt_rr_bb}, // target larger, move right down
          {  8,  8, -4, -4, xpm_sm_ll_tt}, // target smaller, move left up
          {  8,  8, -4, -2, xpm_sm_ll_t},  // target smaller, move left up
          {  8,  8, -4,  0, xpm_sm_ll},    // target smaller, move left
          {  8,  8, -4,  2, xpm_sm_ll_b},  // target smaller, move left down
          {  8,  8, -2, -4, xpm_sm_l_tt},  // target smaller, move left up
          {  8,  8, -2, -2, xpm_sm_l_t},   // target smaller, move left up
          {  8,  8, -2,  0, xpm_sm_l},     // target smaller, move left
          {  8,  8, -2,  2, xpm_sm_l_b},   // target smaller, move left down
          {  8,  8,  0, -4, xpm_sm_tt},    // target smaller, move up
          {  8,  8,  0, -2, xpm_sm_t},     // target smaller, move up
          {  8,  8,  0,  2, xpm_sm_b},     // target smaller, move down
          {  8,  8,  2, -4, xpm_sm_r_tt},  // target smaller, move right up
          {  8,  8,  2, -2, xpm_sm_r_t},   // target smaller, move right up
          {  8,  8,  2,  0, xpm_sm_r},     // target smaller, move right
          {  8,  8,  2,  2, xpm_sm_r_b},   // target smaller, move right down
       };

       const wxImage src_img(xpm_orig);
       for ( unsigned i = 0; i < WXSIZEOF(sizeTestData); i++ )
       {
           SizeTestData& st = sizeTestData[i];
           wxImage
               actual(src_img.Size(wxSize(st.w, st.h), wxPoint(st.dx, st.dy), 0, 0, 0)),
               expected(st.ref_xpm);

           // to check results with an image viewer uncomment this:
           //actual.SaveFile(wxString::Format("imagetest-%02d-actual.png", i), wxBitmapType::PNG);
           //expected.SaveFile(wxString::Format("imagetest-%02d-exp.png", i), wxBitmapType::PNG);

           INFO("Resize test #%u: (%d, %d), (%d, %d)",
                       i, st.w, st.h, st.dx, st.dy);
           // FIXME: Figure out a better way.
           //CHECK_THAT( actual, RGBSameAs(expected) );
       }
    };

    "CompareLoadedImage"_test = []
    {
        wxImage expected8("image/data/horse.xpm");
        expect( expected8.IsOk() );

        wxImage expected24("image/data/horse.png");
        expect( expected24.IsOk() );

        for (const auto& testFile : g_testfiles)
        {
            if ( !(testFile.bitDepth == 8 || testFile.bitDepth == 24)
                || testFile.type == wxBitmapType::JPEG /*skip lossy JPEG*/)
            {
                continue;
            }

            wxImage actual(testFile.file);

            if ( actual.GetSize() != expected8.GetSize() )
            {
                continue;
            }


            INFO("Compare test '%s' for loading", testFile.file);
            // FIXME: Find better way.
            //CHECK_THAT( actual,
            //            RGBSameAs(testFile.bitDepth == 8 ? expected8
            //                                                   : expected24) );
        }
    };

    "CompareSavedImage"_test = []
    {
        wxImage expected24("image/data/horse.png");
        expect(expected24.IsOk());
        expect(!expected24.HasAlpha());

        wxImage expected8 = expected24.ConvertToGreyscale();

#if wxUSE_PALETTE
        std::array<unsigned char, 256> greys;

        std::iota(greys.begin(), greys.end(), 0);

        // FIXME: Add interface for std::array in wxPalette
        wxPalette palette(256, greys.data(), greys.data(), greys.data());
        expected8.SetPalette(palette);
#endif // #if wxUSE_PALETTE

        expected8.SetOption(wxIMAGE_OPTION_BMP_FORMAT, wxBMP_8BPP_PALETTE);

        // Create an image with alpha based on the loaded image
        wxImage expected32(expected24);

        SetAlpha(&expected32);

        const wxList& list = wxImage::GetHandlers();
        for (wxList::compatibility_iterator node = list.GetFirst();
            node; node = node->GetNext())
        {
            wxImageHandler* handler = (wxImageHandler*)node->GetData();

#if wxUSE_PALETTE
            CompareImage(*handler, expected8, wxIMAGE_HAVE_PALETTE);
#endif
            CompareImage(*handler, expected24);
            CompareImage(*handler, expected32, wxIMAGE_HAVE_ALPHA);
        }
    };

    "SavePNG"_test = []
    {
        wxImage expected24("image/data/horse.png");
        expect(expected24.IsOk());
#if wxUSE_PALETTE
        expect(!expected24.HasPalette());
#endif // #if wxUSE_PALETTE

        wxImage expected8 = expected24.ConvertToGreyscale();

        /*
        horse.png converted to greyscale should be saved without a palette.
        */
        CompareImage(*wxImage::FindHandler(wxBitmapType::PNG), expected8);

        /*
        But if we explicitly ask for trying to save with a palette, it should work.
        */
        expected8.SetOption(wxIMAGE_OPTION_PNG_FORMAT, wxPNG_TYPE_PALETTE);

        CompareImage(*wxImage::FindHandler(wxBitmapType::PNG),
            expected8, wxIMAGE_HAVE_PALETTE);


        expect(expected8.LoadFile("image/data/horse.gif"));
#if wxUSE_PALETTE
        expect(expected8.HasPalette());
#endif // #if wxUSE_PALETTE

        CompareImage(*wxImage::FindHandler(wxBitmapType::PNG),
            expected8, wxIMAGE_HAVE_PALETTE);

        /*
        Add alpha to the image in such a way that there will still be a maximum
        of 256 unique RGBA combinations. This should result in a saved
        PNG image still being palettised and having alpha.
        */
        expected8.SetAlpha();

        const int width = expected8.GetWidth();
        const int height = expected8.GetHeight();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                expected8.SetAlpha(x, y, expected8.GetRed(x, y));
            }
        }

        CompareImage(*wxImage::FindHandler(wxBitmapType::PNG),
            expected8, wxIMAGE_HAVE_ALPHA | wxIMAGE_HAVE_PALETTE);

        /*
        Now change the alpha of the first pixel so that we can't save palettised
        anymore because there will be 256+1 entries which is beyond PNGs limit
        of 256 entries.
        */
        expected8.SetAlpha(0, 0, 1);

        CompareImage(*wxImage::FindHandler(wxBitmapType::PNG),
            expected8, wxIMAGE_HAVE_ALPHA);

        /*
        Even if we explicitly ask for saving palettised it should not be done.
        */
        expected8.SetOption(wxIMAGE_OPTION_PNG_FORMAT, wxPNG_TYPE_PALETTE);
        CompareImage(*wxImage::FindHandler(wxBitmapType::PNG),
            expected8, wxIMAGE_HAVE_ALPHA);
    };

#if wxUSE_ZLIB
    "LoadFromZipStream"_test = []
    {
        for (const auto& testFile : g_testfiles)
        {
            switch (testFile.type)
            {
                case wxBitmapType::XPM:
                case wxBitmapType::GIF:
                case wxBitmapType::PCX:
                case wxBitmapType::TGA:
                case wxBitmapType::TIFF:
                continue;       // skip testing those wxImageHandlers which cannot
                                // load data from non-seekable streams

                default:
                    ; // proceed
            }

            // compress the test file on the fly:
            wxMemoryOutputStream memOut;
            {
                wxFileInputStream file(testFile.file);
                expect(file.IsOk());

                wxZlibOutputStream compressFilter(memOut, 5, wxZLIB_GZIP);
                expect(compressFilter.IsOk());

                file.Read(compressFilter);
                expect(file.GetLastError() == wxSTREAM_EOF);
            }

            // now fetch the compressed memory to wxImage, decompressing it on the fly; this
            // allows us to test loading images from non-seekable streams other than socket streams
            wxMemoryInputStream memIn(memOut);
            expect(memIn.IsOk());
            wxZlibInputStream decompressFilter(memIn, wxZLIB_GZIP);
            expect(decompressFilter.IsOk());

            wxImage img;

            // NOTE: it's important to inform wxImage about the type of the image being
            //       loaded otherwise it will try to autodetect the format, but that
            //       requires a seekable stream!
            expect(img.LoadFile(decompressFilter, testFile.type)) <<
                   fmt::format("Could not load file type '%d' after it was zipped", testFile.type);
        }
    };
#endif

#if wxUSE_LIBTIFF
    // FIXME: Can't reach string_view constants in WX.Image.TIFF module
    "SaveTIFF"_test = []
    {
        TestTIFFImage("BitsPerSample", 1);
        TestTIFFImage("SamplesPerPixel", 1);
        TestTIFFImage("Photometric", 0/*PHOTOMETRIC_MINISWHITE*/);
        TestTIFFImage("Photometric", 1/*PHOTOMETRIC_MINISBLACK*/);

        wxImage alphaImage("image/data/horse.png");
        expect(alphaImage.IsOk());
        SetAlpha(&alphaImage);

        // RGB with alpha
        TestTIFFImage("SamplesPerPixel", 4, &alphaImage);

        // Grey with alpha
        TestTIFFImage("SamplesPerPixel", 2, &alphaImage);

        // B/W with alpha
        alphaImage.SetOption("BitsPerSample", 1);
        TestTIFFImage("SamplesPerPixel", 2, &alphaImage);
    };
#endif // wxUSE_LIBTIFF

    "ReadCorruptedTGA"_test = []
    {
        static std::array<unsigned char, 18 + 1 + 3> corruptTGA =
        {
            0,
            0,
            10, // RLE compressed image.
            0, 0,
            0, 0,
            0,
            0, 0,
            0, 0,
            1, 0, // Width is 1.
            1, 0, // Height is 1.
            24, // Bits per pixel.
            0,

            0xff, // Run length (repeat next pixel 127+1 times).
            0xff, 0xff, 0xff // One 24-bit pixel.
        };

        // FIXME: Add std::array (or span) interface to wxMemoryInputStream
        wxMemoryInputStream memIn(corruptTGA.data(), WXSIZEOF(corruptTGA));
        expect(memIn.IsOk());

        wxImage tgaImage;
        expect(!tgaImage.LoadFile(memIn));


        /*
        Instead of repeating a pixel 127+1 times, now tell it there will
        follow 127+1 uncompressed pixels (while we only should have 1 in total).
        */
        corruptTGA[18] = 0x7f;
        expect(!tgaImage.LoadFile(memIn));
    };

#if wxUSE_GIF

    "SaveAnimatedGIF"_test = []
    {
#if wxUSE_PALETTE
        wxImage image("image/data/horse.gif");
        expect(image.IsOk());

        wxImageArray images;
        images.push_back(image);
        for (int i = 0; i < 4 - 1; ++i)
        {
            images.push_back(images[i].Rotate90());

            images[i + 1].SetPalette(images[0].GetPalette());
        }

        wxMemoryOutputStream memOut;
        expect(wxGIFHandler().SaveAnimation(images, &memOut));

        wxGIFHandler handler;
        wxMemoryInputStream memIn(memOut);
        expect(memIn.IsOk());
        const int imageCount = handler.GetImageCount(memIn);
        expect(imageCount == 4);

        for (int i = 0; i < imageCount; ++i)
        {
            wxFileOffset pos = memIn.TellI();
            expect(handler.LoadFile(&image, memIn, true, i)) <<
                fmt::format("Compare test for GIF frame number %d failed", i);
            memIn.SeekI(pos);

            // FIXME: Find better way.
            //CHECK_THAT(image, RGBSameAs(images[i]));
        }
#endif // #if wxUSE_PALETTE
    };

    "GIFComment"_test = []
    {
        // Test reading a comment.
        wxImage image("image/data/horse.gif");
        expect("  Imported from GRADATION image: gray" ==
            image.GetOption(wxIMAGE_OPTION_GIF_COMMENT));


        // Test writing a comment and reading it back.
        TestGIFComment("Giving the GIF a gifted giraffe as a gift");


        // Test writing and reading a comment again but with a long comment.
        static constexpr std::string_view longStr("c", 768);
        TestGIFComment(longStr);

        // Test writing comments in an animated GIF and reading them back.
        expect(image.LoadFile("image/data/horse.gif"));

#if wxUSE_PALETTE
        wxImageArray images;

        for (int i = 0; i < 4; ++i)
        {
            if (i)
            {
                images.push_back(images[i - 1].Rotate90());
                images[i].SetPalette(images[0].GetPalette());
            }
            else
            {
                images.push_back(image);
            }

            images[i].SetOption(wxIMAGE_OPTION_GIF_COMMENT,
                fmt::format("GIF comment for frame #%d", i + 1));

        }

        wxMemoryOutputStream memOut;
        expect(wxGIFHandler().SaveAnimation(images, &memOut));

        wxGIFHandler handler;
        wxMemoryInputStream memIn(memOut);
        expect(memIn.IsOk());
        const int imageCount = handler.GetImageCount(memIn);

        for (int i = 0; i < imageCount; ++i)
        {
            wxFileOffset pos = memIn.TellI();
            expect(handler.LoadFile(&image, memIn, true /*verbose?*/, i));

            expect(fmt::format("GIF comment for frame #%d", i + 1) ==
                image.GetOption(wxIMAGE_OPTION_GIF_COMMENT));

            memIn.SeekI(pos);
        }
#endif //wxUSE_PALETTE
    };
#endif // wxUSE_GIF

    "DibPadding"_test = []
    {
        /*
        There used to be an error with calculating the DWORD aligned scan line
        pitch for a BMP/ICO resulting in buffer overwrites (with at least MSVC9
        Debug this gave a heap corruption assertion when saving the mask of
        an ICO). Test for it here.
        */
        wxImage image("image/data/horse.gif");
        expect(image.IsOk());

        image = image.Scale(99, 99);

        wxMemoryOutputStream memOut;
        expect(image.SaveFile(memOut, wxBitmapType::ICO));
    };

    "BMPFlippingAndRLECompression"_test = []
    {
        CompareBMPImage("image/data/horse_grey.bmp", "image/data/horse_grey_flipped.bmp");

        CompareBMPImage("image/data/horse_rle8.bmp", "image/data/horse_grey.bmp");
        CompareBMPImage("image/data/horse_rle8.bmp", "image/data/horse_rle8_flipped.bmp");

        CompareBMPImage("image/data/horse_rle4.bmp", "image/data/horse_rle4_flipped.bmp");
    };

//     SUBCASE("ScaleCompare")
//     {
//         wxImage original;
//         CHECK(original.LoadFile("image/data/horse.bmp"));

//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(50, 50, wxImageResizeQuality::Bicubic),
//             "image/data/horse_bicubic_50x50.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(100, 100, wxImageResizeQuality::Bicubic),
//             "image/data/horse_bicubic_100x100.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(150, 150, wxImageResizeQuality::Bicubic),
//             "image/data/horse_bicubic_150x150.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(300, 300, wxImageResizeQuality::Bicubic),
//             "image/data/horse_bicubic_300x300.png");

//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(50, 50, wxImageResizeQuality::BoxAverage),
//             "image/data/horse_box_average_50x50.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(100, 100, wxImageResizeQuality::BoxAverage),
//             "image/data/horse_box_average_100x100.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(150, 150, wxImageResizeQuality::BoxAverage),
//             "image/data/horse_box_average_150x150.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(300, 300, wxImageResizeQuality::BoxAverage),
//             "image/data/horse_box_average_300x300.png");

//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(50, 50, wxImageResizeQuality::Bilinear),
//             "image/data/horse_bilinear_50x50.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(100, 100, wxImageResizeQuality::Bilinear),
//             "image/data/horse_bilinear_100x100.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(150, 150, wxImageResizeQuality::Bilinear),
//             "image/data/horse_bilinear_150x150.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(original.Scale(300, 300, wxImageResizeQuality::Bilinear),
//             "image/data/horse_bilinear_300x300.png");

//         // Test scaling symmetric image
//         const static char* cross_xpm[] =
//         {
//             "9 9 5 1",
//             "   c None",
//             "r  c #FF0000",
//             "g  c #00FF00",
//             "b  c #0000FF",
//             "w  c #FFFFFF",
//             "    r    ",
//             "    g    ",
//             "    b    ",
//             "    w    ",
//             "rgbw wbgr",
//             "    w    ",
//             "    b    ",
//             "    g    ",
//             "    r    "
//         };

//         wxImage imgCross(cross_xpm);
//         ASSERT_IMAGE_EQUAL_TO_FILE(imgCross.Scale(256, 256, wxImageResizeQuality::Bilinear),
//             "image/data/cross_bilinear_256x256.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(imgCross.Scale(256, 256, wxImageResizeQuality::Bicubic),
//             "image/data/cross_bicubic_256x256.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(imgCross.Scale(256, 256, wxImageResizeQuality::BoxAverage),
//             "image/data/cross_box_average_256x256.png");
//         ASSERT_IMAGE_EQUAL_TO_FILE(imgCross.Scale(256, 256, wxImageResizeQuality::Nearest),
//             "image/data/cross_nearest_neighb_256x256.png");
//     }

    "CreateBitmapFromCursor"_test = []
    {
#if !defined __WXOSX_IPHONE__ && !defined __WXDFB__ && !defined __WXMOTIF__ && !defined __WXX11__

        wxImage image("image/data/wx.png");
        wxCursor cursor(image);
        wxBitmap bitmap(cursor);

#if defined(__WXGTK__)
        // cursor to bitmap could fail depending on windowing system and cursor (gdk-cursor-get-image)
        if (!bitmap.IsOk())
            return;
#endif

        wxImage result = bitmap.ConvertToImage();

        // on Windows the cursor is always scaled to 32x32px (96 DPI)
        // on macOS the resulting bitmap size depends on the DPI
        if (image.GetSize() == result.GetSize())
        {
            // FIXME: Figure out better way.
            //CHECK_THAT(image, RGBASimilarTo(result, 2));
        }
        else
        {
            std::vector<wxPoint> coords = {
                wxPoint(14, 10), // blue square
                wxPoint(8, 22),  // red square
                wxPoint(26, 18), // yellow square
                wxPoint(25, 5)   // empty / tranparent
            };

            for (const auto p1 : coords)
            {
                wxPoint p2 = wxPoint(p1.x * (result.GetWidth() / (double)image.GetWidth()), p1.y * (result.GetHeight() / (double)image.GetHeight()));

#if defined(__WXMSW__)
                // when the cursor / result image is larger than the source image, the original image is centered in the result image
                if (result.GetWidth() > image.GetWidth())
                    p2.x = (result.GetWidth() / 2) + (p1.x - (image.GetWidth() / 2));
                if (result.GetHeight() > image.GetHeight())
                    p2.y = (result.GetHeight() / 2) + (p1.y - (image.GetHeight() / 2));
#endif

                wxColour cSrc(image.GetRed(p1.x, p1.y), image.GetGreen(p1.x, p1.y), image.GetBlue(p1.x, p1.y), image.GetAlpha(p1.x, p1.y));
                wxColour cRes(result.GetRed(p2.x, p2.y), result.GetGreen(p2.x, p2.y), result.GetBlue(p2.x, p2.y), result.GetAlpha(p2.x, p2.y));

                CHECK_EQUAL_COLOUR_RGBA(cRes, cSrc);
            }
        }
#endif
    };
};

ut::suite wxImagePasteTest = []
{
    using namespace ut;
    
    const static char* squares_xpm[] =
    {
        "9 9 7 1",
        "   c None",
        "y  c #FFFF00",
        "r  c #FF0000",
        "g  c #00FF00",
        "b  c #0000FF",
        "o  c #FF6600",
        "w  c #FFFFFF",
        "rrrrwgggg",
        "rrrrwgggg",
        "rrrrwgggg",
        "rrrrwgggg",
        "wwwwwwwww",
        "bbbbwoooo",
        "bbbbwoooo",
        "bbbbwoooo",
        "bbbbwoooo"
    };

    const static char* toggle_equal_size_xpm[] =
    {
        "9 9 2 1",
        "   c None",
        "y  c #FFFF00",
        "y y y y y",
        " y y y y ",
        "y y y y y",
        " y y y y ",
        "y y y y y",
        " y y y y ",
        "y y y y y",
        " y y y y ",
        "y y y y y",
    };

    const static char* transparent_image_xpm[] =
    {
        "5 5 2 1",
        "   c None", // Mask
        "y  c #FFFF00",
        "     ",
        "     ",
        "     ",
        "     ",
        "     ",
    };

    const static char* light_image_xpm[] =
    {
        "5 5 2 1",
        "   c None",
        "y  c #FFFF00",
        "yyyyy",
        "yyyyy",
        "yyyyy",
        "yyyyy",
        "yyyyy",
    };

    const static char* black_image_xpm[] =
    {
        "5 5 2 1",
        "   c #000000",
        "y  c None", // Mask
        "     ",
        "     ",
        "     ",
        "     ",
        "     ",
    };

    // Execute AddHandler() just once.
    static const bool
        registeredHandler = (wxImage::AddHandler(new wxPNGHandler()), true);

    "Paste same size image"_test = []
    {
        wxImage actual(squares_xpm);
        wxImage paste(toggle_equal_size_xpm);
        wxImage expected(toggle_equal_size_xpm);
        actual.Paste(paste, 0, 0);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));

        // Without alpha using "compose" doesn't change anything.
        actual.Paste(paste, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste larger image"_test = []
    {
        const static char* toggle_larger_size_xpm[] =
        {
            "13 13 2 1",
            "   c None",
            "y  c #FFFF00",
            "y y y y y y y",
            " y y y y y y ",
            "y y y y y y y",
            " y y y y y y ",
            "y y y y y y y",
            " y y y y y y ",
            "y y y y y y y",
            " y y y y y y ",
            "y y y y y y y",
            " y y y y y y ",
            "y y y y y y y",
            " y y y y y y ",
            "y y y y y y y",
        };

        wxImage actual(squares_xpm);
        wxImage paste(toggle_larger_size_xpm);
        wxImage expected(toggle_equal_size_xpm);
        actual.Paste(paste, -2, -2);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste smaller image"_test = []
    {
        const static char* toggle_smaller_size_xpm[] =
        {
            "5 5 2 1",
            "   c None",
            "y  c #FFFF00",
            "y y y",
            " y y ",
            "y y y",
            " y y ",
            "y y y",
        };

        const static char* expected_xpm[] =
        {
            "9 9 7 1",
            "   c None",
            "y  c #FFFF00",
            "r  c #FF0000",
            "g  c #00FF00",
            "b  c #0000FF",
            "o  c #FF6600",
            "w  c #FFFFFF",
            "rrrrwgggg",
            "rrrrwgggg",
            "rry y ygg",
            "rr y y gg",
            "wwy y yww",
            "bb y y oo",
            "bby y yoo",
            "bbbbwoooo",
            "bbbbwoooo"
        };

        wxImage actual(squares_xpm);
        wxImage paste(toggle_smaller_size_xpm);
        wxImage expected(expected_xpm);
        actual.Paste(paste, 2, 2);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste beyond top left corner"_test = []
    {
        const static char* expected_xpm[] =
        {
            "9 9 7 1",
            "   c None",
            "y  c #FFFF00",
            "r  c #FF0000",
            "g  c #00FF00",
            "b  c #0000FF",
            "o  c #FF6600",
            "w  c #FFFFFF",
            "oooowgggg",
            "oooowgggg",
            "oooowgggg",
            "oooowgggg",
            "wwwwwwwww",
            "bbbbwoooo",
            "bbbbwoooo",
            "bbbbwoooo",
            "bbbbwoooo"
        };

        wxImage actual(squares_xpm);
        wxImage paste(squares_xpm);
        wxImage expected(expected_xpm);
        actual.Paste(paste, -5, -5);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste beyond top right corner"_test = []
    {
        const static char* expected_xpm[] =
        {
            "9 9 7 1",
            "   c None",
            "y  c #FFFF00",
            "r  c #FF0000",
            "g  c #00FF00",
            "b  c #0000FF",
            "o  c #FF6600",
            "w  c #FFFFFF",
            "rrrrwbbbb",
            "rrrrwbbbb",
            "rrrrwbbbb",
            "rrrrwbbbb",
            "wwwwwwwww",
            "bbbbwoooo",
            "bbbbwoooo",
            "bbbbwoooo",
            "bbbbwoooo"
        };
        wxImage actual(squares_xpm);
        wxImage paste(squares_xpm);
        wxImage expected(expected_xpm);
        actual.Paste(paste, 5, -5);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste beyond bottom right corner"_test = []
    {
        const static char* expected_xpm[] =
        {
            "9 9 7 1",
            "   c None",
            "y  c #FF0000",
            "r  c #FF0000",
            "g  c #00FF00",
            "b  c #0000FF",
            "o  c #FF6600",
            "w  c #FFFFFF",
            "rrrrwgggg",
            "rrrrwgggg",
            "rrrrwgggg",
            "rrrrwgggg",
            "wwwwwwwww",
            "bbbbwrrrr",
            "bbbbwrrrr",
            "bbbbwrrrr",
            "bbbbwrrrr"
        };
        wxImage actual(squares_xpm);
        wxImage paste(squares_xpm);
        wxImage expected(expected_xpm);
        actual.Paste(paste, 5, 5);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste beyond bottom left corner"_test = []
    {
        const static char* expected_xpm[] =
        {
            "9 9 7 1",
            "   c None",
            "y  c #FFFF00",
            "r  c #FF0000",
            "g  c #00FF00",
            "b  c #0000FF",
            "o  c #FF6600",
            "w  c #FFFFFF",
            "rrrrwgggg",
            "rrrrwgggg",
            "rrrrwgggg",
            "rrrrwgggg",
            "wwwwwwwww",
            "ggggwoooo",
            "ggggwoooo",
            "ggggwoooo",
            "ggggwoooo"
        };
        wxImage actual(squares_xpm);
        wxImage paste(squares_xpm);
        wxImage expected(expected_xpm);
        actual.Paste(paste, -5, 5);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste fully opaque image onto blank image without alpha"_test = []
    {
        const wxImage background("image/data/paste_input_background.png");
        expect(background.IsOk());

        wxImage actual(background.GetSize());
        actual.Paste(background, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(background));
        expect(!actual.HasAlpha());
    };

    "Paste fully opaque image onto blank image with alpha"_test = []
    {
        const wxImage background("image/data/paste_input_background.png");
        expect(background.IsOk());

        wxImage actual(background.GetSize());
        actual.InitAlpha();
        actual.Paste(background, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(background));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(wxALPHA_OPAQUE));
    };

    "Paste fully transparent image"_test = []
    {
        const wxImage background("image/data/paste_input_background.png");
        expect(background.IsOk());

        wxImage actual = background.Copy();
        wxImage transparent(actual.GetSize());
        transparent.InitAlpha();
        memset(transparent.GetAlpha(), 0, transparent.GetWidth() * transparent.GetHeight());
        actual.Paste(transparent, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(background));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(wxALPHA_OPAQUE));
    };

    "Paste image with transparent region"_test = []
    {
        wxImage actual("image/data/paste_input_background.png");
        expect(actual.IsOk());

        const wxImage opaque_square("image/data/paste_input_overlay_transparent_border_opaque_square.png");
        expect(opaque_square.IsOk());

        actual.Paste(opaque_square, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(wxImage("image/data/paste_result_background_plus_overlay_transparent_border_opaque_square.png")));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(wxALPHA_OPAQUE));
    };

    "Paste image with semi transparent region"_test = []
    {
        wxImage actual("image/data/paste_input_background.png");
        expect(actual.IsOk());

        const wxImage transparent_square("image/data/paste_input_overlay_transparent_border_semitransparent_square.png");
        expect(transparent_square.IsOk());

        actual.Paste(transparent_square, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(wxImage("image/data/paste_result_background_plus_overlay_transparent_border_semitransparent_square.png")));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(wxALPHA_OPAQUE));
    };

    "Paste two semi transparent images on top of background"_test = []
    {
        wxImage actual("image/data/paste_input_background.png");
        expect(actual.IsOk());

        const wxImage transparent_square("image/data/paste_input_overlay_transparent_border_semitransparent_square.png");
        expect(transparent_square.IsOk());

        const wxImage transparent_circle("image/data/paste_input_overlay_transparent_border_semitransparent_circle.png");
        expect(transparent_circle.IsOk());

        actual.Paste(transparent_circle, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        actual.Paste(transparent_square, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSimilarTo(wxImage("image/data/paste_result_background_plus_circle_plus_square.png"), 1));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(wxALPHA_OPAQUE));
    };

    "Paste two semi transparent images together first, then on top of background"_test = []
    {
        wxImage actual("image/data/paste_input_background.png");
        expect(actual.IsOk());

        const wxImage transparent_square("image/data/paste_input_overlay_transparent_border_semitransparent_square.png");
        expect(transparent_square.IsOk());

        const wxImage transparent_circle("image/data/paste_input_overlay_transparent_border_semitransparent_circle.png");
        expect(transparent_circle.IsOk());

        wxImage circle = transparent_circle.Copy();
        circle.Paste(transparent_square, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        actual.Paste(circle, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // When applied in this order, two times a rounding difference is triggered.
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSimilarTo(wxImage("image/data/paste_result_background_plus_circle_plus_square.png"), 2));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(wxALPHA_OPAQUE));
    };

    "Paste semitransparent image over transparent image"_test = []
    {
        const wxImage transparent_square("image/data/paste_input_overlay_transparent_border_semitransparent_square.png");
        expect(transparent_square.IsOk());

        const wxImage transparent_circle("image/data/paste_input_overlay_transparent_border_semitransparent_circle.png");
        expect(transparent_circle.IsOk());

        wxImage actual(transparent_circle.GetSize());
        actual.InitAlpha();
        memset(actual.GetAlpha(), 0, actual.GetWidth() * actual.GetHeight());
        actual.Paste(transparent_circle, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, CenterAlphaPixelEquals(192));
        actual.Paste(transparent_square, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSimilarTo(wxImage("image/data/paste_result_no_background_square_over_circle.png"), 1));
        //CHECK_THAT(actual, CenterAlphaPixelEquals(224));
    };

    // todo make test case for 'blend with mask'
    "Paste fully transparent (masked) image over light image"_test = []
    {
        wxImage actual(light_image_xpm);
        actual.InitAlpha();
        wxImage paste(transparent_image_xpm);
        wxImage expected(light_image_xpm);
        actual.Paste(paste, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    // todo make test case for 'blend with mask'
    "Paste fully black (masked) image over light image"_test = []
    {
        wxImage actual(light_image_xpm);
        actual.InitAlpha();
        wxImage paste(black_image_xpm);
        wxImage expected(black_image_xpm);
        actual.Paste(paste, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, RGBSameAs(expected));
    };

    "Paste dark image over light image"_test = []
    {
        wxImage black("image/data/paste_input_black.png");
        wxImage actual("image/data/paste_input_background.png");
        actual.InitAlpha();
        actual.Paste(black, 0, 0, wxIMAGE_ALPHA_BLEND_COMPOSE);
        // FIXME: Figure out better way.
        //CHECK_THAT(actual, CenterAlphaPixelEquals(255));
        //CHECK_THAT(actual, RGBSameAs(black));
    };

    "Paste large image with negative vertical offset"_test = []
    {
        wxImage target(442, 249);
        wxImage to_be_pasted(345, 24900);
        target.InitAlpha();
        target.Paste(to_be_pasted, 48, -12325, wxIMAGE_ALPHA_BLEND_COMPOSE);
    };
    
    "Paste large image with negative horizontal offset"_test = []
    {
        wxImage target(249, 442);
        wxImage to_be_pasted(24900, 345);
        target.InitAlpha();
        target.Paste(to_be_pasted, -12325, 48, wxIMAGE_ALPHA_BLEND_COMPOSE);
    };
};

ut::suite ImageRGBtoHSVTests = []
{
    using namespace ut;

    "RGB(0,0,0) (Black) to HSV"_test = []
    {
       RGBValue rgbBlack(0, 0, 0);
       wxImage::HSVValue hsvBlack = wxImage::RGBtoHSV(rgbBlack);

       expect(hsvBlack.value == doctest::Approx(0.0));
       // saturation and hue are undefined
    };
    
    "RGB(255,255,255) (White) to HSV"_test = []
    {
       RGBValue rgbWhite(255, 255, 255);
       wxImage::HSVValue hsvWhite = wxImage::RGBtoHSV(rgbWhite);

       expect(hsvWhite.saturation == doctest::Approx(0.0));
       expect(hsvWhite.value == doctest::Approx(1.0));
       // hue is undefined
    };

    "RGB(0,255,0) (Green) to HSV"_test = []
    {
       RGBValue rgbGreen(0, 255, 0);
       wxImage::HSVValue hsvGreen = wxImage::RGBtoHSV(rgbGreen);

       expect(hsvGreen.hue == doctest::Approx(1.0/3.0));
       expect(hsvGreen.saturation == doctest::Approx(1.0));
       expect(hsvGreen.value == doctest::Approx(1.0));
    };

    "RGB to HSV to RGB"_test = []
    {
        static constexpr std::array<RGBValue, 10> rgbValues =
        {{
            {   0,   0,   0 },
            {  10,  10,  10 },
            { 255, 255, 255 },
            { 255,   0,   0 },
            {   0, 255,   0 },
            {   0,   0, 255 },
            {   1,   2,   3 },
            {  10,  20,  30 },
            {   0,   1,   6 },
            {   9,   0,  99 }
        }};

        for (const auto& rgbValue : rgbValues)
        {
            wxImage::HSVValue hsvValue = wxImage::RGBtoHSV(rgbValue);
            RGBValue rgbRoundtrip = wxImage::HSVtoRGB(hsvValue);

            expect(rgbRoundtrip.red == rgbValue.red);
            expect(rgbRoundtrip.green == rgbValue.green);
            expect(rgbRoundtrip.blue == rgbValue.blue);
        }
    };
};

ut::suite ImageClipboardTests = []
{
    using namespace ut;

#if wxUSE_CLIPBOARD && wxUSE_DATAOBJ
    wxInitAllImageHandlers();

    wxImage imgOriginal;
    expect(imgOriginal.LoadFile("image/data/horse.png") == true);

    wxImageDataObject* dobj1 = new wxImageDataObject(imgOriginal);
    {
        wxClipboardLocker lockClip;
        expect(wxTheClipboard->SetData(dobj1) == true);
    }

    wxYield();

    wxImageDataObject dobj2;
    {
        wxClipboardLocker lockClip;
        expect(wxTheClipboard->GetData(dobj2) == true);
    }
    wxImage imgRetrieved = dobj2.GetImage();
    expect(imgRetrieved.IsOk());

    // FIXME: Figure out better way to do this.
    //CHECK_THAT(imgOriginal, RGBASameAs(imgRetrieved));
#endif // wxUSE_CLIPBOARD && wxUSE_DATAOBJ
};

ut::suite ImageAlphaTests = []
{
    using namespace ut;

    const wxColour maskCol(*wxRED);
    const wxColour fillCol(*wxGREEN);

    "RGB image without mask"_test = [=]
    {
        wxImage img(2, 2);
        img.SetRGB(0, 0, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(0, 1, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(1, 0, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetRGB(1, 1, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        expect(!img.HasAlpha());
        expect(!img.HasMask());

        wxImage imgRes = img;
        imgRes.InitAlpha();
        expect(imgRes.HasAlpha() == true);
        expect(!imgRes.HasMask());

        for (int y = 0; y < img.GetHeight(); y++)
            for (int x = 0; x < img.GetWidth(); x++)
            {
                wxColour cSrc(img.GetRed(x, y), img.GetGreen(x, y), img.GetBlue(x, y));
                wxColour cRes(imgRes.GetRed(x, y), imgRes.GetGreen(x, y), imgRes.GetBlue(x, y), imgRes.GetAlpha(x, y));

                CHECK_EQUAL_COLOUR_RGB(cRes, cSrc);
                expect((int)cRes.Alpha() == (int)wxIMAGE_ALPHA_OPAQUE);
            }
    };

    "RGB image with mask"_test = [=]
    {
        wxImage img(2, 2);
        img.SetRGB(0, 0, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(0, 1, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(1, 0, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetRGB(1, 1, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetMaskColour(maskCol.Red(), maskCol.Green(), maskCol.Blue());
        expect(!img.HasAlpha());
        expect(img.HasMask() == true);

        wxImage imgRes = img;
        imgRes.InitAlpha();
        expect(imgRes.HasAlpha() == true);
        expect(!imgRes.HasMask());

        for ( int y = 0; y < img.GetHeight(); y++ )
            for ( int x = 0; x < img.GetWidth(); x++ )
            {
                wxColour cSrc(img.GetRed(x, y), img.GetGreen(x, y), img.GetBlue(x, y));
                wxColour cRes(imgRes.GetRed(x, y), imgRes.GetGreen(x, y), imgRes.GetBlue(x, y), imgRes.GetAlpha(x, y));

                CHECK_EQUAL_COLOUR_RGB(cRes, cSrc);
                if ( cSrc == maskCol )
                {
                    expect((int)cRes.Alpha() == (int)wxIMAGE_ALPHA_TRANSPARENT);
                }
                else
                {
                    expect((int)cRes.Alpha() == (int)wxIMAGE_ALPHA_OPAQUE);
                }
            }
    };

    "RGBA image without mask"_test = [=]
    {
        wxImage img(2, 2);
        img.SetRGB(0, 0, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(0, 1, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(1, 0, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetRGB(1, 1, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetAlpha();
        img.SetAlpha(0, 0, 128);
        img.SetAlpha(0, 1, 0);
        img.SetAlpha(1, 0, 128);
        img.SetAlpha(1, 1, 0);
        expect(img.HasAlpha() == true);
        expect(!img.HasMask());

        wxImage imgRes = img;
        expect(throws([&imgRes] { imgRes.InitAlpha(); } ));
        expect(imgRes.HasAlpha() == true);
        expect(!imgRes.HasMask());

        for ( int y = 0; y < img.GetHeight(); y++ )
            for ( int x = 0; x < img.GetWidth(); x++ )
            {
                wxColour cSrc(img.GetRed(x, y), img.GetGreen(x, y), img.GetBlue(x, y), img.GetAlpha(x, y));
                wxColour cRes(imgRes.GetRed(x, y), imgRes.GetGreen(x, y), imgRes.GetBlue(x, y), imgRes.GetAlpha(x, y));

                CHECK_EQUAL_COLOUR_RGBA(cRes, cSrc);
            }
    };

    "RGBA image with mask"_test = [=]
    {
        wxImage img(2, 2);
        img.SetRGB(0, 0, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(0, 1, maskCol.Red(), maskCol.Green(), maskCol.Blue());
        img.SetRGB(1, 0, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetRGB(1, 1, fillCol.Red(), fillCol.Green(), fillCol.Blue());
        img.SetAlpha();
        img.SetAlpha(0, 0, 128);
        img.SetAlpha(0, 1, 0);
        img.SetAlpha(1, 0, 128);
        img.SetAlpha(1, 1, 0);
        img.SetMaskColour(maskCol.Red(), maskCol.Green(), maskCol.Blue());
        expect(img.HasAlpha() == true);
        expect(img.HasMask() == true);

        wxImage imgRes = img;
        expect(throws([&imgRes] { imgRes.InitAlpha(); }));
        expect(imgRes.HasAlpha() == true);
        expect(imgRes.HasMask() == true);

        for ( int y = 0; y < img.GetHeight(); y++ )
            for ( int x = 0; x < img.GetWidth(); x++ )
            {
                wxColour cSrc(img.GetRed(x, y), img.GetGreen(x, y), img.GetBlue(x, y), img.GetAlpha(x, y));
                wxColour cRes(imgRes.GetRed(x, y), imgRes.GetGreen(x, y), imgRes.GetBlue(x, y), imgRes.GetAlpha(x, y));

                CHECK_EQUAL_COLOUR_RGBA(cRes, cSrc);
            }
    };
};

ut::suite ImageXPMTests = []
{
    using namespace ut;

   static const char * dummy_xpm[] = {
      "16 16 2 1",
      "@ c Black",
      "  c None",
      "@               ",
      " @              ",
      "  @             ",
      "   @            ",
      "    @           ",
      "     @          ",
      "      @         ",
      "       @        ",
      "        @       ",
      "         @      ",
      "          @     ",
      "           @    ",
      "            @   ",
      "             @  ",
      "              @ ",
      "               @"
   };

   wxImage image(dummy_xpm);
   expect( image.IsOk() );

   // The goal here is mostly just to check that this code compiles, i.e. that
   // creating all these classes from XPM works.
   expect( wxBitmap(dummy_xpm).IsOk() );
   expect( wxCursor(dummy_xpm).IsOk() );
   expect( wxIcon(dummy_xpm).IsOk() );
};

ut::suite ImageColorChangeTests = []
{
    using namespace ut;

    wxImage original;
    expect(original.LoadFile("image/data/toucan.png", wxBitmapType::PNG));

    wxImage test;
    wxImage expected;

    test = original;
    test.RotateHue(0.538);
    expect(expected.LoadFile("image/data/toucan_hue_0.538.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test.ChangeSaturation(-0.41);
    expect(expected.LoadFile("image/data/toucan_sat_-0.41.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test.ChangeBrightness(-0.259);
    expect(expected.LoadFile("image/data/toucan_bright_-0.259.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test.ChangeHSV(0.538, -0.41, -0.259);
    expect(expected.LoadFile("image/data/toucan_hsv_0.538_-0.41_-0.259.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test = test.ChangeLightness(46);
    expect(expected.LoadFile("image/data/toucan_light_46.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test = test.ConvertToDisabled(240);
    expect(expected.LoadFile("image/data/toucan_dis_240.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test = test.ConvertToGreyscale();
    expect(expected.LoadFile("image/data/toucan_grey.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));

    test = original;
    test = test.ConvertToMono(255, 255, 255);
    expect(expected.LoadFile("image/data/toucan_mono_255_255_255.png", wxBitmapType::PNG));
    // FIXME: Figure out better way.
    //CHECK_THAT(test, RGBSameAs(expected));
};

/*
    TODO: add lots of more tests to wxImage functions
*/

#endif //wxUSE_IMAGE
