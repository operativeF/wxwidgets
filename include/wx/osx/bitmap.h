/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/bitmap.h
// Purpose:     wxBitmap class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BITMAP_H_
#define _WX_BITMAP_H_

#include "wx/palette.h"

// Bitmap
class WXDLLIMPEXP_FWD_CORE wxBitmap;
class wxBitmapRefData ;
class WXDLLIMPEXP_FWD_CORE wxBitmapHandler;
class WXDLLIMPEXP_FWD_CORE wxControl;
class WXDLLIMPEXP_FWD_CORE wxCursor;
class WXDLLIMPEXP_FWD_CORE wxDC;
class WXDLLIMPEXP_FWD_CORE wxIcon;
class WXDLLIMPEXP_FWD_CORE wxImage;
class WXDLLIMPEXP_FWD_CORE wxPixelDataBase;

// A mask is a bitmap used for drawing bitmaps
// Internally it is stored as a 8 bit deep memory chunk, 0 = black means the source will be drawn
// 255 = white means the source will not be drawn, no other values will be present
// 8 bit is chosen only for performance reasons, note also that this is the inverse value range
// from alpha, where 0 = invisible , 255 = fully drawn

class wxMask: public wxMaskBase
{
    wxDECLARE_DYNAMIC_CLASS(wxMask);

public:
    wxMask();

    // Copy constructor
    wxMask(const wxMask& mask);

    // Construct a mask from a bitmap and a colour indicating
    // the transparent area
    wxMask(const wxBitmap& bitmap, const wxColour& colour);

    // Construct a mask from a mono bitmap (black meaning show pixels, white meaning transparent)
    wxMask(const wxBitmap& bitmap);

    virtual ~wxMask();

    wxBitmap GetBitmap() const;

    // Implementation below

    void Init() ;

    // a 8 bit depth mask
    void* GetRawAccess() const;
    int GetBytesPerRow() const;
    int GetWidth() const;
    int GetHeight() const;

    // renders/updates native representation when necessary
    void RealizeNative() ;

    WXHBITMAP GetHBITMAP() const ;

    // implementation helper only : construct a mask from a 8 bpp memory buffer
    bool OSXCreate(const wxMemoryBuffer& buf, int width , int height , int bytesPerRow ) ;

protected:
    // this function is called from Create() to free the existing mask data
    void FreeData() override;

    // these functions must be overridden to implement the corresponding public
    // Create() methods, they shouldn't call FreeData() as it's already called
    // by the public wrappers
    virtual bool InitFromColour(const wxBitmap& bitmap,
                                const wxColour& colour) override;
    bool InitFromMonoBitmap(const wxBitmap& bitmap) override;

private:
    void DoCreateMaskBitmap(int width, int height, int bytesPerRow = -1);

    wxCFRef<CGContextRef> m_maskBitmap ;

};

class wxBitmap: public wxBitmapBase
{
    wxDECLARE_DYNAMIC_CLASS(wxBitmap);

    friend class WXDLLIMPEXP_FWD_CORE wxBitmapHandler;

public:
    wxBitmap() {} // Platform-specific

    // Initialize with raw data.
    wxBitmap(const char bits[], int width, int height, int depth = 1);

    // Initialize with XPM data
    wxBitmap(const char* const* bits);

    // Load a file or resource
    wxBitmap(const wxString& name, wxBitmapType type = wxBITMAP_DEFAULT_TYPE);

    // Constructor for generalised creation from data
    wxBitmap(const void* data, wxBitmapType type, int width, int height, int depth = 1);

    // creates an bitmap from the native image format
    wxBitmap(CGImageRef image, double scale = 1.0);
    wxBitmap(WXImage image);
    wxBitmap(CGContextRef bitmapcontext);

    // Create a bitmap compatible with the given DC
    wxBitmap(int width, int height, const wxDC& dc);

    // If depth is omitted, will create a bitmap compatible with the display
    wxBitmap(int width, int height, int depth = -1) { (void)Create(width, height, depth); }
    wxBitmap(const wxSize& sz, int depth = -1) { (void)Create(sz, depth); }

    // Convert from wxImage:
    wxBitmap(const wxImage& image, int depth = -1, double scale = 1.0);

    // Convert from wxIcon
    wxBitmap(const wxIcon& icon) { CopyFromIcon(icon); }

#if wxOSX_USE_COCOA
    // Convert from wxCursor
    wxBitmap(const wxCursor &cursor);
#endif

    virtual ~wxBitmap() {}

    wxImage ConvertToImage() const override;

    // get the given part of bitmap
    wxBitmap GetSubBitmap( const wxRect& rect ) const override;

    bool Create(int width, int height, int depth = wxBITMAP_SCREEN_DEPTH) override;
    bool Create(const wxSize& sz, int depth = wxBITMAP_SCREEN_DEPTH) override
        { return Create(sz.GetWidth(), sz.GetHeight(), depth); }

    virtual bool Create(const void* data, wxBitmapType type, int width, int height, int depth = 1);
    bool Create( CGImageRef image, double scale = 1.0 );
    bool Create( WXImage image );
    bool Create( CGContextRef bitmapcontext);

    // Create a bitmap compatible with the given DC, inheriting its magnification factor
    bool Create(int width, int height, const wxDC& dc);

    // Create a bitmap with a scale factor, width and height are multiplied with that factor
    bool CreateScaled(int logwidth, int logheight, int depth, double logicalScale) override;

    // virtual bool Create( WXHICON icon) ;
    bool LoadFile(const wxString& name, wxBitmapType type = wxBITMAP_DEFAULT_TYPE) override;
    bool SaveFile(const wxString& name, wxBitmapType type, const wxPalette *cmap = NULL) const override;

    const wxBitmapRefData *GetBitmapData() const
        { return (const wxBitmapRefData *)m_refData; }

    wxBitmapRefData *GetBitmapData()
        { return (wxBitmapRefData *)m_refData; }

    // copies the contents and mask of the given (colour) icon to the bitmap
    bool CopyFromIcon(const wxIcon& icon) override;

    int GetWidth() const override;
    int GetHeight() const override;
    int GetDepth() const override;

#if wxUSE_PALETTE
    wxPalette* GetPalette() const override;
    void SetPalette(const wxPalette& palette) override;
#endif // wxUSE_PALETTE

    wxMask *GetMask() const override;
    void SetMask(wxMask *mask) override;

    static void InitStandardHandlers();

    // raw bitmap access support functions, for internal use only
    void *GetRawData(wxPixelDataBase& data, int bpp);
    void UngetRawData(wxPixelDataBase& data);

    // these functions are internal and shouldn't be used, they risk to
    // disappear in the future
    bool HasAlpha() const;
    void UseAlpha(bool use = true);

    // returns the 'native' implementation, a GWorldPtr for the content and one for the mask
    WXHBITMAP GetHBITMAP( WXHBITMAP * mask = NULL ) const;

    // returns a CGImageRef which must released after usage with CGImageRelease
    CGImageRef CreateCGImage() const ;

    WXImage GetImage() const;
#if wxOSX_USE_COCOA
    // returns an autoreleased version of the image
    WX_NSImage GetNSImage() const
        { return GetImage(); }
#endif
#if wxOSX_USE_IPHONE
    // returns an autoreleased version of the image
    WX_UIImage GetUIImage() const
        { return GetImage(); }
#endif

    double GetScaleFactor() const override;

    void SetSelectedInto(wxDC *dc);
    wxDC *GetSelectedInto() const;

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;
};

#endif // _WX_BITMAP_H_
