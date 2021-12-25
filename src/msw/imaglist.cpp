/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/imaglist.cpp
// Purpose:     wxImageList implementation for Win32
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/app.h"
#include "wx/dc.h"
#include "wx/dcmemory.h"
#include "wx/icon.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/window.h"
#include "wx/dc.h"

#include "wx/imaglist.h"
#include "wx/msw/dc.h"
#include "wx/msw/dib.h"

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"

import WX.Image;
import WX.Win.UniqueHnd;
import WX.WinDef;

import <algorithm>;

using msw::utils::unique_bitmap;


#define GetHImageList()     ((HIMAGELIST)m_hImageList)

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// returns the mask if it's valid, otherwise the bitmap mask and, if it's not
// valid neither, a "solid" mask (no transparent zones at all)
static WXHBITMAP GetMaskForImage(const wxBitmap& bitmap, const wxBitmap& mask);

// Creates an image list
bool wxImageList::Create(int width, int height, bool mask, int initial)
{
    // Prevent from storing negative dimensions
    m_size = wxSize(std::max(width, 0), std::max(height, 0));
    WXUINT flags = 0;

    // as we want to be able to use 32bpp bitmaps in the image lists, we always
    // use ILC_COLOR32, even if the display resolution is less -- the system
    // will make the best effort to show the bitmap if we do this resulting in
    // quite acceptable display while using a lower depth ILC_COLOR constant
    // (e.g. ILC_COLOR16) shows completely broken bitmaps
    flags |= ILC_COLOR32;

    // For comctl32.dll < 6 always use masks as it doesn't support alpha.
    if ( mask )
        flags |= ILC_MASK;

    // Grow by 1, I guess this is reasonable behaviour most of the time
    m_hImageList = (WXHIMAGELIST) ImageList_Create(width, height, flags,
                                                   initial, 1);
    if ( !m_hImageList )
    {
        wxLogLastError("ImageList_Create()");
    }

    m_useMask = (flags & ILC_MASK) != 0;
    return m_hImageList != nullptr;
}

wxImageList::~wxImageList()
{
    if ( m_hImageList )
    {
        ImageList_Destroy(GetHImageList());
        m_hImageList = nullptr;
    }
}

// ----------------------------------------------------------------------------
// wxImageList attributes
// ----------------------------------------------------------------------------

// Returns the number of images in the image list.
int wxImageList::GetImageCount() const
{
    wxASSERT_MSG( m_hImageList, "invalid image list" );

    return ImageList_GetImageCount(GetHImageList());
}

// Returns the size (same for all images) of the images in the list
bool wxImageList::GetSize([[maybe_unused]] int index, int &width, int &height) const
{
    wxASSERT_MSG( m_hImageList, "invalid image list" );

    return ImageList_GetIconSize(GetHImageList(), &width, &height) != 0;
}

// ----------------------------------------------------------------------------
// wxImageList operations
// ----------------------------------------------------------------------------

namespace
{
void GetImageListBitmaps(const wxBitmap& bitmap, const wxBitmap& mask, bool useMask,
                         unique_bitmap& hbmpRelease, unique_bitmap& hbmpMask, WXHBITMAP& hbmp)
{
#if wxUSE_WXDIB && wxUSE_IMAGE
    // wxBitmap normally stores alpha in pre-multiplied format but
    // ImageList_Draw() does pre-multiplication internally so we need to undo
    // the pre-multiplication here. Converting back and forth like this is, of
    // course, very inefficient but it's better than wrong appearance so we do
    // this for now until a better way can be found.
    if ( useMask )
    {
        if ( bitmap.HasAlpha() )
        {
            // Remove alpha channel from image to prevent
            // possible interferences with the mask.
            // The bitmap isn't drawn correctly if we use both.
            wxImage img = bitmap.ConvertToImage();
            img.ClearAlpha();
            hbmp = wxDIB(img, wxDIB::PixelFormat::NotPreMultiplied).Detach();
            hbmpRelease = unique_bitmap(hbmp);
        }
        else
        {
            hbmp = GetHbitmapOf(bitmap);
        }

        hbmpMask = unique_bitmap(GetMaskForImage(bitmap, mask));
    }
    else
    {
        if ( bitmap.HasAlpha() )
        {
            wxBitmap bmp(bitmap);
            if ( mask.IsOk() || bmp.GetMask() )
            {
                // Blend mask with alpha channel.
                if ( mask.IsOk() )
                {
                    bmp.SetMask(new wxMask(mask));
                }
                bmp.MSWBlendMaskWithAlpha();
            }
            wxImage img = bmp.ConvertToImage();
            hbmp = wxDIB(img, wxDIB::PixelFormat::NotPreMultiplied).Detach();
            hbmpRelease = unique_bitmap(hbmp);
        }
        else
        {
            if ( mask.IsOk() || bitmap.GetMask() )
            {
                // Convert mask to alpha channel.
                wxBitmap bmp(bitmap);
                if ( mask.IsOk() )
                {
                    bmp.SetMask(new wxMask(mask));
                }
                wxImage img = bmp.ConvertToImage();
                img.InitAlpha();
                hbmp = wxDIB(img, wxDIB::PixelFormat::NotPreMultiplied).Detach();
                hbmpRelease = unique_bitmap(hbmp);
            }
            else
            {
                hbmp = GetHbitmapOf(bitmap);
            }
        }
    }
#else
    hbmp = GetHbitmapOf(bitmap);
#endif // wxUSE_WXDIB && wxUSE_IMAGE
}
};

