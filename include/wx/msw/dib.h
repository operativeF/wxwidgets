/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dib.h
// Purpose:     wxDIB class representing Win32 device independent bitmaps
// Author:      Vadim Zeitlin
// Modified by:
// Created:     03.03.03 (replaces the old file with the same name)
// Copyright:   (c) 1997-2003 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DIB_H_
#define _WX_MSW_DIB_H_

class wxPalette;

#include "wx/msw/private.h"

#if wxUSE_WXDIB

#ifdef __WXMSW__
    #include "wx/bitmap.h"
#endif // __WXMSW__

import WX.WinDef;

import <string>;

// ----------------------------------------------------------------------------
// wxDIB: represents a DIB section
// ----------------------------------------------------------------------------

class wxDIB
{
public:
    // create an uninitialized DIB with the given width, height and depth (only
    // 24 and 32 bpp DIBs are currently supported)
    //
    // after using this ctor, GetData() and GetHandle() may be used if IsOk()
    // returns true
    wxDIB(wxSize sz, int depth)
    { 
        Create(sz, depth);
    }

#ifdef __WXMSW__
    // create a DIB from the DDB
    wxDIB(const wxBitmap& bmp, int depth = -1)
    { 
        Create(bmp, depth);
    }
#endif // __WXMSW__

    // create a DIB from the Windows DDB
    wxDIB(WXHBITMAP hbmp)
    {
        Create(hbmp);
    }

    // load a DIB from file (any depth is supoprted here unlike above)
    //
    // as above, use IsOk() to see if the bitmap was loaded successfully
    wxDIB(const std::string& filename)
    { 
        Load(filename);
    }

    // DIBs can't be copied
    // FIXME: Can DIBs be moved?
    wxDIB& operator=(wxDIB&&) = delete;

    // same as the corresponding ctors but with return value
    [[maybe_unused]] bool Create(wxSize sz, int depth);
#ifdef __WXMSW__
    [[maybe_unused]] bool Create(const wxBitmap& bmp, int depth = -1) { return Create(GetHbitmapOf(bmp), depth); }
#endif
    [[maybe_unused]] bool Create(WXHBITMAP hbmp, int depth = -1);
    bool Load(const std::string& filename);

    ~wxDIB();

    // operations
    // ----------

    // create a bitmap compatible with the given WXHDC (or screen by default) and
    // return its handle, the caller is responsible for freeing it (using
    // DeleteObject())
    WXHBITMAP CreateDDB(WXHDC hdc = nullptr) const;

    // get the handle from the DIB and reset it, i.e. this object won't destroy
    // the DIB after this (but the caller should do it)
    WXHBITMAP Detach() { WXHBITMAP hbmp = m_handle; m_handle = nullptr; return hbmp; }

#if defined(__WXMSW__) && wxUSE_PALETTE
    // create a palette for this DIB (always a trivial/default one for 24bpp)
    wxPalette *CreatePalette() const;
#endif // defined(__WXMSW__) && wxUSE_PALETTE

    // save the DIB as a .BMP file to the file with the given name
    bool Save(const std::string& filename);

    // return true if DIB was successfully created, false otherwise
    bool IsOk() const { return m_handle != nullptr; }

    // get the bitmap size
    wxSize GetSize() const { DoGetObject(); return m_size; }
    int GetWidth() const { DoGetObject(); return m_size.x; }
    int GetHeight() const { DoGetObject(); return m_size.y; }

    // get the number of bits per pixel, or depth
    int GetDepth() const { DoGetObject(); return m_depth; }

    // get the DIB handle
    WXHBITMAP GetHandle() const { return m_handle; }

    // get raw pointer to bitmap bits, you should know what you do if you
    // decide to use it
    unsigned char *GetData() const
        { DoGetObject(); return (unsigned char *)m_data; }


    // WXHBITMAP conversion
    // ------------------

    // these functions are only used by wxWidgets internally right now, please
    // don't use them directly if possible as they're subject to change

    // creates a DDB compatible with the given (or screen) DC from either
    // a plain DIB or a DIB section (in which case the last parameter must be
    // non NULL)
    static WXHBITMAP ConvertToBitmap(const BITMAPINFO *pbi,
                                   WXHDC hdc = nullptr,
                                   const void *bits = nullptr);

