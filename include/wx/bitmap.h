///////////////////////////////////////////////////////////////////////////////
// Name:        wx/bitmap.h
// Purpose:     wxBitmap class interface
// Author:      Vaclav Slavik
// Modified by:
// Created:     22.04.01
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BITMAP_H_BASE_
#define _WX_BITMAP_H_BASE_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/gdicmn.h"  // for wxBitmapType
#include "wx/colour.h"
#include "wx/image.h"

#include <string>

class wxBitmap;
struct wxBitmapHandler;
class wxIcon;
class wxMask;
class wxPalette;
class wxDC;

// ----------------------------------------------------------------------------
// wxVariant support
// ----------------------------------------------------------------------------

#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT_EXPORTED(wxBitmap)
#endif

// ----------------------------------------------------------------------------
// wxMask represents the transparent area of the bitmap
// ----------------------------------------------------------------------------

// TODO: all implementation of wxMask, except the generic one,
//       do not derive from wxMaskBase,,, they should
class wxMaskBase
{
public:
    // create the mask from bitmap pixels of the given colour
    bool Create(const wxBitmap& bitmap, const wxColour& colour);

#if wxUSE_PALETTE
    // create the mask from bitmap pixels with the given palette index
    bool Create(const wxBitmap& bitmap, int paletteIndex);
#endif // wxUSE_PALETTE

    // create the mask from the given mono bitmap
    bool Create(const wxBitmap& bitmap);

protected:
    // this function is called from Create() to free the existing mask data
    virtual void FreeData() = 0;

    // these functions must be overridden to implement the corresponding public
    // Create() methods, they shouldn't call FreeData() as it's already called
    // by the public wrappers
    virtual bool InitFromColour(const wxBitmap& bitmap,
                                const wxColour& colour) = 0;
    virtual bool InitFromMonoBitmap(const wxBitmap& bitmap) = 0;
};

#if defined(__WXDFB__) || \
    defined(__WXMAC__) || \
    defined(__WXGTK__) || \
    defined(__WXMOTIF__) || \
    defined(__WXX11__) || \
    defined(__WXQT__)
    #define wxUSE_BITMAP_BASE 1
#else
    #define wxUSE_BITMAP_BASE 0
#endif

// a more readable way to tell
#define wxBITMAP_SCREEN_DEPTH       (-1)


// ----------------------------------------------------------------------------
// wxBitmapHelpers: container for various bitmap methods common to all ports.
// ----------------------------------------------------------------------------

// Unfortunately, currently wxBitmap does not inherit from wxBitmapBase on all
// platforms and this is not easy to fix. So we extract at least some common
// methods into this class from which both wxBitmapBase (and hence wxBitmap on
// all platforms where it does inherit from it) and wxBitmap in wxMSW and other
// exceptional ports (only wxPM and old wxCocoa) inherit.
class wxBitmapHelpers
{
public:
    // Create a new wxBitmap from the PNG data in the given buffer.
    static wxBitmap NewFromPNGData(const void* data, size_t size);
};


// All ports except wxMSW use wxBitmapHandler and wxBitmapBase as
// base class for wxBitmapHandler; wxMSW uses wxGDIImageHandler as
// base class since it allows some code reuse there.
#if wxUSE_BITMAP_BASE

// ----------------------------------------------------------------------------
// wxBitmapHandler: class which knows how to create/load/save bitmaps in
// different formats
// ----------------------------------------------------------------------------

class wxBitmapHandler : public wxObject
{
public:
    wxBitmapHandler() { m_type = wxBitmapType::Invalid; }
    virtual ~wxBitmapHandler() { }

    // NOTE: the following functions should be pure virtuals, but they aren't
    //       because otherwise almost all ports would have to implement
    //       them as "return false"...

    virtual bool Create(wxBitmap *WXUNUSED(bitmap), const void* WXUNUSED(data),
                         wxBitmapType WXUNUSED(type), int WXUNUSED(width), int WXUNUSED(height),
                         int WXUNUSED(depth) = 1)
        { return false; }

    virtual bool LoadFile(wxBitmap *WXUNUSED(bitmap), const std::string& WXUNUSED(name),
                           wxBitmapType WXUNUSED(type), int WXUNUSED(desiredWidth),
                           int WXUNUSED(desiredHeight))
        { return false; }

    virtual bool SaveFile(const wxBitmap *WXUNUSED(bitmap), const std::string& WXUNUSED(name),
                           wxBitmapType WXUNUSED(type), const wxPalette *WXUNUSED(palette) = NULL) const
        { return false; }

    void SetName(const std::string& name)      { m_name = name; }
    void SetExtension(const std::string& ext)  { m_extension = ext; }
    void SetType(wxBitmapType type)         { m_type = type; }
    const std::string& GetName() const         { return m_name; }
    const std::string& GetExtension() const    { return m_extension; }
    wxBitmapType GetType() const            { return m_type; }

private:
    std::string      m_name;
    std::string      m_extension;
    wxBitmapType  m_type;

    wxDECLARE_ABSTRACT_CLASS(wxBitmapHandler);
};

// ----------------------------------------------------------------------------
// wxBitmap: class which represents platform-dependent bitmap (unlike wxImage)
// ----------------------------------------------------------------------------