// Adds a bitmap, and optionally a mask bitmap.
// Note that wxImageList creates new bitmaps, so you may delete
// 'bitmap' and 'mask'.
int wxImageList::Add(const wxBitmap& bitmap, const wxBitmap& mask)
{
    WXHBITMAP hbmp = nullptr;
    unique_bitmap hbmpRelease;
    unique_bitmap hbmpMask;

    GetImageListBitmaps(bitmap, mask, m_useMask, hbmpRelease, hbmpMask, hbmp);

    int index = ImageList_Add(GetHImageList(), hbmp, hbmpMask.get());
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    return index;
}

// Adds a bitmap, using the specified colour to create the mask bitmap
// Note that wxImageList creates new bitmaps, so you may delete
// 'bitmap'.
int wxImageList::Add(const wxBitmap& bitmap, const wxColour& maskColour)
{
    WXHBITMAP hbmp = nullptr;
    unique_bitmap hbmpRelease;
    unique_bitmap hbmpMask;

    wxMask mask(bitmap, maskColour);
    GetImageListBitmaps(bitmap, mask.GetBitmap(), m_useMask, hbmpRelease, hbmpMask, hbmp);

    int index = ImageList_AddMasked(GetHImageList(),
                                    hbmp,
                                    wxColourToRGB(maskColour));
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    return index;
}

// Adds a bitmap and mask from an icon.
int wxImageList::Add(const wxIcon& icon)
{
    int index = ImageList_AddIcon(GetHImageList(), GetHiconOf(icon));
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    return index;
}

// Replaces a bitmap, optionally passing a mask bitmap.
// Note that wxImageList creates new bitmaps, so you may delete
// 'bitmap' and 'mask'.
bool wxImageList::Replace(int index,
                          const wxBitmap& bitmap,
                          const wxBitmap& mask)
{
    WXHBITMAP hbmp = nullptr;
    unique_bitmap hbmpRelease;
    unique_bitmap hbmpMask;
    GetImageListBitmaps(bitmap, mask, m_useMask, hbmpRelease, hbmpMask, hbmp);

    if ( !ImageList_Replace(GetHImageList(), index, hbmp, hbmpMask.get()) )
    {
        wxLogLastError("ImageList_Replace()");
        return false;
    }

    return true;
}

// Replaces a bitmap and mask from an icon.
bool wxImageList::Replace(int i, const wxIcon& icon)
{
    bool ok = ImageList_ReplaceIcon(GetHImageList(), i, GetHiconOf(icon)) != -1;
    if ( !ok )
    {
        wxLogLastError("ImageList_ReplaceIcon()");
    }

    return ok;
}

// Removes the image at the given index.
bool wxImageList::Remove(int index)
{
    bool ok = index >= 0 && ImageList_Remove(GetHImageList(), index) != FALSE;
    if ( !ok )
    {
        wxLogLastError("ImageList_Remove()");
    }

    return ok;
}

// Remove all images
bool wxImageList::RemoveAll()
{
    // don't use ImageList_RemoveAll() because mingw32 headers don't have it
    bool ok = ImageList_Remove(GetHImageList(), -1) != FALSE;
    if ( !ok )
    {
        wxLogLastError("ImageList_Remove()");
    }

    return ok;
}

// Draws the given image on a dc at the specified position.
// If 'solidBackground' is true, Draw sets the image list background
// colour to the background colour of the wxDC, to speed up
// drawing by eliminating masked drawing where possible.
bool wxImageList::Draw(int index,
                       wxDC& dc,
                       int x, int y,
                       unsigned int flags,
                       bool solidBackground)
{
    wxDCImpl *impl = dc.GetImpl();
    auto msw_impl = dynamic_cast<wxMSWDCImpl*>( impl );
    if (!msw_impl)
       return false;

    WXHDC hDC = GetHdcOf(*msw_impl);
    wxCHECK_MSG( hDC, false, "invalid wxDC in wxImageList::Draw" );

    COLORREF clr = CLR_NONE;    // transparent by default
    if ( solidBackground )
    {
        const wxBrush& brush = dc.GetBackground();
        if ( brush.IsOk() )
        {
            clr = wxColourToRGB(brush.GetColour());
        }
    }

    ImageList_SetBkColor(GetHImageList(), clr);

    WXUINT style = 0;
    if ( flags & wxIMAGELIST_DRAW_NORMAL )
        style |= ILD_NORMAL;
    if ( flags & wxIMAGELIST_DRAW_TRANSPARENT )
        style |= ILD_TRANSPARENT;
    if ( flags & wxIMAGELIST_DRAW_SELECTED )
        style |= ILD_SELECTED;
    if ( flags & wxIMAGELIST_DRAW_FOCUSED )
        style |= ILD_FOCUS;

    bool ok = ImageList_Draw(GetHImageList(), index, hDC, x, y, style) != 0;
    if ( !ok )
    {
        wxLogLastError("ImageList_Draw()");
    }

    return ok;
}

