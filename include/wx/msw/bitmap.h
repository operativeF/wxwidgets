/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/bitmap.h
// Purpose:     wxBitmap class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BITMAP_H_
#define _WX_BITMAP_H_

#include "wx/msw/gdiimage.h"
#include "wx/palette.h"
#include "wx/icon.h"

import WX.Win.UniqueHnd;
import Utils.Geometry;

import <cmath>;
import <string>;

class wxBitmap;
struct wxBitmapHandler; // FIXME: uses in class in non-MSW
class wxBitmapRefData;
class wxControl;
class wxCursor;
class wxDC;
#if wxUSE_WXDIB
class wxDIB;
#endif
class wxMask;
class wxPalette;
class wxPixelDataBase;

// What kind of transparency should a bitmap copied from an icon or cursor
// have?
enum class wxBitmapTransparency
{
  Auto,    // default: copy alpha if the source has it
  None,    // never create alpha
  Always   // always use alpha
};

// ----------------------------------------------------------------------------
// wxBitmap: a mono or colour bitmap
// NOTE: for wxMSW we don't use the wxBitmapBase base class declared in bitmap.h!
// ----------------------------------------------------------------------------

class wxBitmap : public wxGDIImage,
                 public wxBitmapHelpers
{
public:
    // default ctor creates an invalid bitmap, you must Create() it later
    wxBitmap() = default;

    // Initialize with raw data
    wxBitmap(const char bits[], wxSize sz, int depth = 1);

    // Initialize with XPM data
    explicit wxBitmap(const char* const* data);

    // Load a file or resource
    wxBitmap(const std::string& name, wxBitmapType type = wxBITMAP_DEFAULT_TYPE);

    // New constructor for generalised creation from data
    wxBitmap(const void* data, wxBitmapType type, wxSize sz, int depth = 1);

    // Create a new, uninitialized bitmap of the given size and depth (if it
    // is omitted, will create a bitmap compatible with the display)
    //
    // NB: this ctor will create a DIB for 24 and 32bpp bitmaps, use ctor
    //     taking a DC argument if you want to force using DDB in this case
    wxBitmap(wxSize sz, int depth = -1) { Create(sz, depth); }

    // Create a bitmap compatible with the given DC
    wxBitmap(wxSize sz, const wxDC& dc);

#if wxUSE_IMAGE
    // Convert from wxImage
    wxBitmap(const wxImage& image, int depth = -1, [[maybe_unused]] double scale = 1.0)
        { CreateFromImage(image, depth); }

    // Create a DDB compatible with the given DC from wxImage
    wxBitmap(const wxImage& image, const wxDC& dc)
        { CreateFromImage(image, dc); }
#endif // wxUSE_IMAGE

    // we must have this, otherwise icons are silently copied into bitmaps using
    // the copy ctor but the resulting bitmap is invalid!
    wxBitmap(const wxIcon& icon,
             wxBitmapTransparency transp = wxBitmapTransparency::Auto)
    {
        CopyFromIcon(icon, transp);
    }

    // Convert from wxCursor
    explicit wxBitmap(const wxCursor& cursor)
    {
        CopyFromCursor(cursor, wxBitmapTransparency::Auto);
    }

#if wxUSE_IMAGE
    wxBitmap& operator=(const wxImage& image)
    {
        return *this = wxBitmap(image);
    }
#endif // wxUSE_IMAGE

    wxBitmap& operator=(const wxIcon& icon)
    {
        CopyFromIcon(icon);

        return *this;
    }

#if wxUSE_IMAGE
    wxImage ConvertToImage() const;
    wxBitmap ConvertToDisabled(unsigned char brightness = 255) const;
#endif // wxUSE_IMAGE

    // get the given part of bitmap
    wxBitmap GetSubBitmap( const wxRect& rect ) const;

    // NB: This should not be called from user code. It is for wx internal
    // use only.
    wxBitmap GetSubBitmapOfHDC( const wxRect& rect, WXHDC hdc ) const;

    // copies the contents and mask of the given (colour) icon to the bitmap
    [[maybe_unused]] bool CopyFromIcon(const wxIcon& icon,
                      wxBitmapTransparency transp = wxBitmapTransparency::Auto);

    // copies the contents and mask of the given cursor to the bitmap
    [[maybe_unused]] bool CopyFromCursor(const wxCursor& cursor,
                        wxBitmapTransparency transp = wxBitmapTransparency::Auto);

#if wxUSE_WXDIB
    // copies from a device independent bitmap
    bool CopyFromDIB(const wxDIB& dib);
    bool IsDIB() const;
    bool ConvertToDIB();
#endif

    [[maybe_unused]] virtual bool Create(wxSize sz, int depth = wxBITMAP_SCREEN_DEPTH);
    [[maybe_unused]] virtual bool Create(wxSize sz, const wxDC& dc);
    [[maybe_unused]] virtual bool Create(const void* data, wxBitmapType type, wxSize sz, int depth = 1);
    virtual bool CreateScaled(int w, int h, int d, double logicalScale)
        { return Create(wxSize{std::lround(w*logicalScale), std::lround(h*logicalScale)}, d); }

    virtual bool LoadFile(const std::string& name, wxBitmapType type = wxBITMAP_DEFAULT_TYPE);
    virtual bool SaveFile(const std::string& name, wxBitmapType type, const wxPalette *cmap = nullptr) const;

    wxBitmapRefData *GetBitmapData() const
        { return (wxBitmapRefData *)m_refData; }

    // raw bitmap access support functions
    void *GetRawData(wxPixelDataBase& data, int bpp);
    void UngetRawData(wxPixelDataBase& data);

#if wxUSE_PALETTE
    wxPalette* GetPalette() const;
    void SetPalette(const wxPalette& palette);
#endif // wxUSE_PALETTE

    wxMask *GetMask() const;
    void SetMask(wxMask *mask);

    // these functions are internal and shouldn't be used, they risk to
    // disappear in the future
    bool HasAlpha() const;
    void UseAlpha(bool use = true);
    void ResetAlpha() { UseAlpha(false); }

    // support for scaled bitmaps
    virtual double GetScaleFactor() const { return 1.0; }
    virtual double GetScaledWidth() const { return GetWidth() / GetScaleFactor(); }
    virtual double GetScaledHeight() const { return GetHeight() / GetScaleFactor(); }
    virtual wxSize GetScaledSize() const
        { return wxSize(std::lround(GetScaledWidth()), std::lround(GetScaledHeight())); }

    // implementation only from now on
    // -------------------------------

    // Set alpha flag to true if this is a 32bpp bitmap which has any non-0
    // values in its alpha channel.
    void MSWUpdateAlpha();

    // Blend mask with alpha channel and remove the mask
    void MSWBlendMaskWithAlpha();

    WXHBITMAP GetHBITMAP() const { return (WXHBITMAP)GetHandle(); }
    bool InitFromHBITMAP(WXHBITMAP bmp, wxSize sz, int depth);
    void ResetHBITMAP() { InitFromHBITMAP(nullptr, wxSize{0, 0}, 0); }

    void SetSelectedInto(wxDC *dc);
    wxDC *GetSelectedInto() const;

protected:
    wxGDIImageRefData *CreateData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

    // creates an uninitialized bitmap, called from Create()s above
    bool DoCreate(wxSize sz, int depth, WXHDC hdc);

#if wxUSE_IMAGE
    // creates the bitmap from wxImage, supposed to be called from ctor
    [[maybe_unused]] bool CreateFromImage(const wxImage& image, int depth);

    // creates a DDB from wxImage, supposed to be called from ctor
    [[maybe_unused]] bool CreateFromImage(const wxImage& image, const wxDC& dc);

    // common part of the 2 methods above (hdc may be 0)
    [[maybe_unused]] bool CreateFromImage(const wxImage& image, int depth, WXHDC hdc);
#endif // wxUSE_IMAGE

private:
    // common part of CopyFromIcon/CopyFromCursor for Win32
    bool
    CopyFromIconOrCursor(const wxGDIImage& icon,
                         wxBitmapTransparency transp = wxBitmapTransparency::Auto);
};