    // create a plain DIB (not a DIB section) from a DDB, the caller is
    // responsible for freeing it using ::GlobalFree()
    static HGLOBAL ConvertFromBitmap(WXHBITMAP hbmp);

    // creates a DIB from the given DDB or calculates the space needed by it:
    // if pbi is NULL, only the space is calculated, otherwise pbi is supposed
    // to point at BITMAPINFO of the correct size which is filled by this
    // function (this overload is needed for wxBitmapDataObject code in
    // src/msw/ole/dataobj.cpp)
    static size_t ConvertFromBitmap(BITMAPINFO *pbi, WXHBITMAP hbmp);


    // wxImage conversion
    // ------------------

#if wxUSE_IMAGE
    // Possible formats for DIBs created by the functions below.
    enum class PixelFormat
    {
        PreMultiplied,
        NotPreMultiplied
    };

    // Create a DIB from the given image, the DIB will be either 24 or 32 (if
    // the image has alpha channel) bpp.
    //
    // By default the DIB stores pixel data in pre-multiplied format so that it
    // can be used with ::AlphaBlend() but it is also possible to disable
    // pre-multiplication for the DIB to be usable with ImageList_Draw() which
    // does pre-multiplication internally.
    wxDIB(const wxImage& image, PixelFormat pf = PixelFormat::PreMultiplied, int depth = -1)
    {
        Create(image, pf, depth);
    }
    
    // same as the above ctor but with the return code
    [[maybe_unused]] bool Create(const wxImage& image, PixelFormat pf = PixelFormat::PreMultiplied, int depth = -1);

    // create wxImage having the same data as this DIB

    // Possible options of conversion to wxImage
    enum class ConversionFlags
    {
        // Determine whether 32bpp DIB contains real alpha channel
        // and return wxImage with or without alpha channel values.
        AlphaAuto,
        // Assume that 32bpp DIB contains valid alpha channel and always
        // return wxImage with alpha channel values in this case.
        AlphaAlwaysIf32bpp
    };
    wxImage ConvertToImage(ConversionFlags flags = ConversionFlags::AlphaAuto) const;
#endif // wxUSE_IMAGE


    // helper functions
    // ----------------

    // return the size of one line in a DIB with given width and depth: the
    // point here is that as the scan lines need to be WXDWORD aligned so we may
    // need to add some padding
    static unsigned long GetLineSize(int width, int depth)
    {
        return ((width*depth + 31) & ~31) >> 3;
    }

private:    
    // free resources
    void Free();

    // initialize the contents from the provided DDB (Create() must have been
    // already called)
    bool CopyFromDDB(WXHBITMAP hbmp);


    // the DIB section handle, 0 if invalid
    WXHBITMAP m_handle{nullptr};

    // NB: we could store only m_handle and not any of the other fields as
    //     we may always retrieve them from it using GetObjectW(), but we
    //     decide to still store them for efficiency concerns -- however if we
    //     don't have them from the very beginning (e.g. DIB constructed from a
    //     bitmap), we only retrieve them when necessary and so these fields
    //     should *never* be accessed directly, even from inside wxDIB code

    // function which must be called before accessing any members and which
    // gets their values from m_handle, if not done yet
    void DoGetObject() const;

    // pointer to DIB bits, may be NULL
    void *m_data{nullptr};

    // size and depth of the image
    wxSize m_size{0, 0};
    int m_depth{0};

    // in some cases we could be using a handle which we didn't create and in
    // this case we shouldn't free it neither -- this flag tell us if this is
    // the case
    bool m_ownsHandle{true};
};

// ----------------------------------------------------------------------------
// inline functions implementation
// ----------------------------------------------------------------------------



inline
void wxDIB::Free()
{
    if ( m_handle && m_ownsHandle )
    {
        if ( !::DeleteObject(m_handle) )
        {
            wxLogLastError("DeleteObject(hDIB)");
        }

        m_handle = nullptr;
        m_ownsHandle = true;

        m_data = nullptr;

        m_size = {0, 0};
        m_depth = 0;
    }
}

inline wxDIB::~wxDIB()
{
    Free();
}

#endif
    // wxUSE_WXDIB

#endif // _WX_MSW_DIB_H_