class wxBitmapBase : public wxGDIObject,
                                      public wxBitmapHelpers
{
public:
    /*
    Derived class must implement these:

    wxBitmap();
    wxBitmap(const wxBitmap& bmp);
    wxBitmap(const char bits[], int width, int height, int depth = 1);
    wxBitmap(int width, int height, int depth = wxBITMAP_SCREEN_DEPTH);
    wxBitmap(const wxSize& sz, int depth = wxBITMAP_SCREEN_DEPTH);
    wxBitmap(const char* const* bits);
    wxBitmap(const std::string &filename, wxBitmapType type = wxBitmapType::XPM);
    wxBitmap(const wxImage& image, int depth = wxBITMAP_SCREEN_DEPTH, double scale = 1.0);

    static void InitStandardHandlers();
    */

    virtual bool Create(int width, int height, int depth = wxBITMAP_SCREEN_DEPTH) = 0;
    virtual bool Create(const wxSize& sz, int depth = wxBITMAP_SCREEN_DEPTH) = 0;
    virtual bool CreateScaled(int w, int h, int d, double logicalScale)
        { return Create(std::lround(w*logicalScale), std::lround(h*logicalScale), d); }

    virtual int GetHeight() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetDepth() const = 0;

    wxSize GetSize() const
        { return wxSize(GetWidth(), GetHeight()); }

    // support for scaled bitmaps
    virtual double GetScaleFactor() const { return 1.0; }
    virtual double GetScaledWidth() const { return GetWidth() / GetScaleFactor(); }
    virtual double GetScaledHeight() const { return GetHeight() / GetScaleFactor(); }
    virtual wxSize GetScaledSize() const
        { return wxSize(std::lround(GetScaledWidth()), std::lround(GetScaledHeight())); }

#if wxUSE_IMAGE
    virtual wxImage ConvertToImage() const = 0;

    // Convert to disabled (dimmed) bitmap.
    wxBitmap ConvertToDisabled(unsigned char brightness = 255) const;
#endif // wxUSE_IMAGE

    virtual wxMask *GetMask() const = 0;
    virtual void SetMask(wxMask *mask) = 0;

    virtual wxBitmap GetSubBitmap(const wxRect& rect) const = 0;

    virtual bool SaveFile(const std::string& name, wxBitmapType type,
                          const wxPalette *palette = NULL) const = 0;
    virtual bool LoadFile(const std::string& name, wxBitmapType type) = 0;

    /*
       If raw bitmap access is supported (see wx/rawbmp.h), the following
       methods should be implemented:

       virtual bool GetRawData(wxRawBitmapData *data) = 0;
       virtual void UngetRawData(wxRawBitmapData *data) = 0;
     */

#if wxUSE_PALETTE
    virtual wxPalette *GetPalette() const = 0;
    virtual void SetPalette(const wxPalette& palette) = 0;
#endif // wxUSE_PALETTE

    // copies the contents and mask of the given (colour) icon to the bitmap
    virtual bool CopyFromIcon(const wxIcon& icon) = 0;

    // Format handling
    static inline wxList& GetHandlers() { return sm_handlers; }
    static void AddHandler(wxBitmapHandler *handler);
    static void InsertHandler(wxBitmapHandler *handler);
    static bool RemoveHandler(const std::string& name);
    static wxBitmapHandler *FindHandler(const std::string& name);
    static wxBitmapHandler *FindHandler(const std::string& extension, wxBitmapType bitmapType);
    static wxBitmapHandler *FindHandler(wxBitmapType bitmapType);

    //static void InitStandardHandlers();
    //  (wxBitmap must implement this one)

    static void CleanUpHandlers();

    // this method is only used by the generic implementation of wxMask
    // currently but could be useful elsewhere in the future: it can be
    // overridden to quantize the colour to correspond to bitmap colour depth
    // if necessary; default implementation simply returns the colour as is
    virtual wxColour QuantizeColour(const wxColour& colour) const
    {
        return colour;
    }

protected:
    inline static wxList sm_handlers;

    wxDECLARE_ABSTRACT_CLASS(wxBitmapBase);
};

#endif // wxUSE_BITMAP_BASE


// the wxBITMAP_DEFAULT_TYPE constant defines the default argument value
// for wxBitmap's ctor and wxBitmap::LoadFile() functions.
#if defined(__WXMSW__)
    #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::BMP_Resource
    #include "wx/msw/bitmap.h"
#elif defined(__WXMOTIF__)
    #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::XPM
    #include "wx/x11/bitmap.h"
#elif defined(__WXGTK20__)
    #ifdef WX_WINDOWS
        #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::BMP_Resource
    #else
        #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::XPM
    #endif
    #include "wx/gtk/bitmap.h"
#elif defined(__WXGTK__)
    #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::XPM
    #include "wx/gtk1/bitmap.h"
#elif defined(__WXX11__)
    #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::XPM
    #include "wx/x11/bitmap.h"
#elif defined(__WXDFB__)
    #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::BMP_Resource
    #include "wx/dfb/bitmap.h"
#elif defined(__WXMAC__)
    #define wxBITMAP_DEFAULT_TYPE    wxBITMAP_TYPE_PICT_RESOURCE
    #include "wx/osx/bitmap.h"
#elif defined(__WXQT__)
    #define wxBITMAP_DEFAULT_TYPE    wxBitmapType::XPM
    #include "wx/qt/bitmap.h"
#endif

#if wxUSE_IMAGE
inline
wxBitmap
#if wxUSE_BITMAP_BASE
wxBitmapBase::
#else
wxBitmap::
#endif
ConvertToDisabled(unsigned char brightness) const
{
    const wxImage imgDisabled = ConvertToImage().ConvertToDisabled(brightness);
    return wxBitmap(imgDisabled, -1, GetScaleFactor());
}
#endif // wxUSE_IMAGE

// we must include generic mask.h after wxBitmap definition
#if defined(__WXDFB__)
    #define wxUSE_GENERIC_MASK 1
#else
    #define wxUSE_GENERIC_MASK 0
#endif

#if wxUSE_GENERIC_MASK
    #include "wx/generic/mask.h"
#endif

#endif // _WX_BITMAP_H_BASE_