// Get the bitmap
wxBitmap wxImageList::GetBitmap(int index) const
{
    // TODO: Return size
    int bmp_width = 0, bmp_height = 0;
    GetSize(index, bmp_width, bmp_height);

    wxBitmap bitmap(wxSize{bmp_width, bmp_height});

#if wxUSE_WXDIB && wxUSE_IMAGE
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    IMAGEINFO ii;
    ImageList_GetImageInfo(GetHImageList(), index, &ii);
    if ( ii.hbmMask )
    {
        // draw it the first time to find a suitable mask colour
        if ( !const_cast<wxImageList*>(this)->Draw(index, dc, 0, 0, wxIMAGELIST_DRAW_TRANSPARENT) )
            return wxNullBitmap;

        dc.SelectObject(wxNullBitmap);

        // find the suitable mask colour
        wxImage image = bitmap.ConvertToImage();
        unsigned char r = 0, g = 0, b = 0;
        image.FindFirstUnusedColour(&r, &g, &b);

        // redraw whole image and bitmap in the mask colour
        image.Create(bmp_width, bmp_height);
        image.Replace(0, 0, 0, r, g, b);
        bitmap = wxBitmap(image);

        // redraw icon over the mask colour to actually draw it
        dc.SelectObject(bitmap);
        const_cast<wxImageList*>(this)->Draw(index, dc, 0, 0, wxIMAGELIST_DRAW_TRANSPARENT);
        dc.SelectObject(wxNullBitmap);

        // get the image, set the mask colour and convert back to get transparent bitmap
        image = bitmap.ConvertToImage();
        image.SetMaskColour(r, g, b);
        bitmap = wxBitmap(image);
    }
    else // no mask
    {
        // Just draw it normally.
        if ( !const_cast<wxImageList*>(this)->Draw(index, dc, 0, 0, wxIMAGELIST_DRAW_NORMAL) )
            return wxNullBitmap;

        dc.SelectObject(wxNullBitmap);

        // And adjust its alpha flag as the destination bitmap would get it if
        // the source one had it.
        //
        // Note that perhaps we could just call UseAlpha() which would set the
        // "has alpha" flag unconditionally as it doesn't seem to do any harm,
        // but for now only do it if necessary, just to be on the safe side,
        // even if it requires more work (and takes more time).
        bitmap.MSWUpdateAlpha();
    }
#endif
    return bitmap;
}

// Get the icon
wxIcon wxImageList::GetIcon(int index) const
{
    WXHICON hIcon = ImageList_ExtractIcon(0, GetHImageList(), index);
    if (hIcon)
    {
        wxIcon icon;

        // TODO: Return size.
        int iconW, iconH;
        GetSize(index, iconW, iconH);
        icon.InitFromHICON((WXHICON)hIcon, wxSize{iconW, iconH});

        return icon;
    }
    else
        return wxNullIcon;
}

// ----------------------------------------------------------------------------
// helpers
// ----------------------------------------------------------------------------

static WXHBITMAP GetMaskForImage(const wxBitmap& bitmap, const wxBitmap& mask)
{
#if wxUSE_IMAGE
    wxBitmap bitmapWithMask;
#endif // wxUSE_IMAGE

    WXHBITMAP hbmpMask;
    wxMask *pMask{nullptr};
    bool deleteMask = false;

    if ( mask.IsOk() )
    {
        hbmpMask = GetHbitmapOf(mask);
    }
    else
    {
        pMask = bitmap.GetMask();

#if wxUSE_IMAGE
        // check if we don't have alpha in this bitmap -- we can create a mask
        // from it (and we need to do it for the older systems which don't
        // support 32bpp bitmaps natively)
        if ( !pMask )
        {
            wxImage img(bitmap.ConvertToImage());
            if ( img.HasAlpha() )
            {
                img.ConvertAlphaToMask();
                bitmapWithMask = wxBitmap(img);
                pMask = bitmapWithMask.GetMask();
            }
        }
#endif // wxUSE_IMAGE

        if ( !pMask )
        {
            // use the light grey count as transparent: the trouble here is
            // that the light grey might have been changed by Windows behind
            // our back, so use the standard colour map to get its real value
            wxCOLORMAP *cmap = wxGetStdColourMap();
            wxColour col;
            wxRGBToColour(col, cmap[wxSTD_COL_BTNFACE].from);

            pMask = new wxMask(bitmap, col);

            deleteMask = true;
        }

        hbmpMask = (WXHBITMAP)pMask->GetMaskBitmap();
    }

    // windows mask convention is opposite to the wxWidgets one
    WXHBITMAP hbmpMaskInv = wxInvertMask(hbmpMask);

    if ( deleteMask )
    {
        delete pMask;
    }

    return hbmpMaskInv;
}
