///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/measuring.cpp
// Purpose:     Tests for wxGraphicsRenderer::CreateMeasuringContext
// Author:      Kevin Ollivier, Vadim Zeitlin (non wxGC parts)
// Created:     2008-02-12
// Copyright:   (c) 2008 Kevin Ollivier <kevino@theolliviers.com>
//              (c) 2012 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/font.h"
#include "wx/window.h"

// wxCairoRenderer::CreateMeasuringContext() is not implement for wxX11
#if wxUSE_GRAPHICS_CONTEXT && !defined(__WXX11__)
    #include "wx/graphics.h"
    #define TEST_GC
#endif

#include "wx/dcclient.h"
#include "wx/dcps.h"
#include "wx/metafile.h"

#include "asserthelper.h"

export module WX.Test.Measuring;

import WX.MetaTest;
import WX.Test.Prec;

// ----------------------------------------------------------------------------
// helper for XXXTextExtent() methods
// ----------------------------------------------------------------------------

namespace ut = boost::ut;

template <typename T>
struct GetTextExtentTester
{
    // Constructor runs a couple of simple tests for GetTextExtent().
    GetTextExtentTester(const T& obj)
    {
        using namespace ut;

        // Test that getting the height only doesn't crash.
        auto y = obj.GetTextExtent("H").y;

        expect( y > 1 );

        auto size = obj.GetTextExtent("Hello");
        expect( size.x > 1 );
        expect( size.y == y );

        // Test that getting text extent of an empty string returns (0, 0).
        expect( obj.GetTextExtent("") == wxSize(0, 0) );
    }
};

ut::suite WindowGetTextExtentTest = []
{
    using namespace ut;

    wxClientDC dc = wxTheApp->GetTopWindow();

    GetTextExtentTester<wxClientDC> testDC(dc);

    int w;
    dc.GetMultiLineTextExtent("Good\nbye", &w, nullptr);
    const wxSize sz = dc.GetTextExtent("Good");
    expect( w == sz.x );

    expect( dc.GetMultiLineTextExtent("Good\nbye").y >= 2*sz.y );

    // Check that empty lines get counted
    expect( dc.GetMultiLineTextExtent("\n\n\n").y >= 3*sz.y );

    // And even empty strings count like one line.
    expect( dc.GetMultiLineTextExtent("") == wxSize(0, sz.y) );

    // Test the functions with some other DC kinds also.
#if wxUSE_PRINTING_ARCHITECTURE && wxUSE_POSTSCRIPT
    wxPostScriptDC psdc;
    // wxPostScriptDC doesn't have any font set by default but its
    // GetTextExtent() requires one to be set. This is probably a bug and we
    // should set the default font in it implicitly but for now just work
    // around it.
    psdc.SetFont(*wxNORMAL_FONT);
    GetTextExtentTester<wxPostScriptDC> testPS(psdc);
#endif

#if wxUSE_ENH_METAFILE
    wxEnhMetaFileDC metadc;
    GetTextExtentTester<wxEnhMetaFileDC> testMF(metadc);
#endif
};

ut::suite DCLeadingAndDescentTest = []
{
    using namespace ut;

    wxClientDC dc = wxTheApp->GetTopWindow();

    // Retrieving just the descent should work.
    int descent = -17;
    dc.GetTextExtent("foo", &descent, nullptr);
    expect( descent != -17 );

    // Same for external leading.
    int leading = -289;
    dc.GetTextExtent("foo", nullptr, &leading);
    expect( leading != -289 );

    // And both should also work for the empty string as they retrieve the
    // values valid for the entire font and not just this string.
    int descent2;
    int leading2;
    dc.GetTextExtent("", &descent2, &leading2);

    expect( descent2 == descent );
    expect( leading2 == leading );
};

ut::suite DCGetTextExtentTest = []
{
    wxWindow* const win = wxTheApp->GetTopWindow();

    GetTextExtentTester<wxWindow> testWin(*win);
};

ut::suite DCGetPartialTextExtentTest = []
{
    using namespace ut;

    wxClientDC dc = wxTheApp->GetTopWindow();

    std::vector<int> widths = dc.GetPartialTextExtents("Hello");
    
    expect( widths.size() == 5 );
    expect( widths[0] == dc.GetTextExtent("H").x );
    expect( widths[4] == dc.GetTextExtent("Hello").x );
};

#ifdef TEST_GC

ut::suite GCGetTextExtentTest = []
{
    using namespace ut;

#ifndef __WXMSW__
    wxGraphicsRenderer* renderer = wxGraphicsRenderer::GetDefaultRenderer();
#else
    wxGraphicsRenderer* renderer = wxGraphicsRenderer::GetDirect2DRenderer();
#endif

    expect(renderer);
    std::unique_ptr<wxGraphicsContext> context = renderer->CreateMeasuringContext();
    expect(context.get() != nullptr);
    wxFont font(12, wxFontFamily::Default, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
    expect(font.IsOk());
    context->SetFont(font, *wxBLACK);
    float descent, externalLeading = 0.0F;
    auto [width, height] = context->GetTextExtent("x", &descent, &externalLeading);

    // TODO: Determine a way to make these tests more robust.
    expect(width > 0.0F);
    expect(height > 0.0F);
};

#endif // TEST_GC
