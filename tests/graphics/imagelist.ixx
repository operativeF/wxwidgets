///////////////////////////////////////////////////////////////////////////////
// Name:        tests/graphics/imagelist.cpp
// Purpose:     image list unit tests
// Author:      Artur Wieczorek
// Created:     2021-01-11
// Copyright:   (c) 2021 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/bitmap.h"
#include "wx/graphics.h"
#include "wx/icon.h"
#include "wx/imaglist.h"

#include "wx/dcmemory.h"

#include <fmt/format.h>

export module WX.Test.ImageList;

import WX.MetaTest;
import WX.Test.Prec;

// ----------------------------------------------------------------------------
// tests
// ----------------------------------------------------------------------------

namespace ut = boost::ut;

ut::suite ImageListWithMaskTest = []
{
    using namespace ut;

    wxInitAllImageHandlers();

    wxBitmap bmpRGB(wxSize{32, 32}, 24);
    {
        wxMemoryDC mdc(bmpRGB);
        mdc.SetBackground(*wxBLUE_BRUSH);
        mdc.Clear();
        mdc.SetBrush(*wxRED_BRUSH);
        mdc.DrawRectangle(4, 4, 24, 24);
    }

    expect(bmpRGB.IsOk());

    wxBitmap bmpRGBA;
    bmpRGBA.LoadFile("image/data/wx.png", wxBitmapType::PNG);
    expect(bmpRGBA.IsOk());

    wxBitmap bmpMask(wxSize{32, 32}, 1);
    {
        wxMemoryDC mdc(bmpMask);
#if wxUSE_GRAPHICS_CONTEXT
        wxGraphicsContext* gc = mdc.GetGraphicsContext();
        if ( gc )
            gc->SetAntialiasMode(wxAntialiasMode::None);
#endif //wxUSE_GRAPHICS_CONTEXT
        mdc.SetBackground(*wxBLACK_BRUSH);
        mdc.Clear();
        mdc.SetBrush(*wxWHITE_BRUSH);
        mdc.DrawRectangle(0, 0, 16, 32);
    }

    wxBitmap bmpRGBWithMask(bmpRGB);
    bmpRGBWithMask.SetMask(new wxMask(bmpMask));
    expect(bmpRGBWithMask.IsOk());

    wxBitmap bmpRGBAWithMask(bmpRGBA);
    bmpRGBAWithMask.SetMask(new wxMask(bmpMask));
    expect(bmpRGBAWithMask.IsOk());

    wxIcon ico;
    ico.LoadFile("image/data/wx.ico", wxBitmapType::ICO);
    expect(ico.IsOk());

    expect(bmpRGB.HasAlpha() == false);
    expect(bmpRGB.GetMask() == nullptr);

    expect(bmpRGBWithMask.HasAlpha() == false);
    expect(bmpRGBWithMask.GetMask() != nullptr);

    expect(bmpRGBA.HasAlpha() == true);
    expect(bmpRGBA.GetMask() == nullptr);

    expect(bmpRGBAWithMask.HasAlpha() == true);
    expect(bmpRGBAWithMask.GetMask() != nullptr);

    wxImageList il(32, 32, true);

    "Add RGB image to list"_test = [&]
    {
        il.RemoveAll();
        int idx = il.Add(bmpRGB);
        expect(il.GetImageCount() == 1);
        wxBitmap bmp1 = il.GetBitmap(idx);
        expect(bmp1.HasAlpha() == false);
        expect(bmp1.GetMask() != nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        idx = il.Add(bmpRGBWithMask);
        expect(il.GetImageCount() == 2);
        wxBitmap bmp2 = il.GetBitmap(idx);
        expect(bmp2.HasAlpha() == false);
        expect(bmp2.GetMask() != nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);

        idx = il.Add(bmpRGB, *wxRED);
        expect(il.GetImageCount() == 3);
        wxBitmap bmp3 = il.GetBitmap(idx);
        expect(bmp3.HasAlpha() == false);
        expect(bmp3.GetMask() != nullptr);
        expect(bmp3.GetWidth() == 32);
        expect(bmp3.GetHeight() == 32);
    };

    "Add RGBA image to list"_test = [&]
    {
        il.RemoveAll();
        int idx = il.Add(bmpRGBA);
        expect(il.GetImageCount() == 1);
        wxBitmap bmp1 = il.GetBitmap(idx);
        expect(bmp1.HasAlpha() == false);
        expect(bmp1.GetMask() != nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        idx = il.Add(bmpRGBAWithMask);
        expect(il.GetImageCount() == 2);
        wxBitmap bmp2 = il.GetBitmap(idx);
        expect(bmp2.HasAlpha() == false);
        expect(bmp2.GetMask() != nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);

        idx = il.Add(bmpRGBA, *wxRED);
        expect(il.GetImageCount() == 3);
        wxBitmap bmp3 = il.GetBitmap(idx);
        expect(bmp3.HasAlpha() == false);
        expect(bmp3.GetMask() != nullptr);
        expect(bmp3.GetWidth() == 32);
        expect(bmp3.GetHeight() == 32);
    };

    "Add icon to list"_test = [&]
    {
        il.RemoveAll();
        int idx = il.Add(ico);
        expect(il.GetImageCount() == 1);
        wxIcon icon1 = il.GetIcon(idx);
        expect(icon1.GetWidth() == 32);
        expect(icon1.GetHeight() == 32);
    };

    "Replace with RGB image"_test = [&]
    {
        il.RemoveAll();
        int idx1 = il.Add(bmpRGBA);
        expect(il.GetImageCount() == 1);
        int idx2 = il.Add(bmpRGBAWithMask);
        expect(il.GetImageCount() == 2);

        il.Replace(idx1, bmpRGB);
        il.Replace(idx2, bmpRGBWithMask);

        wxBitmap bmp1 = il.GetBitmap(idx1);
        expect(bmp1.HasAlpha() == false);
        expect(bmp1.GetMask() != nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        wxBitmap bmp2 = il.GetBitmap(idx2);
        expect(bmp2.HasAlpha() == false);
        expect(bmp2.GetMask() != nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);
    };

    "Replace with RGBA image"_test = [&]
    {
        il.RemoveAll();
        int idx1 = il.Add(bmpRGB);
        expect(il.GetImageCount() == 1);
        int idx2 = il.Add(bmpRGBWithMask);
        expect(il.GetImageCount() == 2);

        il.Replace(idx1, bmpRGBA);
        il.Replace(idx2, bmpRGBAWithMask);

        wxBitmap bmp1 = il.GetBitmap(idx1);
        expect(bmp1.HasAlpha() == false);
        expect(bmp1.GetMask() != nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        wxBitmap bmp2 = il.GetBitmap(idx2);
        expect(bmp2.HasAlpha() == false);
        expect(bmp2.GetMask() != nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);
    };

    "Add images with incompatible sizes"_test = [&]
    {
        il.RemoveAll();
        wxSize sz = il.GetSize();

        wxBitmap bmpSmallerW(wxSize{sz.GetWidth() / 2, sz.GetHeight()}, 24);
        {
            wxMemoryDC mdc(bmpSmallerW);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpSmallerW.IsOk());

        wxBitmap bmpSmallerH(wxSize{sz.GetWidth(), sz.GetHeight() / 2}, 24);
        {
            wxMemoryDC mdc(bmpSmallerH);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpSmallerH.IsOk());

        wxBitmap bmpSmallerWH(wxSize{sz.GetWidth() / 2, sz.GetHeight() / 2}, 24);
        {
            wxMemoryDC mdc(bmpSmallerWH);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpSmallerWH.IsOk());

        wxBitmap bmpBiggerW(wxSize{sz.GetWidth() * 3 / 2, sz.GetHeight()}, 24);
        {
            wxMemoryDC mdc(bmpBiggerW);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpBiggerW.IsOk());

        wxBitmap bmpBiggerW2x(wxSize{sz.GetWidth() * 2, sz.GetHeight()}, 24);
        {
            wxMemoryDC mdc(bmpBiggerW2x);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpBiggerW2x.IsOk());

        wxBitmap bmpBiggerH(wxSize{sz.GetWidth(), sz.GetHeight() * 3 / 2}, 24);
        {
            wxMemoryDC mdc(bmpBiggerH);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpBiggerH.IsOk());

        wxBitmap bmpBiggerH2x(wxSize{sz.GetWidth(), sz.GetHeight() * 2}, 24);
        {
            wxMemoryDC mdc(bmpBiggerH2x);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpBiggerH2x.IsOk());

        wxBitmap bmpBiggerWH(wxSize{sz.GetWidth() * 3 / 2, sz.GetHeight() * 3 / 2}, 24);
        {
            wxMemoryDC mdc(bmpBiggerWH);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpBiggerWH.IsOk());

        wxBitmap bmpBiggerWH2x(wxSize{sz.GetWidth() * 2, sz.GetHeight() * 2}, 24);
        {
            wxMemoryDC mdc(bmpBiggerWH2x);
            mdc.SetBackground(*wxBLUE_BRUSH);
            mdc.Clear();
        }

        expect(bmpBiggerWH2x.IsOk());

        // Adding
        int cnt = il.GetImageCount();
        int idx = il.Add(bmpSmallerW);
        expect(idx == -1);
        expect(il.GetImageCount() == cnt);

        cnt = il.GetImageCount();
        idx = il.Add(bmpSmallerH);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 1);
        wxBitmap bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        idx = il.Add(bmpSmallerWH);
        expect(idx == -1);
        expect(il.GetImageCount() == cnt);

        cnt = il.GetImageCount();
        idx = il.Add(bmpBiggerW);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 1);
        bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        idx = il.Add(bmpBiggerW2x);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 2);
        bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        idx = il.Add(bmpBiggerH);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 1);
        bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        idx = il.Add(bmpBiggerH2x);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 1);
        bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        idx = il.Add(bmpBiggerWH);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 1);
        bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        idx = il.Add(bmpBiggerWH2x);
        expect(idx >= 0);
        expect(il.GetImageCount() == cnt + 2);
        bmp = il.GetBitmap(idx);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        // Replacing
        il.RemoveAll();

        cnt = il.GetImageCount();
        bool ok = il.Replace(0, bmpRGBA);
        expect(ok == false);
        expect(il.GetImageCount() == cnt);

        // List with 1 image
        idx = il.Add(bmpRGB);
        expect(idx >= 0);

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpRGBA);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpSmallerW);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpSmallerH);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpSmallerWH);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpBiggerW);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpBiggerW2x);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpBiggerH);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpBiggerH2x);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpBiggerWH);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());

        cnt = il.GetImageCount();
        ok = il.Replace(0, bmpBiggerWH2x);
        expect(ok == true);
        expect(il.GetImageCount() == cnt);
        bmp = il.GetBitmap(0);
        expect(bmp.GetWidth() == sz.GetWidth());
        expect(bmp.GetHeight() == sz.GetHeight());
    };
};