// ----------------------------------------------------------------------------
// wxMask: a mono bitmap used for drawing bitmaps transparently.
// ----------------------------------------------------------------------------

using msw::utils::unique_bitmap;

class wxMask
{
public:
    wxMask();

    // Copy constructor
    wxMask(const wxMask &mask);

    // Construct a mask from a bitmap and a colour indicating the transparent
    // area
    wxMask(const wxBitmap& bitmap, const wxColour& colour);

    // Construct a mask from a bitmap and a palette index indicating the
    // transparent area
    wxMask(const wxBitmap& bitmap, int paletteIndex);

    // Construct a mask from a mono bitmap (copies the bitmap).
    wxMask(const wxBitmap& bitmap);

    // construct a mask from the givne bitmap handle
    wxMask(WXHBITMAP hbmp) { m_maskBitmap = unique_bitmap{hbmp}; }

    [[maybe_unused]] bool Create(const wxBitmap& bitmap, const wxColour& colour);
    [[maybe_unused]] bool Create(const wxBitmap& bitmap, int paletteIndex);
    [[maybe_unused]] bool Create(const wxBitmap& bitmap);

    wxBitmap GetBitmap() const;

    // Implementation
    WXHBITMAP GetMaskBitmap() const { return m_maskBitmap.get(); }
    void SetMaskBitmap(WXHBITMAP bmp) { m_maskBitmap.reset(bmp); }

protected:
    unique_bitmap m_maskBitmap;
};


// ----------------------------------------------------------------------------
// wxBitmapHandler is a class which knows how to load/save bitmaps to/from file
// NOTE: for wxMSW we don't use the wxBitmapHandler class declared in bitmap.h!
// ----------------------------------------------------------------------------

struct wxBitmapHandler : public wxGDIImageHandler
{
    wxBitmapHandler() = default;
    wxBitmapHandler(const std::string& name, const std::string& ext, wxBitmapType type)
        : wxGDIImageHandler(name, ext, type) { }

    // implement wxGDIImageHandler's pure virtuals:

    [[maybe_unused]] bool Create(wxGDIImage *image,
                        const void* data,
                        wxBitmapType type,
                        wxSize sz, int depth = 1) override;
    bool Load(wxGDIImage *image,
                      const std::string& name,
                      wxBitmapType type,
                      wxSize desiredSz) override;
    bool Save(const wxGDIImage *image,
                      const std::string& name,
                      wxBitmapType type) const override;


    // make wxBitmapHandler compatible with the wxBitmapHandler interface
    // declared in bitmap.h, even if it's derived from wxGDIImageHandler:

    [[maybe_unused]] virtual bool Create(wxBitmap *bitmap,
                        const void* data,
                        wxBitmapType type,
                        wxSize sz, int depth = 1);
    virtual bool LoadFile(wxBitmap *bitmap,
                          const std::string& name,
                          wxBitmapType type,
                          wxSize desiredSz);
    virtual bool SaveFile(const wxBitmap *bitmap,
                          const std::string& name,
                          wxBitmapType type,
                          const wxPalette *palette = nullptr) const;
};

#endif
  // _WX_BITMAP_H_