ut::suite ImageListNoMaskTest = []
{
    using namespace ut;

    wxInitAllImageHandlers();

    wxBitmap bmpRGB(wxSize{32, 32}, 24);
    {
        wxMemoryDC mdc(bmpRGB);
        mdc.SetBackground(*wxBLUE_BRUSH);
        mdc.Clear();
        mdc.SetBrush(*wxRED_BRUSH);
        mdc.DrawRectangle(4, 4, 24, 24);
    }

    expect(bmpRGB.IsOk());

    wxBitmap bmpRGBA;
    bmpRGBA.LoadFile("image/data/wx.png", wxBitmapType::PNG);
    expect(bmpRGBA.IsOk());

    wxBitmap bmpMask(wxSize{32, 32}, 1);
    {
        wxMemoryDC mdc(bmpMask);
#if wxUSE_GRAPHICS_CONTEXT
        wxGraphicsContext* gc = mdc.GetGraphicsContext();
        if ( gc )
            gc->SetAntialiasMode(wxAntialiasMode::None);
#endif //wxUSE_GRAPHICS_CONTEXT
        mdc.SetBackground(*wxBLACK_BRUSH);
        mdc.Clear();
        mdc.SetBrush(*wxWHITE_BRUSH);
        mdc.DrawRectangle(0, 0, 16, 32);
    }

    wxBitmap bmpRGBWithMask(bmpRGB);
    bmpRGBWithMask.SetMask(new wxMask(bmpMask));
    expect(bmpRGBWithMask.IsOk());

    wxBitmap bmpRGBAWithMask(bmpRGBA);
    bmpRGBAWithMask.SetMask(new wxMask(bmpMask));
    expect(bmpRGBAWithMask.IsOk());

    wxIcon ico;
    ico.LoadFile("image/data/wx.ico", wxBitmapType::ICO);
    expect(ico.IsOk());

    expect(bmpRGB.HasAlpha() == false);
    expect(bmpRGB.GetMask() == nullptr);

    expect(bmpRGBWithMask.HasAlpha() == false);
    expect(bmpRGBWithMask.GetMask() != nullptr);

    expect(bmpRGBA.HasAlpha() == true);
    expect(bmpRGBA.GetMask() == nullptr);

    expect(bmpRGBAWithMask.HasAlpha() == true);
    expect(bmpRGBAWithMask.GetMask() != nullptr);

    wxImageList il(32, 32, false);

    "Add RGB image to list"_test = [&]
    {
        il.RemoveAll();
        int idx = il.Add(bmpRGB);
        expect(il.GetImageCount() == 1);
        wxBitmap bmp1 = il.GetBitmap(idx);
        expect(bmp1.HasAlpha() == false);
        expect(bmp1.GetMask() == nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        idx = il.Add(bmpRGBWithMask);
        expect(il.GetImageCount() == 2);
        wxBitmap bmp2 = il.GetBitmap(idx);
        expect(bmp2.HasAlpha() == true);
        expect(bmp2.GetMask() == nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);

        idx = il.Add(bmpRGB, *wxRED);
        expect(il.GetImageCount() == 3);
        wxBitmap bmp3 = il.GetBitmap(idx);
        expect(bmp3.HasAlpha() == true);
        expect(bmp3.GetMask() == nullptr);
        expect(bmp3.GetWidth() == 32);
        expect(bmp3.GetHeight() == 32);
    };

    "Add RGBA image to list"_test = [&]
    {
        il.RemoveAll();
        int idx = il.Add(bmpRGBA);
        expect(il.GetImageCount() == 1);
        wxBitmap bmp1 = il.GetBitmap(idx);
        expect(bmp1.HasAlpha() == true);
        expect(bmp1.GetMask() == nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        idx = il.Add(bmpRGBAWithMask);
        expect(il.GetImageCount() == 2);
        wxBitmap bmp2 = il.GetBitmap(idx);
        expect(bmp2.HasAlpha() == true);
        expect(bmp2.GetMask() == nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);

        idx = il.Add(bmpRGBA, *wxRED);
        expect(il.GetImageCount() == 3);
        wxBitmap bmp3 = il.GetBitmap(idx);
        expect(bmp3.HasAlpha() == true);
        expect(bmp3.GetMask() == nullptr);
        expect(bmp3.GetWidth() == 32);
        expect(bmp3.GetHeight() == 32);
    };

    "Add icon to list"_test = [&]
    {
        il.RemoveAll();
        int idx = il.Add(ico);
        expect(il.GetImageCount() == 1);
        wxIcon icon1 = il.GetIcon(idx);
        expect(icon1.GetWidth() == 32);
        expect(icon1.GetHeight() == 32);
    };

    "Replace with RGB image"_test = [&]
    {
        il.RemoveAll();
        int idx1 = il.Add(bmpRGBA);
        expect(il.GetImageCount() == 1);
        int idx2 = il.Add(bmpRGBAWithMask);
        expect(il.GetImageCount() == 2);

        il.Replace(idx1, bmpRGB);
        il.Replace(idx2, bmpRGBWithMask);

        wxBitmap bmp1 = il.GetBitmap(idx1);
        expect(bmp1.HasAlpha() == false);
        expect(bmp1.GetMask() == nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        wxBitmap bmp2 = il.GetBitmap(idx2);
        expect(bmp2.HasAlpha() == true);
        expect(bmp2.GetMask() == nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);
    };

    "Replace with RGBA image"_test = [&]
    {
        il.RemoveAll();
        int idx1 = il.Add(bmpRGB);
        expect(il.GetImageCount() == 1);
        int idx2 = il.Add(bmpRGBWithMask);
        expect(il.GetImageCount() == 2);

        il.Replace(idx1, bmpRGBA);
        il.Replace(idx2, bmpRGBAWithMask);

        wxBitmap bmp1 = il.GetBitmap(idx1);
        expect(bmp1.HasAlpha() == true);
        expect(bmp1.GetMask() == nullptr);
        expect(bmp1.GetWidth() == 32);
        expect(bmp1.GetHeight() == 32);

        wxBitmap bmp2 = il.GetBitmap(idx2);
        expect(bmp2.HasAlpha() == true);
        expect(bmp2.GetMask() == nullptr);
        expect(bmp2.GetWidth() == 32);
        expect(bmp2.GetHeight() == 32);
    };
};

ut::suite ImageListNegativeTests = []
{
    using namespace ut;

    wxBitmap bmp(wxSize{32, 32}, 24);
    {
        wxMemoryDC mdc(bmp);
        mdc.SetBackground(*wxBLUE_BRUSH);
        mdc.Clear();
        mdc.SetBrush(*wxRED_BRUSH);
        mdc.DrawRectangle(4, 4, 24, 24);
    }

    expect(bmp.IsOk());

    "Invalid size (negative)"_test = [&]
    {
        wxImageList il;
        bool ok = il.Create(-1, -1);
        expect(!ok);
#ifdef __WXDEBUG__
        expect(throws([&il] { il.GetImageCount(); }));
#else
        expect(il.GetImageCount() == 0);
#endif

        wxSize sz = il.GetSize();
        expect(sz.x == 0);
        expect(sz.y == 0);

        int w = -1;
        int h = -1;
#ifdef __WXDEBUG__
        expect(throws([&]{ il.GetSize(0, w, h); }));
#else
        ok = il.GetSize(0, w, h);
        expect(!ok);
        expect(w == 0);
        expect(h == 0);
#endif

        int idx = il.Add(bmp);
        expect(idx == -1);
#ifdef __WXDEBUG__
        expect(throws([&]{ il.GetImageCount(); }));
#else
        expect(il.GetImageCount() == 0);
#endif
    };

    "Invalid size (zero)"_test = [&]
    {
        wxImageList il;
        bool ok = il.Create(0, 0);
        expect(!ok);
#ifdef __WXDEBUG__
        expect(throws([&]{ il.GetImageCount(); }));
#else
        expect(il.GetImageCount() == 0);
#endif

        wxSize sz = il.GetSize();
        expect(sz.x == 0);
        expect(sz.y == 0);

        int w = -1;
        int h = -1;
#ifdef __WXDEBUG__
        expect(throws([&]{ ok = il.GetSize(0, w, h); }));
#else
        ok = il.GetSize(0, w, h);
        expect(!ok);
        expect(w == 0);
        expect(h == 0);
#endif

        int idx = il.Add(bmp);
        expect(idx == -1);
#ifdef __WXDEBUG__
        expect(throws([&]{ il.GetImageCount(); }));
#else
        expect(il.GetImageCount() == 0);
#endif

        ok = il.Replace(0, bmp);
        expect(!ok);
#ifdef __WXDEBUG__
        expect(throws([&]{ il.GetImageCount(); }));
#else
        expect(il.GetImageCount() == 0);
#endif
    };

    "Invalid Get/Replace/Remove indices"_test = [&]
    {
        wxImageList il(32, 32, false);
        expect(il.GetImageCount() == 0);

        wxSize sz = il.GetSize();
        expect(sz.x == 32);
        expect(sz.y == 32);

        int w = -1;
        int h = -1;
        bool ok = il.GetSize(0, w, h);
        expect(ok == true);
        expect(w == 32);
        expect(h == 32);

        int idx = il.Add(bmp);
        expect(idx == 0);
        expect(il.GetImageCount() == 1);

        wxBitmap bmp2 = il.GetBitmap(-1);
        expect(!bmp2.IsOk());
        expect(il.GetImageCount() == 1);

        wxBitmap bmp3 = il.GetBitmap(5);
        expect(!bmp3.IsOk());
        expect(il.GetImageCount() == 1);

        wxIcon icon2 = il.GetIcon(-1);
        expect(!icon2.IsOk());
        expect(il.GetImageCount() == 1);

        wxBitmap icon3 = il.GetIcon(5);
        expect(!icon3.IsOk());
        expect(il.GetImageCount() == 1);

        ok = il.Replace(-1, bmp);
        expect(!ok);
        expect(il.GetImageCount() == 1);

        ok = il.Replace(5, bmp);
        expect(!ok);
        expect(il.GetImageCount() == 1);

        ok = il.Remove(-1);
        expect(!ok);
        expect(il.GetImageCount() == 1);

        ok = il.Remove(5);
        expect(!ok);
        expect(il.GetImageCount() == 1);
    };
};

// Skipping HiDPI image tests known not to work in wxMSW and wxGTK2.
#if !defined(__WXMSW__)  || ( !defined(__WXGTK20__) && defined(__WXGTK3__) )
TEST_CASE("ImageList:HiDPI")
{
    wxImage img(16, 8);
    img.SetRGB(wxRect(0, 0, 16, 8), 255, 128, 64);
    REQUIRE(img.IsOk());

    wxBitmap bmp1x(img, -1, 1.0);
    REQUIRE(bmp1x.IsOk());
    CHECK(bmp1x.GetWidth() == 16);
    CHECK(bmp1x.GetHeight() == 8);
    CHECK(bmp1x.GetScaledWidth() == 16);
    CHECK(bmp1x.GetScaledHeight() == 8);
    CHECK_FALSE(bmp1x.HasAlpha());
    CHECK(bmp1x.GetMask() == nullptr);

    wxBitmap bmp2x(img, -1, 2.0);
    REQUIRE(bmp2x.IsOk());
    CHECK(bmp2x.GetWidth() == 16);
    CHECK(bmp2x.GetHeight() == 8);
    CHECK(bmp2x.GetScaledWidth() == 8);
    CHECK(bmp2x.GetScaledHeight() == 4);
    CHECK_FALSE(bmp2x.HasAlpha());
    CHECK(bmp2x.GetMask() == nullptr);

    SUBCASE("Add images 2x to the list 2x")
    {
        // Logical image size
        wxImageList il(8, 4, false);

        int idx = il.Add(bmp2x);
        CHECK(idx == 0);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp1x);
        CHECK(idx == -1);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp2x);
        CHECK(idx == 1);
        CHECK(il.GetImageCount() == 2);

        idx = il.Add(bmp1x);
        CHECK(idx == -1);
        CHECK(il.GetImageCount() == 2);

        wxBitmap bmp = il.GetBitmap(1);
        CHECK(bmp.IsOk() == true);
        CHECK(bmp.GetScaleFactor() == 2.0);
        CHECK(bmp.GetScaledWidth() == 8);
        CHECK(bmp.GetScaledHeight() == 4);
        CHECK(bmp.GetWidth() == 16);
        CHECK(bmp.GetHeight() == 8);
        CHECK_FALSE(bmp.HasAlpha());
        CHECK(bmp.GetMask() == nullptr);
    }

    SUBCASE("Add images 2x to the list 1x")
    {
        // Logical image size
        wxImageList il(16, 8, false);

        int idx = il.Add(bmp1x);
        CHECK(idx == 0);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp2x);
        CHECK(idx == -1);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp1x);
        CHECK(idx == 1);
        CHECK(il.GetImageCount() == 2);

        idx = il.Add(bmp2x);
        CHECK(idx == -1);
        CHECK(il.GetImageCount() == 2);

        wxBitmap bmp = il.GetBitmap(1);
        CHECK(bmp.IsOk() == true);
        CHECK(bmp.GetScaleFactor() == 1.0);
        CHECK(bmp.GetScaledWidth() == 16);
        CHECK(bmp.GetScaledHeight() == 8);
        CHECK(bmp.GetWidth() == 16);
        CHECK(bmp.GetHeight() == 8);
        CHECK_FALSE(bmp.HasAlpha());
        CHECK(bmp.GetMask() == nullptr);
    }

    SUBCASE("Replaces images in the list 2x")
    {
        // Logical image size
        wxImageList il(8, 4, false);

        int idx = il.Add(bmp2x);
        CHECK(idx == 0);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp2x);
        CHECK(idx == 1);
        CHECK(il.GetImageCount() == 2);

        bool ok = il.Replace(1, bmp1x);
        CHECK_FALSE(ok);
        CHECK(il.GetImageCount() == 2);

        ok = il.Replace(0, bmp2x);
        CHECK(ok == true);
        CHECK(il.GetImageCount() == 2);

        wxBitmap bmp = il.GetBitmap(0);
        CHECK(bmp.IsOk() == true);
        CHECK(bmp.GetScaleFactor() == 2.0);
        CHECK(bmp.GetScaledWidth() == 8);
        CHECK(bmp.GetScaledHeight() == 4);
        CHECK(bmp.GetWidth() == 16);
        CHECK(bmp.GetHeight() == 8);
        CHECK_FALSE(bmp.HasAlpha());
        CHECK(bmp.GetMask() == nullptr);
    }

    SUBCASE("Replaces images in the list 1x")
    {
        // Logical image size
        wxImageList il(16, 8, false);

        int idx = il.Add(bmp1x);
        CHECK(idx == 0);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp1x);
        CHECK(idx == 1);
        CHECK(il.GetImageCount() == 2);

        bool ok = il.Replace(1, bmp2x);
        CHECK_FALSE(ok);
        CHECK(il.GetImageCount() == 2);

        ok = il.Replace(0, bmp1x);
        CHECK(ok == true);
        CHECK(il.GetImageCount() == 2);

        wxBitmap bmp = il.GetBitmap(0);
        CHECK(bmp.GetScaleFactor() == 1.0);
        CHECK(bmp.GetScaledWidth() == 16);
        CHECK(bmp.GetScaledHeight() == 8);
        CHECK(bmp.GetWidth() == 16);
        CHECK(bmp.GetHeight() == 8);
        CHECK_FALSE(bmp.HasAlpha());
        CHECK(bmp.GetMask() == nullptr);
    }

    SUBCASE("Changes list 1x to 2x")
    {
        wxImage img2(32, 16);
        img2.SetRGB(wxRect(0, 0, 32, 16), 255, 128, 64);
        REQUIRE(img2.IsOk());

        wxBitmap bmp2x2(img2, -1, 2.0);
        REQUIRE(bmp2x2.IsOk());
        CHECK(bmp2x2.GetWidth() == 32);
        CHECK(bmp2x2.GetHeight() == 16);
        CHECK(bmp2x2.GetScaledWidth() == 16);
        CHECK(bmp2x2.GetScaledHeight() == 8);
        CHECK(bmp2x2.HasAlpha() == false);
        CHECK(bmp2x2.GetMask() == nullptr);

        // Logical image size
        wxImageList il(16, 8, false);

        // Now it should be the list with 1x images
        int idx = il.Add(bmp1x);
        CHECK(idx == 0);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp1x);
        CHECK(idx == 1);
        CHECK(il.GetImageCount() == 2);

        idx = il.Add(bmp2x2);
        CHECK(idx == -1);
        CHECK(il.GetImageCount() == 2);

        wxBitmap bmp = il.GetBitmap(0);
        CHECK(bmp.GetScaleFactor() == 1.0);
        CHECK(bmp.GetScaledWidth() == 16);
        CHECK(bmp.GetScaledHeight() == 8);
        CHECK(bmp.GetWidth() == 16);
        CHECK(bmp.GetHeight() == 8);
        CHECK_FALSE(bmp.HasAlpha());
        CHECK(bmp.GetMask() == nullptr);

        il.RemoveAll();

        // Now it should be the list with 2x images (the same logical size 16x8)
        idx = il.Add(bmp2x2);
        CHECK(idx == 0);
        CHECK(il.GetImageCount() == 1);

        idx = il.Add(bmp2x2);
        CHECK(idx == 1);
        CHECK(il.GetImageCount() == 2);

        idx = il.Add(bmp1x);
        CHECK(idx == -1);
        CHECK(il.GetImageCount() == 2);

        bmp = il.GetBitmap(0);
        CHECK(bmp.GetScaleFactor() == 2.0);
        CHECK(bmp.GetScaledWidth() == 16);
        CHECK(bmp.GetScaledHeight() == 8);
        CHECK(bmp.GetWidth() == 32);
        CHECK(bmp.GetHeight() == 16);
        CHECK_FALSE(bmp.HasAlpha());
        CHECK(bmp.GetMask() == nullptr);
    }
}
#endif // !__WXMSW__
