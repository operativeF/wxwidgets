/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/image.cpp
// Purpose:     wxImage
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/log.h"
#include "wx/hash.h"
#include "wx/utils.h"
#include "wx/module.h"
#include "wx/palette.h"
#include "wx/intl.h"
#include "wx/colour.h"

#include "wx/wfstream.h"
#include "wx/xpmdecod.h"

// Under Windows we can load wxImage not only from files but also from
// resources.
#if defined(WX_WINDOWS) && wxUSE_WXDIB && wxUSE_IMAGE \
&& !defined(__WXQT__) // undefined reference to `wxDIB::ConvertToImage(wxDIB::ConversionFlags) const'
    #define HAS_LOAD_FROM_RESOURCE
#endif

#ifdef HAS_LOAD_FROM_RESOURCE
    #include "wx/msw/dib.h"
    #include "wx/msw/private.h"
#endif

module WX.Image.Base;

import WX.WinDef;

import Utils.Strings;

import WX.Utils.Cast;

import <algorithm>;
import <cmath>;
import <cstring>; // For memcpy

#if wxUSE_IMAGE

// make the code compile with either wxFile*Stream or wxFFile*Stream:
#define HAS_FILE_STREAMS (wxUSE_STREAMS && (wxUSE_FILE || wxUSE_FFILE))

#if HAS_FILE_STREAMS
    #if wxUSE_FFILE
        using wxImageFileInputStream = wxFFileInputStream;
        using wxImageFileOutputStream = wxFFileOutputStream;
    #elif wxUSE_FILE
        typedef wxFileInputStream wxImageFileInputStream;
        typedef wxFileOutputStream wxImageFileOutputStream;
    #endif // wxUSE_FILE/wxUSE_FFILE
#endif // HAS_FILE_STREAMS

#if wxUSE_VARIANT
IMPLEMENT_VARIANT_SHALLOWCMP(wxImage,WXDLLEXPORT)
#endif

//-----------------------------------------------------------------------------
// wxImageRefData
//-----------------------------------------------------------------------------

struct wxImageRefData: public wxObjectRefData
{
    wxImageRefData() = default;
    ~wxImageRefData();

    wxImageRefData(const wxImageRefData&) = delete;
	wxImageRefData& operator=(const wxImageRefData&) = delete;

    std::vector<std::string>   m_optionNames;
    std::vector<std::string>   m_optionValues;

#if wxUSE_PALETTE
    wxPalette       m_palette;
#endif // wxUSE_PALETTE

    unsigned char* m_data{ nullptr };

    // alpha channel data, may be nullptr for the formats without alpha support
    unsigned char* m_alpha{ nullptr };

    static unsigned int  sm_defaultLoadFlags;

    int             m_width{0};
    int             m_height{0};
    unsigned int    m_loadFlags{ sm_defaultLoadFlags };

    unsigned char   m_maskRed{0};
    unsigned char   m_maskGreen{0};
    unsigned char   m_maskBlue{0};

    // global and per-object flags determining LoadFile() behaviour

    wxBitmapType    m_type{ wxBitmapType::Invalid };

    bool            m_ok{ false };

    // if true, m_data is pointer to static data and shouldn't be freed
    bool            m_static{ false };

    // same as m_static but for m_alpha
    bool            m_staticAlpha{ false };

    bool            m_hasMask{ false };
};

// For compatibility, if nothing else, loading is verbose by default.
unsigned int wxImageRefData::sm_defaultLoadFlags = wxImage::Load_Verbose;

wxImageRefData::~wxImageRefData()
{
    if ( !m_static )
        free( m_data );
    if ( !m_staticAlpha )
        free( m_alpha );
}


//-----------------------------------------------------------------------------
// wxImage
//-----------------------------------------------------------------------------

#define M_IMGDATA static_cast<wxImageRefData*>(m_refData)

bool wxImage::Create(const char* const* xpmData)
{
#if wxUSE_XPM
    UnRef();

    wxXPMDecoder decoder;
    (*this) = decoder.ReadData(xpmData);
    return IsOk();
#else
    wxUnusedVar(xpmData);
    return false;
#endif
}

bool wxImage::Create( int width, int height, bool clear )
{
    UnRef();

    m_refData = new wxImageRefData();

    M_IMGDATA->m_data = (unsigned char *) malloc( width*height*3 );
    if (!M_IMGDATA->m_data)
    {
        UnRef();
        return false;
    }

    M_IMGDATA->m_width = width;
    M_IMGDATA->m_height = height;
    M_IMGDATA->m_ok = true;

    if (clear)
    {
        Clear();
    }

    return true;
}

bool wxImage::Create( int width, int height, unsigned char* data, bool static_data )
{
    UnRef();

    wxCHECK_MSG( data, false, "NULL data in wxImage::Create" );

    m_refData = new wxImageRefData();

    M_IMGDATA->m_data = data;
    M_IMGDATA->m_width = width;
    M_IMGDATA->m_height = height;
    M_IMGDATA->m_ok = true;
    M_IMGDATA->m_static = static_data;

    return true;
}

bool wxImage::Create( int width, int height, unsigned char* data, unsigned char* alpha, bool static_data )
{
    UnRef();

    wxCHECK_MSG( data, false, "NULL data in wxImage::Create" );

    m_refData = new wxImageRefData();

    M_IMGDATA->m_data = data;
    M_IMGDATA->m_alpha = alpha;
    M_IMGDATA->m_width = width;
    M_IMGDATA->m_height = height;
    M_IMGDATA->m_ok = true;
    M_IMGDATA->m_static = static_data;
    M_IMGDATA->m_staticAlpha = static_data;

    return true;
}

void wxImage::Destroy()
{
    UnRef();
}

void wxImage::Clear(unsigned char value)
{
    memset(M_IMGDATA->m_data, value, M_IMGDATA->m_width*M_IMGDATA->m_height*3);
}

wxObjectRefData* wxImage::CreateRefData() const
{
    return new wxImageRefData;
}

wxObjectRefData* wxImage::CloneRefData(const wxObjectRefData* that) const
{
    const auto* refData = dynamic_cast<const wxImageRefData*>(that);
    wxCHECK_MSG(refData->m_ok, nullptr, "invalid image" );

    wxImageRefData* refData_new = new wxImageRefData;
    refData_new->m_width = refData->m_width;
    refData_new->m_height = refData->m_height;
    refData_new->m_maskRed = refData->m_maskRed;
    refData_new->m_maskGreen = refData->m_maskGreen;
    refData_new->m_maskBlue = refData->m_maskBlue;
    refData_new->m_hasMask = refData->m_hasMask;
    refData_new->m_ok = true;
    unsigned size = unsigned(refData->m_width) * unsigned(refData->m_height);
    if (refData->m_alpha != nullptr)
    {
        refData_new->m_alpha = (unsigned char*)malloc(size);
        memcpy(refData_new->m_alpha, refData->m_alpha, size);
    }
    size *= 3;
    refData_new->m_data = (unsigned char*)malloc(size);
    memcpy(refData_new->m_data, refData->m_data, size);
#if wxUSE_PALETTE
    refData_new->m_palette = refData->m_palette;
#endif
    refData_new->m_optionNames = refData->m_optionNames;
    refData_new->m_optionValues = refData->m_optionValues;
    return refData_new;
}

// returns a new image with the same dimensions, alpha, and mask as *this
// if on_its_side is true, width and height are swapped
wxImage wxImage::MakeEmptyClone(int flags) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, "invalid image" );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    if ( flags & Clone_SwapOrientation )
        std::swap( width, height );

    if ( !image.Create( width, height, false ) )
    {
        wxFAIL_MSG( "unable to create image" );
        return image;
    }

    if ( M_IMGDATA->m_alpha )
    {
        image.SetAlpha();
        wxCHECK2_MSG( image.GetAlpha(), return wxImage(),
                      "unable to create alpha channel" );
    }

    if ( M_IMGDATA->m_hasMask )
    {
        image.SetMaskColour( M_IMGDATA->m_maskRed,
                             M_IMGDATA->m_maskGreen,
                             M_IMGDATA->m_maskBlue );
    }

    return image;
}

wxImage wxImage::Copy() const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, "invalid image" );

    image.m_refData = CloneRefData(m_refData);

    return image;
}

wxImage wxImage::ShrinkBy( int xFactor , int yFactor ) const
{
    if( xFactor == 1 && yFactor == 1 )
        return *this;

    wxImage image;

    wxCHECK_MSG( IsOk(), image, "invalid image" );

    // can't scale to/from 0 size
    wxCHECK_MSG( (xFactor > 0) && (yFactor > 0), image,
                 "invalid new image size" );

    const long old_height = M_IMGDATA->m_height;
    const long old_width  = M_IMGDATA->m_width;

    wxCHECK_MSG( (old_height > 0) && (old_width > 0), image,
                 "invalid old image size" );

    const long width = old_width / xFactor ;
    const long height = old_height / yFactor ;

    image.Create( width, height, false );

    char unsigned *data = image.GetData();

    wxCHECK_MSG( data, image, "unable to create image" );

    bool hasMask = false ;
    unsigned char maskRed = 0;
    unsigned char maskGreen = 0;
    unsigned char maskBlue = 0 ;

    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data = data;
    const unsigned char *source_alpha = nullptr ;
    unsigned char *target_alpha = nullptr ;
    if (M_IMGDATA->m_hasMask)
    {
        hasMask = true ;
        maskRed = M_IMGDATA->m_maskRed;
        maskGreen = M_IMGDATA->m_maskGreen;
        maskBlue =M_IMGDATA->m_maskBlue ;

        image.SetMaskColour( M_IMGDATA->m_maskRed,
                             M_IMGDATA->m_maskGreen,
                             M_IMGDATA->m_maskBlue );
    }
    else
    {
        source_alpha = M_IMGDATA->m_alpha ;
        if ( source_alpha )
        {
            image.SetAlpha() ;
            target_alpha = image.GetAlpha() ;
        }
    }

    for (long y = 0; y < height; y++)
    {
        for (long x = 0; x < width; x++)
        {
            unsigned long avgRed = 0 ;
            unsigned long avgGreen = 0;
            unsigned long avgBlue = 0;
            unsigned long avgAlpha = 0 ;
            unsigned long counter = 0 ;
            // determine average
            for ( int y1 = 0 ; y1 < yFactor ; ++y1 )
            {
                const long y_offset = (y * yFactor + y1) * old_width;
                for ( int x1 = 0 ; x1 < xFactor ; ++x1 )
                {
                    const unsigned char *pixel = source_data + 3 * ( y_offset + x * xFactor + x1 ) ;
                    const unsigned char red = pixel[0] ;
                    const unsigned char green = pixel[1] ;
                    const unsigned char blue = pixel[2] ;
                    unsigned char alpha = 255  ;
                    if ( source_alpha )
                        alpha = *(source_alpha + y_offset + x * xFactor + x1) ;
                    if ( !hasMask || red != maskRed || green != maskGreen || blue != maskBlue )
                    {
                        if ( alpha > 0 )
                        {
                            avgRed += red ;
                            avgGreen += green ;
                            avgBlue += blue ;
                        }
                        avgAlpha += alpha ;
                        counter++ ;
                    }
                }
            }
            if ( counter == 0 )
            {
                *(target_data++) = M_IMGDATA->m_maskRed ;
                *(target_data++) = M_IMGDATA->m_maskGreen ;
                *(target_data++) = M_IMGDATA->m_maskBlue ;
            }
            else
            {
                if ( source_alpha )
                    *(target_alpha++) = (unsigned char)(avgAlpha / counter ) ;
                *(target_data++) = (unsigned char)(avgRed / counter);
                *(target_data++) = (unsigned char)(avgGreen / counter);
                *(target_data++) = (unsigned char)(avgBlue / counter);
            }
        }
    }

    // In case this is a cursor, make sure the hotspot is scaled accordingly:
    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X))/xFactor);
    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y))/yFactor);

    return image;
}

wxImage
wxImage::Scale( int width, int height, wxImageResizeQuality quality ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, "invalid image" );

    // can't scale to/from 0 size
    wxCHECK_MSG( (width > 0) && (height > 0), image,
                 "invalid new image size" );

    const long old_height = M_IMGDATA->m_height;
    const long old_width  = M_IMGDATA->m_width;
    wxCHECK_MSG( (old_height > 0) && (old_width > 0), image,
                 "invalid old image size" );

    // If the image's new width and height are the same as the original, no
    // need to waste time or CPU cycles
    if ( old_width == width && old_height == height )
        return *this;

    // Resample the image using the method as specified.
    switch ( quality )
    {
        case wxImageResizeQuality::Nearest:
            if ( old_width % width == 0 && old_width >= width &&
                old_height % height == 0 && old_height >= height )
            {
                return ShrinkBy( old_width / width , old_height / height );
            }

            image = ResampleNearest(width, height);
            break;

        case wxImageResizeQuality::Bilinear:
            image = ResampleBilinear(width, height);
            break;

        case wxImageResizeQuality::Bicubic:
            image = ResampleBicubic(width, height);
            break;

        case wxImageResizeQuality::BoxAverage:
            image = ResampleBox(width, height);
            break;

        case wxImageResizeQuality::High:
            image = width < old_width && height < old_height
                        ? ResampleBox(width, height)
                        : ResampleBicubic(width, height);
            break;
    }

    // If the original image has a mask, apply the mask to the new image
    if (M_IMGDATA->m_hasMask)
    {
        image.SetMaskColour( M_IMGDATA->m_maskRed,
                            M_IMGDATA->m_maskGreen,
                            M_IMGDATA->m_maskBlue );
    }

    // In case this is a cursor, make sure the hotspot is scaled accordingly:
    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X)*width)/old_width);
    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y)*height)/old_height);

    return image;
}

wxImage wxImage::ResampleNearest(int width, int height) const
{
    wxImage image;

    const unsigned long old_width  = M_IMGDATA->m_width;
    const unsigned long old_height = M_IMGDATA->m_height;
    wxCHECK_MSG(old_width  <= (ULONG_MAX >> 16) &&
                old_height <= (ULONG_MAX >> 16), image, "image dimension too large");

    image.Create( width, height, false );

    unsigned char *data = image.GetData();

    wxCHECK_MSG( data, image, "unable to create image" );

    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data = data;
    const unsigned char *source_alpha = nullptr ;
    unsigned char *target_alpha = nullptr ;

    if ( !M_IMGDATA->m_hasMask )
    {
        source_alpha = M_IMGDATA->m_alpha ;
        if ( source_alpha )
        {
            image.SetAlpha() ;
            target_alpha = image.GetAlpha() ;
        }
    }

    const unsigned long x_delta = (old_width  << 16) / width;
    const unsigned long y_delta = (old_height << 16) / height;

    unsigned char* dest_pixel = target_data;

    unsigned long y = 0;
    for (int j = 0; j < height; j++)
    {
        const unsigned char* src_line = &source_data[(y>>16)*old_width*3];
        const unsigned char* src_alpha_line = source_alpha ? &source_alpha[(y>>16)*old_width] : nullptr ;

        unsigned long x = 0;
        for (int i = 0; i < width; i++)
        {
            const unsigned char* src_pixel = &src_line[(x>>16)*3];
            const unsigned char* src_alpha_pixel = source_alpha ? &src_alpha_line[(x>>16)] : nullptr ;
            dest_pixel[0] = src_pixel[0];
            dest_pixel[1] = src_pixel[1];
            dest_pixel[2] = src_pixel[2];
            dest_pixel += 3;
            if ( source_alpha )
                *(target_alpha++) = *src_alpha_pixel ;
            x += x_delta;
        }

        y += y_delta;
    }

    return image;
}

namespace
{

struct BoxPrecalc
{
    int boxStart;
    int boxEnd;
};

void ResampleBoxPrecalc(std::vector<BoxPrecalc>& boxes, int oldDim)
{
    const auto newDim = boxes.size();
    wxASSERT( oldDim > 0 && newDim > 0 );

    // We need to map pixel values in the range [-0.5 .. (newDim-1)+0.5]
    // to the pixel values in the range [-0.5 .. (oldDim-1)+0.5].
    // Transformation function is therefore:
    //   pOld = sc * (pNew + 0.5) - 0.5, where sc = oldDim/newDim
    //
    // A new pixel pNew in the interval [pNew-0.5 .. pNew+0.5]
    // is mapped to the old pixel in the interval [pOldLoBound..pOldUpBound],
    // where:
    //   pOldLoBound = sc * ((pNew-0.5) + 0.5) - 0.5 = sc * pNew - 0.5
    //   pOldUpBound = sc * ((pNew+0.5) + 0.5) - 0.5 = sc * (pNew+1) - 0.5
    // So, the lower bound of the pixel box (interval) is:
    //   boxStart = round(pOldLoBound) = trunc((sc * pNew - 0.5) + 0.5) = trunc(sc * pNew)
    // and the upper bound is:
    // - if fraction(pOldUpBound) != 0.5 (bound inside the pixel):
    //   boxEnd = round(pixOldUpBound) = trunc((sc * (pNew+1) - 0.5) + 0.5) = trunc(sc * (pNew+1))
    //    e.g. for UpBound = 7.2 -> boxEnd = 7
    //         for UpBound = 7.6 -> boxEnd = 8
    // - if fraction(pOldUpBound) == 0.5 (bound at the edge of the pixel):
    //   boxEnd = round(pOldUpBound)-1 = trunc((sc * (pNew+1) - 0.5) + 0.5) - 1 = trunc(sc * (pNew+1))-1
    //    e.g. for UpBound = 7.5 -> boxEnd = 7 (not 8)
    //
    // In integer arithmetic:
    //  boxStart = (oldDim * pNew) / newDim
    //  boxEnd:
    //   vEnd =  oldDim * (pNew+1) = oldDim * pNew + oldDim
    //  if vEnd % newDim != 0 (frac(pOldUpBound) != 0.5) => boxEnd = vEnd / newDim
    //  if vEnd % newDim == 0 (frac(pOldUpBound) == 0.5) => boxEnd = (vEnd / newDim) - 1

    int v = 0; // oldDim * 0
    for ( int dst = 0; dst < newDim; dst++ )
    {
        BoxPrecalc& precalc = boxes[dst];
        precalc.boxStart = v / newDim;
        v += oldDim;
        precalc.boxEnd = v % newDim != 0 ? v / newDim : (v / newDim) - 1;
    }
}

} // anonymous namespace

wxImage wxImage::ResampleBox(int width, int height) const
{
    // This function implements a simple pre-blur/box averaging method for
    // downsampling that gives reasonably smooth results To scale the image
    // down we will need to gather a grid of pixels of the size of the scale
    // factor in each direction and then do an averaging of the pixels.

    wxImage ret_image(width, height, false);

    std::vector<BoxPrecalc> vPrecalcs(height);
    std::vector<BoxPrecalc> hPrecalcs(width);

    ResampleBoxPrecalc(vPrecalcs, M_IMGDATA->m_height);
    ResampleBoxPrecalc(hPrecalcs, M_IMGDATA->m_width);


    const unsigned char* src_data = M_IMGDATA->m_data;
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_data = ret_image.GetData();
    unsigned char* dst_alpha = nullptr;

    wxCHECK_MSG( dst_data, ret_image, "unable to create image" );

    if ( src_alpha )
    {
        ret_image.SetAlpha();
        dst_alpha = ret_image.GetAlpha();
    }

    int averaged_pixels, src_pixel_index;
    double sum_r, sum_g, sum_b, sum_a;

    for ( int y = 0; y < height; y++ )         // Destination image - Y direction
    {
        // Source pixel in the Y direction
        const BoxPrecalc& vPrecalc = vPrecalcs[y];

        for ( int x = 0; x < width; x++ )      // Destination image - X direction
        {
            // Source pixel in the X direction
            const BoxPrecalc& hPrecalc = hPrecalcs[x];

            // Box of pixels to average
            averaged_pixels = 0;
            sum_r = sum_g = sum_b = sum_a = 0.0;

            for ( int j = vPrecalc.boxStart; j <= vPrecalc.boxEnd; ++j )
            {
                for ( int i = hPrecalc.boxStart; i <= hPrecalc.boxEnd; ++i )
                {
                    // Calculate the actual index in our source pixels
                    src_pixel_index = j * M_IMGDATA->m_width + i;

                    if (src_alpha)
                    {
                        sum_r += src_data[src_pixel_index * 3 + 0] * src_alpha[src_pixel_index];
                        sum_g += src_data[src_pixel_index * 3 + 1] * src_alpha[src_pixel_index];
                        sum_b += src_data[src_pixel_index * 3 + 2] * src_alpha[src_pixel_index];
                        sum_a += src_alpha[src_pixel_index];
                    }
                    else
                    {
                        sum_r += src_data[src_pixel_index * 3 + 0];
                        sum_g += src_data[src_pixel_index * 3 + 1];
                        sum_b += src_data[src_pixel_index * 3 + 2];
                    }

                    averaged_pixels++;
                }
            }

            // Calculate the average from the sum and number of averaged pixels
            if (src_alpha)
            {
                if (sum_a)
                {
                    dst_data[0] = (unsigned char)(sum_r / sum_a);
                    dst_data[1] = (unsigned char)(sum_g / sum_a);
                    dst_data[2] = (unsigned char)(sum_b / sum_a);
                }
                else
                {
                    dst_data[0] = 0;
                    dst_data[1] = 0;
                    dst_data[2] = 0;
                }
                *dst_alpha++ = (unsigned char)(sum_a / averaged_pixels);
            }
            else
            {
                dst_data[0] = (unsigned char)(sum_r / averaged_pixels);
                dst_data[1] = (unsigned char)(sum_g / averaged_pixels);
                dst_data[2] = (unsigned char)(sum_b / averaged_pixels);
            }
            dst_data += 3;
        }
    }

    return ret_image;
}

namespace
{

struct BilinearPrecalc
{
    int offset1;
    int offset2;
    double dd;
    double dd1;
};

inline void DoCalc(BilinearPrecalc& precalc, double srcpix, int srcpixmax)
{
    const int srcpix1 = int(srcpix);
    const int srcpix2 = srcpix1 == srcpixmax ? srcpix1 : srcpix1 + 1;

    precalc.dd = srcpix - (int)srcpix;
    precalc.dd1 = 1.0 - precalc.dd;
    precalc.offset1 = srcpix1 < 0.0
                        ? 0
                        : srcpix1 > srcpixmax
                            ? srcpixmax
                            : (int)srcpix1;
    precalc.offset2 = srcpix2 < 0.0
                        ? 0
                        : srcpix2 > srcpixmax
                            ? srcpixmax
                            : (int)srcpix2;
}

void ResampleBilinearPrecalc(std::vector<BilinearPrecalc>& precalcs, int oldDim)
{
    const auto newDim = precalcs.size();
    wxASSERT( oldDim > 0 && newDim > 0 );
    const int srcpixmax = oldDim - 1;
    if ( newDim > 1 )
    {
        // We want to map pixels in the range [0..newDim-1]
        // to the range [0..oldDim-1]
        const double scale_factor = double(oldDim - 1) / (newDim - 1);

        for ( int dsty = 0; dsty < newDim; dsty++ )
        {
            // We need to calculate the source pixel to interpolate from - Y-axis
            const double srcpix = (double)dsty * scale_factor;

            DoCalc(precalcs[dsty], srcpix, srcpixmax);
        }
    }
    else
    {
        // Let's take the pixel from the center of the source image.
        const double srcpix = static_cast<double>(srcpixmax) / 2.0;

        DoCalc(precalcs[0], srcpix, srcpixmax);
    }
}

} // anonymous namespace

wxImage wxImage::ResampleBilinear(int width, int height) const
{
    // This function implements a Bilinear algorithm for resampling.
    wxImage ret_image(width, height, false);
    const unsigned char* src_data = M_IMGDATA->m_data;
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_data = ret_image.GetData();
    unsigned char* dst_alpha = nullptr;

    wxCHECK_MSG( dst_data, ret_image, "unable to create image" );

    if ( src_alpha )
    {
        ret_image.SetAlpha();
        dst_alpha = ret_image.GetAlpha();
    }

    std::vector<BilinearPrecalc> vPrecalcs(height);
    std::vector<BilinearPrecalc> hPrecalcs(width);
    ResampleBilinearPrecalc(vPrecalcs, M_IMGDATA->m_height);
    ResampleBilinearPrecalc(hPrecalcs, M_IMGDATA->m_width);

    // initialize alpha values to avoid g++ warnings about possibly
    // uninitialized variables
    double r1, g1, b1, a1 = 0;
    double r2, g2, b2, a2 = 0;

    for ( int dsty = 0; dsty < height; dsty++ )
    {
        // We need to calculate the source pixel to interpolate from - Y-axis
        const BilinearPrecalc& vPrecalc = vPrecalcs[dsty];
        const int y_offset1 = vPrecalc.offset1;
        const int y_offset2 = vPrecalc.offset2;
        const double dy = vPrecalc.dd;
        const double dy1 = vPrecalc.dd1;


        for ( int dstx = 0; dstx < width; dstx++ )
        {
            // X-axis of pixel to interpolate from
            const BilinearPrecalc& hPrecalc = hPrecalcs[dstx];

            const int x_offset1 = hPrecalc.offset1;
            const int x_offset2 = hPrecalc.offset2;
            const double dx = hPrecalc.dd;
            const double dx1 = hPrecalc.dd1;

            const int src_pixel_index00 = y_offset1 * M_IMGDATA->m_width + x_offset1;
            const int src_pixel_index01 = y_offset1 * M_IMGDATA->m_width + x_offset2;
            const int src_pixel_index10 = y_offset2 * M_IMGDATA->m_width + x_offset1;
            const int src_pixel_index11 = y_offset2 * M_IMGDATA->m_width + x_offset2;

            // first line
            r1 = src_data[src_pixel_index00 * 3 + 0] * dx1 + src_data[src_pixel_index01 * 3 + 0] * dx;
            g1 = src_data[src_pixel_index00 * 3 + 1] * dx1 + src_data[src_pixel_index01 * 3 + 1] * dx;
            b1 = src_data[src_pixel_index00 * 3 + 2] * dx1 + src_data[src_pixel_index01 * 3 + 2] * dx;
            if ( src_alpha )
                a1 = src_alpha[src_pixel_index00] * dx1 + src_alpha[src_pixel_index01] * dx;

            // second line
            r2 = src_data[src_pixel_index10 * 3 + 0] * dx1 + src_data[src_pixel_index11 * 3 + 0] * dx;
            g2 = src_data[src_pixel_index10 * 3 + 1] * dx1 + src_data[src_pixel_index11 * 3 + 1] * dx;
            b2 = src_data[src_pixel_index10 * 3 + 2] * dx1 + src_data[src_pixel_index11 * 3 + 2] * dx;
            if ( src_alpha )
                a2 = src_alpha[src_pixel_index10] * dx1 + src_alpha[src_pixel_index11] * dx;

            // result lines

            dst_data[0] = static_cast<unsigned char>(r1 * dy1 + r2 * dy + .5);
            dst_data[1] = static_cast<unsigned char>(g1 * dy1 + g2 * dy + .5);
            dst_data[2] = static_cast<unsigned char>(b1 * dy1 + b2 * dy + .5);
            dst_data += 3;

            if ( src_alpha )
                *dst_alpha++ = static_cast<unsigned char>(a1 * dy1 + a2 * dy +.5);
        }
    }

    return ret_image;
}

// The following two local functions are for the B-spline weighting of the
// bicubic sampling algorithm
constexpr double spline_cube(double value)
{
    return value <= 0.0 ? 0.0 : value * value * value;
}

static inline double spline_weight(double value)
{
    return (spline_cube(value + 2) -
            4 * spline_cube(value + 1) +
            6 * spline_cube(value) -
            4 * spline_cube(value - 1)) / 6;
}


namespace
{

struct BicubicPrecalc
{
    double weight[4];
    int offset[4];
};

inline void DoCalc(BicubicPrecalc& precalc, double srcpixd, int oldDim)
{
    const double dd = srcpixd - static_cast<int>(srcpixd);

    for ( int k = -1; k <= 2; k++ )
    {
        precalc.offset[k + 1] = srcpixd + k < 0.0
            ? 0
            : srcpixd + k >= oldDim
                ? oldDim - 1
                : static_cast<int>(srcpixd + k);

        precalc.weight[k + 1] = spline_weight(k - dd);
    }
}

void ResampleBicubicPrecalc(std::vector<BicubicPrecalc> &aWeight, int oldDim)
{
    const auto newDim = aWeight.size();
    wxASSERT( oldDim > 0 && newDim > 0 );

    if ( newDim > 1 )
    {
        // We want to map pixels in the range [0..newDim-1]
        // to the range [0..oldDim-1]
        const double scale_factor = static_cast<double>(oldDim - 1) / (newDim - 1);

        for ( int dstd = 0; dstd < newDim; dstd++ )
        {
            // We need to calculate the source pixel to interpolate from - Y-axis
            const double srcpixd = static_cast<double>(dstd) * scale_factor;

            DoCalc(aWeight[dstd], srcpixd, oldDim);
        }
    }
    else
    {
        // Let's take the pixel from the center of the source image.
        const double srcpixd = static_cast<double>(oldDim - 1) / 2.0;

        DoCalc(aWeight[0], srcpixd, oldDim);
    }
}

} // anonymous namespace

// This is the bicubic resampling algorithm
wxImage wxImage::ResampleBicubic(int width, int height) const
{
    // This function implements a Bicubic B-Spline algorithm for resampling.
    // This method is certainly a little slower than wxImage's default pixel
    // replication method, however for most reasonably sized images not being
    // upsampled too much on a fairly average CPU this difference is hardly
    // noticeable and the results are far more pleasing to look at.
    //
    // This particular bicubic algorithm does pixel weighting according to a
    // B-Spline that basically implements a Gaussian bell-like weighting
    // kernel. Because of this method the results may appear a bit blurry when
    // upsampling by large factors.  This is basically because a slight
    // gaussian blur is being performed to get the smooth look of the upsampled
    // image.

    // Edge pixels: 3-4 possible solutions
    // - (Wrap/tile) Wrap the image, take the color value from the opposite
    // side of the image.
    // - (Mirror)    Duplicate edge pixels, so that pixel at coordinate (2, n),
    // where n is nonpositive, will have the value of (2, 1).
    // - (Ignore)    Simply ignore the edge pixels and apply the kernel only to
    // pixels which do have all neighbours.
    // - (Clamp)     Choose the nearest pixel along the border. This takes the
    // border pixels and extends them out to infinity.
    //
    // NOTE: below the y_offset and x_offset variables are being set for edge
    // pixels using the "Mirror" method mentioned above

    wxImage ret_image;

    ret_image.Create(width, height, false);

    const unsigned char* src_data = M_IMGDATA->m_data;
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_data = ret_image.GetData();
    unsigned char* dst_alpha = nullptr;

    wxCHECK_MSG( dst_data, ret_image, "unable to create image" );

    if ( src_alpha )
    {
        ret_image.SetAlpha();
        dst_alpha = ret_image.GetAlpha();
    }

    // Precalculate weights
    std::vector<BicubicPrecalc> vPrecalcs(height);
    std::vector<BicubicPrecalc> hPrecalcs(width);

    ResampleBicubicPrecalc(vPrecalcs, M_IMGDATA->m_height);
    ResampleBicubicPrecalc(hPrecalcs, M_IMGDATA->m_width);

    for ( int dsty = 0; dsty < height; dsty++ )
    {
        // We need to calculate the source pixel to interpolate from - Y-axis
        const BicubicPrecalc& vPrecalc = vPrecalcs[dsty];

        for ( int dstx = 0; dstx < width; dstx++ )
        {
            // X-axis of pixel to interpolate from
            const BicubicPrecalc& hPrecalc = hPrecalcs[dstx];

            // Sums for each color channel
            double sum_r = 0;
            double sum_g = 0;
            double sum_b = 0;
            double sum_a = 0;

            // Here we actually determine the RGBA values for the destination pixel
            for ( int k = -1; k <= 2; k++ )
            {
                // Y offset
                const int y_offset = vPrecalc.offset[k + 1];

                // Loop across the X axis
                for ( int i = -1; i <= 2; i++ )
                {
                    // X offset
                    const int x_offset = hPrecalc.offset[i + 1];

                    // Calculate the exact position where the source data
                    // should be pulled from based on the x_offset and y_offset
                    const int src_pixel_index = y_offset*M_IMGDATA->m_width + x_offset;

                    // Calculate the weight for the specified pixel according
                    // to the bicubic b-spline kernel we're using for
                    // interpolation
                    const double
                        pixel_weight = vPrecalc.weight[k + 1] * hPrecalc.weight[i + 1];

                    // Create a sum of all velues for each color channel
                    // adjusted for the pixel's calculated weight
                    if ( src_alpha )
                    {
                        const unsigned char a = src_alpha[src_pixel_index];
                        sum_r += src_data[src_pixel_index * 3 + 0] * pixel_weight * a;
                        sum_g += src_data[src_pixel_index * 3 + 1] * pixel_weight * a;
                        sum_b += src_data[src_pixel_index * 3 + 2] * pixel_weight * a;
                        sum_a += a * pixel_weight;
                    }
                    else
                    {
                        sum_r += src_data[src_pixel_index * 3 + 0] * pixel_weight;
                        sum_g += src_data[src_pixel_index * 3 + 1] * pixel_weight;
                        sum_b += src_data[src_pixel_index * 3 + 2] * pixel_weight;
                    }
                }
            }

            // Put the data into the destination image.  The summed values are
            // of double data type and are rounded here for accuracy
            if ( src_alpha )
            {
                if ( sum_a )
                {
                     dst_data[0] = (unsigned char)(sum_r / sum_a + 0.5);
                     dst_data[1] = (unsigned char)(sum_g / sum_a + 0.5);
                     dst_data[2] = (unsigned char)(sum_b / sum_a + 0.5);
                }
                else
                {
                    dst_data[0] = 0;
                    dst_data[1] = 0;
                    dst_data[2] = 0;
                }
                *dst_alpha++ = (unsigned char)sum_a;
            }
            else
            {
                dst_data[0] = (unsigned char)(sum_r + 0.5);
                dst_data[1] = (unsigned char)(sum_g + 0.5);
                dst_data[2] = (unsigned char)(sum_b + 0.5);
            }
            dst_data += 3;
        }
    }

    return ret_image;
}

// Blur in the horizontal direction
wxImage wxImage::BlurHorizontal(int blurRadius) const
{
    wxImage ret_image(MakeEmptyClone());

    wxCHECK( ret_image.IsOk(), ret_image );

    const unsigned char* src_data = M_IMGDATA->m_data;
    unsigned char* dst_data = ret_image.GetData();
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_alpha = ret_image.GetAlpha();

    // number of pixels we average over
    const int blurArea = blurRadius*2 + 1;

    // Horizontal blurring algorithm - average all pixels in the specified blur
    // radius in the X or horizontal direction
    for ( int y = 0; y < M_IMGDATA->m_height; y++ )
    {
        // Variables used in the blurring algorithm
        long sum_r = 0,
             sum_g = 0,
             sum_b = 0,
             sum_a = 0;

        long pixel_idx;
        const unsigned char *src;
        unsigned char *dst;

        // Calculate the average of all pixels in the blur radius for the first
        // pixel of the row
        for ( int kernel_x = -blurRadius; kernel_x <= blurRadius; kernel_x++ )
        {
            // To deal with the pixels at the start of a row so it's not
            // grabbing GOK values from memory at negative indices of the
            // image's data or grabbing from the previous row
            if ( kernel_x < 0 )
                pixel_idx = y * M_IMGDATA->m_width;
            else
                pixel_idx = kernel_x + y * M_IMGDATA->m_width;

            src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];
        }

        dst = dst_data + y * M_IMGDATA->m_width*3;
        dst[0] = (unsigned char)(sum_r / blurArea);
        dst[1] = (unsigned char)(sum_g / blurArea);
        dst[2] = (unsigned char)(sum_b / blurArea);
        if ( src_alpha )
            dst_alpha[y * M_IMGDATA->m_width] = (unsigned char)(sum_a / blurArea);

        // Now average the values of the rest of the pixels by just moving the
        // blur radius box along the row
        for ( int x = 1; x < M_IMGDATA->m_width; x++ )
        {
            // Take care of edge pixels on the left edge by essentially
            // duplicating the edge pixel
            if ( x - blurRadius - 1 < 0 )
                pixel_idx = y * M_IMGDATA->m_width;
            else
                pixel_idx = (x - blurRadius - 1) + y * M_IMGDATA->m_width;

            // Subtract the value of the pixel at the left side of the blur
            // radius box
            src = src_data + pixel_idx*3;
            sum_r -= src[0];
            sum_g -= src[1];
            sum_b -= src[2];
            if ( src_alpha )
                sum_a -= src_alpha[pixel_idx];

            // Take care of edge pixels on the right edge
            if ( x + blurRadius > M_IMGDATA->m_width - 1 )
                pixel_idx = M_IMGDATA->m_width - 1 + y * M_IMGDATA->m_width;
            else
                pixel_idx = x + blurRadius + y * M_IMGDATA->m_width;

            // Add the value of the pixel being added to the end of our box
            src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];

            // Save off the averaged data
            dst = dst_data + x*3 + y*M_IMGDATA->m_width*3;
            dst[0] = (unsigned char)(sum_r / blurArea);
            dst[1] = (unsigned char)(sum_g / blurArea);
            dst[2] = (unsigned char)(sum_b / blurArea);
            if ( src_alpha )
                dst_alpha[x + y * M_IMGDATA->m_width] = (unsigned char)(sum_a / blurArea);
        }
    }

    return ret_image;
}

// Blur in the vertical direction
wxImage wxImage::BlurVertical(int blurRadius) const
{
    wxImage ret_image(MakeEmptyClone());

    wxCHECK( ret_image.IsOk(), ret_image );

    const unsigned char* src_data = M_IMGDATA->m_data;
    unsigned char* dst_data = ret_image.GetData();
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_alpha = ret_image.GetAlpha();

    // number of pixels we average over
    const int blurArea = blurRadius*2 + 1;

    // Vertical blurring algorithm - same as horizontal but switched the
    // opposite direction
    for ( int x = 0; x < M_IMGDATA->m_width; x++ )
    {
        // Variables used in the blurring algorithm
        long sum_r = 0,
             sum_g = 0,
             sum_b = 0,
             sum_a = 0;

        long pixel_idx;
        const unsigned char *src;
        unsigned char *dst;

        // Calculate the average of all pixels in our blur radius box for the
        // first pixel of the column
        for ( int kernel_y = -blurRadius; kernel_y <= blurRadius; kernel_y++ )
        {
            // To deal with the pixels at the start of a column so it's not
            // grabbing GOK values from memory at negative indices of the
            // image's data or grabbing from the previous column
            if ( kernel_y < 0 )
                pixel_idx = x;
            else
                pixel_idx = x + kernel_y * M_IMGDATA->m_width;

            src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];
        }

        dst = dst_data + x*3;
        dst[0] = (unsigned char)(sum_r / blurArea);
        dst[1] = (unsigned char)(sum_g / blurArea);
        dst[2] = (unsigned char)(sum_b / blurArea);
        if ( src_alpha )
            dst_alpha[x] = (unsigned char)(sum_a / blurArea);

        // Now average the values of the rest of the pixels by just moving the
        // box along the column from top to bottom
        for ( int y = 1; y < M_IMGDATA->m_height; y++ )
        {
            // Take care of pixels that would be beyond the top edge by
            // duplicating the top edge pixel for the column
            if ( y - blurRadius - 1 < 0 )
                pixel_idx = x;
            else
                pixel_idx = x + (y - blurRadius - 1) * M_IMGDATA->m_width;

            // Subtract the value of the pixel at the top of our blur radius box
            src = src_data + pixel_idx*3;
            sum_r -= src[0];
            sum_g -= src[1];
            sum_b -= src[2];
            if ( src_alpha )
                sum_a -= src_alpha[pixel_idx];

            // Take care of the pixels that would be beyond the bottom edge of
            // the image similar to the top edge
            if ( y + blurRadius > M_IMGDATA->m_height - 1 )
                pixel_idx = x + (M_IMGDATA->m_height - 1) * M_IMGDATA->m_width;
            else
                pixel_idx = x + (blurRadius + y) * M_IMGDATA->m_width;

            // Add the value of the pixel being added to the end of our box
            src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];

            // Save off the averaged data
            dst = dst_data + (x + y * M_IMGDATA->m_width) * 3;
            dst[0] = (unsigned char)(sum_r / blurArea);
            dst[1] = (unsigned char)(sum_g / blurArea);
            dst[2] = (unsigned char)(sum_b / blurArea);
            if ( src_alpha )
                dst_alpha[x + y * M_IMGDATA->m_width] = (unsigned char)(sum_a / blurArea);
        }
    }

    return ret_image;
}

// The new blur function
wxImage wxImage::Blur(int blurRadius) const
{
    wxImage ret_image;
    ret_image.Create(M_IMGDATA->m_width, M_IMGDATA->m_height, false);

    // Blur the image in each direction
    ret_image = BlurHorizontal(blurRadius);
    ret_image = ret_image.BlurVertical(blurRadius);

    return ret_image;
}

wxImage wxImage::Rotate90( bool clockwise ) const
{
    wxImage image(MakeEmptyClone(Clone_SwapOrientation));

    wxCHECK( image.IsOk(), image );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
    {
        int hot_x = GetOptionInt( wxIMAGE_OPTION_CUR_HOTSPOT_X );
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                        clockwise ? hot_x : width - 1 - hot_x);
    }

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
    {
        int hot_y = GetOptionInt( wxIMAGE_OPTION_CUR_HOTSPOT_Y );
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                        clockwise ? height - 1 - hot_y : hot_y);
    }

    unsigned char *data = image.GetData();
    unsigned char *target_data;

    // we rotate the image in 21-pixel (63-byte) wide strips
    // to make better use of cpu cache - memory transfers
    // (note: while much better than single-pixel "strips",
    //  our vertical strips will still generally straddle 64-byte cachelines)
    for (long ii = 0; ii < width; )
    {
        const long next_ii = std::min(ii + 21, width);

        for (long j = 0; j < height; j++)
        {
            const unsigned char *source_data =
                M_IMGDATA->m_data + (j*width + ii)*3;

            for (long i = ii; i < next_ii; i++)
            {
                if ( clockwise )
                {
                    target_data = data + ((i + 1)*height - j - 1)*3;
                }
                else
                {
                    target_data = data + (height*(width - 1 - i) + j)*3;
                }
                memcpy( target_data, source_data, 3 );
                source_data += 3;
            }
        }

        ii = next_ii;
    }

    const unsigned char *source_alpha = M_IMGDATA->m_alpha;

    if ( source_alpha )
    {
        unsigned char *alpha_data = image.GetAlpha();

        for (long ii = 0; ii < width; )
        {
            const long next_ii = std::min(ii + 64, width);

            for (long j = 0; j < height; j++)
            {
                source_alpha = M_IMGDATA->m_alpha + j*width + ii;

                for (long i = ii; i < next_ii; i++)
                {
                    unsigned char* target_alpha;
                    if ( clockwise )
                    {
                        target_alpha = alpha_data + (i+1)*height - j - 1;
                    }
                    else
                    {
                        target_alpha = alpha_data + height*(width - i - 1) + j;
                    }

                    *target_alpha = *source_alpha++;
                }
            }

            ii = next_ii;
        }
    }

    return image;
}

wxImage wxImage::Rotate180() const
{
    wxImage image(MakeEmptyClone());

    wxCHECK( image.IsOk(), image );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
    {
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                        width - 1 - GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X));
    }

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
    {
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                        height - 1 - GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y));
    }

    unsigned char *data = image.GetData();
    unsigned char *alpha = image.GetAlpha();
    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data = data + width * height * 3;

    for (long j = 0; j < height; j++)
    {
        for (long i = 0; i < width; i++)
        {
            target_data -= 3;
            memcpy( target_data, source_data, 3 );
            source_data += 3;
        }
    }

    if ( alpha )
    {
        const unsigned char *src_alpha = M_IMGDATA->m_alpha;
        unsigned char *dest_alpha = alpha + width * height;

        for (long j = 0; j < height; ++j)
        {
            for (long i = 0; i < width; ++i)
            {
                *(--dest_alpha) = *(src_alpha++);
            }
        }
    }

    return image;
}

wxImage wxImage::Mirror( bool horizontally ) const
{
    wxImage image(MakeEmptyClone());

    wxCHECK( image.IsOk(), image );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    unsigned char *data = image.GetData();
    unsigned char *alpha = image.GetAlpha();
    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data;

    if (horizontally)
    {
        for (long j = 0; j < height; j++)
        {
            data += width*3;
            target_data = data-3;
            for (long i = 0; i < width; i++)
            {
                memcpy( target_data, source_data, 3 );
                source_data += 3;
                target_data -= 3;
            }
        }

        if (alpha != nullptr)
        {
            // src_alpha starts at the first pixel and increases by 1 after each step
            // (a step here is the copy of the alpha value of one pixel)
            const unsigned char *src_alpha = M_IMGDATA->m_alpha;
            // dest_alpha starts just beyond the first line, decreases before each step,
            // and after each line is finished, increases by 2 widths (skipping the line
            // just copied and the line that will be copied next)
            unsigned char *dest_alpha = alpha + width;

            for (long jj = 0; jj < height; ++jj)
            {
                for (long i = 0; i < width; ++i) {
                    *(--dest_alpha) = *(src_alpha++); // copy one pixel
                }
                dest_alpha += 2 * width; // advance beyond the end of the next line
            }
        }
    }
    else
    {
        for (long i = 0; i < height; i++)
        {
            target_data = data + 3*width*(height-1-i);
            memcpy( target_data, source_data, (size_t)3*width );
            source_data += 3*width;
        }

        if ( alpha )
        {
            // src_alpha starts at the first pixel and increases by 1 width after each step
            // (a step here is the copy of the alpha channel of an entire line)
            const unsigned char *src_alpha = M_IMGDATA->m_alpha;
            // dest_alpha starts just beyond the last line (beyond the whole image)
            // and decreases by 1 width before each step
            unsigned char *dest_alpha = alpha + width * height;

            for (long jj = 0; jj < height; ++jj)
            {
                dest_alpha -= width;
                memcpy( dest_alpha, src_alpha, (size_t)width );
                src_alpha += width;
            }
        }
    }

    return image;
}

wxImage wxImage::GetSubImage( const wxRect &rect ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, "invalid image" );

    wxCHECK_MSG( (rect.GetLeft()>=0) && (rect.GetTop()>=0) &&
                 (rect.GetRight()<=GetWidth()) && (rect.GetBottom()<=GetHeight()),
                 image, "invalid subimage size" );

    const int subwidth = rect.GetWidth();
    const int subheight = rect.GetHeight();

    image.Create( subwidth, subheight, false );

    const unsigned char *src_data = GetData();
    const unsigned char *src_alpha = M_IMGDATA->m_alpha;
    unsigned char *subdata = image.GetData();
    unsigned char *subalpha = nullptr;

    wxCHECK_MSG( subdata, image, "unable to create image" );

    if ( src_alpha ) {
        image.SetAlpha();
        subalpha = image.GetAlpha();
        wxCHECK_MSG( subalpha, image, "unable to create alpha channel");
    }

    if (M_IMGDATA->m_hasMask)
        image.SetMaskColour( M_IMGDATA->m_maskRed, M_IMGDATA->m_maskGreen, M_IMGDATA->m_maskBlue );

    const int width = GetWidth();
    const int pixsoff = rect.GetLeft() + width * rect.GetTop();

    src_data += 3 * pixsoff;
    src_alpha += pixsoff; // won't be used if was NULL, so this is ok

    for (long j = 0; j < subheight; ++j)
    {
        memcpy( subdata, src_data, 3 * subwidth );
        subdata += 3 * subwidth;
        src_data += 3 * width;
        if (subalpha != nullptr) {
            memcpy( subalpha, src_alpha, subwidth );
            subalpha += subwidth;
            src_alpha += width;
        }
    }

    return image;
}

wxImage wxImage::Size( const wxSize& size, const wxPoint& pos,
                       int r_, int g_, int b_ ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, "invalid image" );
    wxCHECK_MSG( (size.x > 0) && (size.y > 0), image, "invalid size" );

    const int width = GetWidth();
    const int height = GetHeight();
    image.Create(size.x, size.y, false);

    unsigned char r = (unsigned char)r_;
    unsigned char g = (unsigned char)g_;
    unsigned char b = (unsigned char)b_;
    if ((r_ == -1) && (g_ == -1) && (b_ == -1))
    {
        GetOrFindMaskColour( &r, &g, &b );
        image.SetMaskColour(r, g, b);
    }

    image.SetRGB(wxRect(), r, g, b);

    // we have two coordinate systems:
    // source:     starting at 0,0 of source image
    // destination starting at 0,0 of destination image
    // Documentation says:
    // "The image is pasted into a new image [...] at the position pos relative
    // to the upper left of the new image." this means the transition rule is:
    // "dest coord" = "source coord" + pos;

    // calculate the intersection using source coordinates:
    wxRect srcRect(0, 0, width, height);
    const wxRect dstRect(-pos, size);

    srcRect.Intersect(dstRect);

    if (!srcRect.IsEmpty())
    {
        // insertion point is needed in destination coordinates.
        // NB: it is not always "pos"!
        wxPoint ptInsert = srcRect.GetTopLeft() + pos;

        if ((srcRect.GetWidth() == width) && (srcRect.GetHeight() == height))
            image.Paste(*this, ptInsert.x, ptInsert.y);
        else
            image.Paste(GetSubImage(srcRect), ptInsert.x, ptInsert.y);
    }

    return image;
}

void
wxImage::Paste(const wxImage & image, int x, int y,
               wxImageAlphaBlendMode alphaBlend)
{
    wxCHECK_RET( IsOk(), "invalid image" );
    wxCHECK_RET( image.IsOk(), "invalid image" );

    AllocExclusive();

    int xx = 0;
    int yy = 0;
    int width = image.GetWidth();
    int height = image.GetHeight();

    if (x < 0)
    {
        xx = -x;
        width += x;
    }
    if (y < 0)
    {
        yy = -y;
        height += y;
    }

    if ((x+xx)+width > M_IMGDATA->m_width)
        width = M_IMGDATA->m_width - (x+xx);
    if ((y+yy)+height > M_IMGDATA->m_height)
        height = M_IMGDATA->m_height - (y+yy);

    if (width < 1) return;
    if (height < 1) return;

    bool copiedPixels = false;

    // If we can, copy the data using memcpy() as this is the fastest way. But
    // for this we must not do alpha compositing and the image being pasted
    // must have "compatible" mask with this one meaning that either it must
    // not have one at all or it must use the same masked colour.
    if (alphaBlend == wxIMAGE_ALPHA_BLEND_OVER &&
        (!image.HasMask() ||
        ((HasMask() &&
         (GetMaskRed()==image.GetMaskRed()) &&
         (GetMaskGreen()==image.GetMaskGreen()) &&
         (GetMaskBlue()==image.GetMaskBlue())))) )
    {
        const unsigned char* source_data = image.GetData() + 3*(xx + yy*image.GetWidth());
        const int source_step = image.GetWidth()*3;

        unsigned char* target_data = GetData() + 3*((x+xx) + (y+yy)*M_IMGDATA->m_width);
        int target_step = M_IMGDATA->m_width*3;
        for (int j = 0; j < height; j++)
        {
            memcpy( target_data, source_data, width*3 );
            source_data += source_step;
            target_data += target_step;
        }

        copiedPixels = true;
    }

    // Copy over the alpha channel from the original image
    if ( image.HasAlpha() )
    {
        if ( !HasAlpha() )
            InitAlpha();

        const unsigned char*
            alpha_source_data = image.GetAlpha() + xx + yy * image.GetWidth();
        const int source_step = image.GetWidth();

        unsigned char*
            alpha_target_data = GetAlpha() + (x + xx) + (y + yy) * M_IMGDATA->m_width;
        const int target_step = M_IMGDATA->m_width;

        switch (alphaBlend)
        {
            case wxIMAGE_ALPHA_BLEND_OVER:
            {
                // Copy just the alpha values.
                for (int j = 0; j < height; j++,
                     alpha_source_data += source_step,
                     alpha_target_data += target_step)
                {
                    memcpy(alpha_target_data, alpha_source_data, width);
                }
                break;
            }
            case wxIMAGE_ALPHA_BLEND_COMPOSE:
            {
                const unsigned char*
                    source_data = image.GetData() + 3 * (xx + yy * image.GetWidth());

                unsigned char*
                    target_data = GetData() + 3 * ((x + xx) + (y + yy) * M_IMGDATA->m_width);

                // Combine the alpha values but also apply alpha blending to
                // the pixels themselves while we copy them.
                for (int j = 0; j < height; j++,
                     alpha_source_data += source_step,
                     alpha_target_data += target_step,
                     source_data += 3 * source_step,
                     target_data += 3 * target_step)
                {
                    for (int i = 0; i < width; i++)
                    {
                        const float source_alpha = alpha_source_data[i] / 255.0f;
                        const float light_left = (alpha_target_data[i] / 255.0f) * (1.0f - source_alpha);
                        const float result_alpha = source_alpha + light_left;
                        alpha_target_data[i] = (unsigned char)((result_alpha * 255) + 0.5f);
                        for (int c = 3 * i; c < 3 * (i + 1); c++)
                        {
                            target_data[c] =
                                (unsigned char)(((source_data[c] * source_alpha +
                                    target_data[c] * light_left) /
                                result_alpha) + 0.5f);
                        }
                    }
                }

                copiedPixels = true;
                break;
            }
        }

    }

    // If we hadn't copied them yet we must need to take the mask of the image
    // being pasted into account.
    if (!copiedPixels)
    {
        const unsigned char* source_data = image.GetData() + 3 * (xx + yy * image.GetWidth());
        const int source_step = image.GetWidth() * 3;

        unsigned char* target_data = GetData() + 3 * ((x + xx) + (y + yy) * M_IMGDATA->m_width);
        int target_step = M_IMGDATA->m_width * 3;

        unsigned char* alpha_target_data = nullptr;
        const int target_alpha_step = M_IMGDATA->m_width;
        if (HasAlpha())
        {
            alpha_target_data = GetAlpha() + (x + xx) + (y + yy) * M_IMGDATA->m_width;
        }

        // The mask colours should only be taken into account if the mask is actually enabled
        if (!image.HasMask())
        {
            // Copy all pixels
            for (int j = 0; j < height; j++)
            {
                memcpy(target_data, source_data, width * 3);
                source_data += source_step;
                target_data += target_step;
                // Make all the copied pixels fully opaque
                if (alpha_target_data != nullptr)
                {
                    memset(alpha_target_data, wxALPHA_OPAQUE, width);
                    alpha_target_data += target_alpha_step;
                }
            }
        }
        else
        {
            // Copy all 'non masked' pixels
            const unsigned char r = image.GetMaskRed();
            const unsigned char g = image.GetMaskGreen();
            const unsigned char b = image.GetMaskBlue();

            for (int j = 0; j < height; j++)
            {
                for (int i = 0; i < width * 3; i += 3)
                {
                    if ((source_data[i] != r) ||
                        (source_data[i + 1] != g) ||
                        (source_data[i + 2] != b))
                    {
                        // Copy the non masked pixel
                        memcpy(target_data + i, source_data + i, 3);
                        if (alpha_target_data != nullptr) // Make the copied pixel fully opaque
                            alpha_target_data[i / 3] = wxALPHA_OPAQUE;
                    }
                }
                source_data += source_step;
                target_data += target_step;
                if (alpha_target_data != nullptr)
                    alpha_target_data += target_alpha_step;
            }
        }
    }
}

void wxImage::Replace( unsigned char r1, unsigned char g1, unsigned char b1,
                       unsigned char r2, unsigned char g2, unsigned char b2 )
{
    wxCHECK_RET( IsOk(), "invalid image" );

    AllocExclusive();

    unsigned char *data = GetData();

    const int w = GetWidth();
    const int h = GetHeight();

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            if ((data[0] == r1) && (data[1] == g1) && (data[2] == b1))
            {
                data[0] = r2;
                data[1] = g2;
                data[2] = b2;
            }
            data += 3;
        }
}

wxImage wxImage::ConvertToGreyscale() const
{
    return ConvertToGreyscale(0.299, 0.587, 0.114);
}

namespace
{

// red, green and blue are doubles in the range [0.0..1.0], they are used
// internally by DoMakeGrey() only.
class WeightValue
{
public:
    explicit WeightValue(double r = 0.0, double g = 0.0, double b = 0.0)
        : red(r), green(g), blue(b) {}
    double red;
    double green;
    double blue;
};

} // anonymous namespace

// TODO-C++11: Replace with a lambda function.
static void DoMakeGrey(wxImage *image, unsigned char *rgb, WeightValue weight)
{
    if ( !image->HasMask() || rgb[0] != image->GetMaskRed() ||
         rgb[1] != image->GetMaskGreen() || rgb[2] != image->GetMaskBlue() )
        wxColour::MakeGrey(rgb, rgb + 1, rgb + 2, weight.red, weight.green, weight.blue);
}

wxImage wxImage::ConvertToGreyscale(double weight_r, double weight_g, double weight_b) const
{
    wxImage image = *this;
    image.ApplyToAllPixels(&DoMakeGrey, WeightValue(weight_r, weight_g, weight_b));
    return image;
}

// TODO-C++11: Replace with a lambda function.
static void DoMakeMono([[maybe_unused]] wxImage *image, unsigned char *rgb, RGBValue rgbValue)
{
    const bool on = (rgb[0] == rgbValue.red) && (rgb[1] == rgbValue.green) && (rgb[2] == rgbValue.blue);
    wxColour::MakeMono(rgb, rgb + 1, rgb + 2, on);
}

wxImage wxImage::ConvertToMono(unsigned char r, unsigned char g, unsigned char b) const
{
    wxImage image = *this;

    if ( image.HasMask() )
    {
        if ( image.GetMaskRed() == r && image.GetMaskGreen() == g && image.GetMaskBlue() == b)
            image.SetMaskColour(255, 255, 255);
        else
            image.SetMaskColour(0, 0, 0);
    }

    image.ApplyToAllPixels(&DoMakeMono, RGBValue(r, g, b));
    return image;
}

// TODO-C++11: Replace with a lambda function.
static void DoMakeDisabled(wxImage *image, unsigned char *rgb, unsigned char brightness)
{
    if ( !image->HasMask() || rgb[0] != image->GetMaskRed() ||
         rgb[1] != image->GetMaskGreen() || rgb[2] != image->GetMaskBlue() )
        wxColour::MakeDisabled(rgb, rgb + 1, rgb + 2, brightness);
}

wxImage wxImage::ConvertToDisabled(unsigned char brightness) const
{
    wxImage image = *this;
    image.ApplyToAllPixels(&DoMakeDisabled, brightness);
    return image;
}

// TODO-C++11: Replace with a lambda function.
static void DoChangeLightness(wxImage *image, unsigned char *rgb, int alpha)
{
    if ( !image->HasMask() || rgb[0] != image->GetMaskRed() ||
         rgb[1] != image->GetMaskGreen() || rgb[2] != image->GetMaskBlue() )
        wxColour::ChangeLightness(rgb, rgb + 1, rgb + 2, alpha);
}

wxImage wxImage::ChangeLightness(int alpha) const
{
    wxASSERT(alpha >= 0 && alpha <= 200);
    wxImage image = *this;
    image.ApplyToAllPixels(&DoChangeLightness, alpha);
    return image;
}

int wxImage::GetWidth() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid image" );

    return M_IMGDATA->m_width;
}

int wxImage::GetHeight() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid image" );

    return M_IMGDATA->m_height;
}

wxBitmapType wxImage::GetType() const
{
    wxCHECK_MSG( IsOk(), wxBitmapType::Invalid, "invalid image" );

    return M_IMGDATA->m_type;
}

void wxImage::SetType(wxBitmapType type)
{
    wxCHECK_RET( IsOk(), "must create the image before setting its type");

    // type can be wxBitmapType::Invalid to reset the image type to default
    wxASSERT_MSG( type != wxBitmapType::Max, "invalid bitmap type" );

    M_IMGDATA->m_type = type;
}

long wxImage::XYToIndex(int x, int y) const
{
    if ( IsOk() &&
            x >= 0 && y >= 0 &&
                x < M_IMGDATA->m_width && y < M_IMGDATA->m_height )
    {
        return y*M_IMGDATA->m_width + x;
    }

    return -1;
}

void wxImage::SetRGB( int x, int y, unsigned char r, unsigned char g, unsigned char b )
{
    long pos = XYToIndex(x, y);
    wxCHECK_RET( pos != -1, "invalid image coordinates" );

    AllocExclusive();

    pos *= 3;

    M_IMGDATA->m_data[ pos   ] = r;
    M_IMGDATA->m_data[ pos+1 ] = g;
    M_IMGDATA->m_data[ pos+2 ] = b;
}

void wxImage::SetRGB( const wxRect& rect_, unsigned char r, unsigned char g, unsigned char b )
{
    wxCHECK_RET( IsOk(), "invalid image" );

    AllocExclusive();

    wxRect rect(rect_);
    const wxRect imageRect(0, 0, GetWidth(), GetHeight());
    if ( rect == wxRect() )
    {
        rect = imageRect;
    }
    else
    {
        wxCHECK_RET( imageRect.Contains(rect.GetTopLeft()) &&
                     imageRect.Contains(rect.GetBottomRight()),
                     "invalid bounding rectangle" );
    }

    const int x1 = rect.GetLeft();
    const int y1 = rect.GetTop();
    const int x2 = rect.GetRight() + 1;
    const int y2 = rect.GetBottom() + 1;

    int x, y, width = GetWidth();
    for (y = y1; y < y2; y++)
    {
        unsigned char* data = M_IMGDATA->m_data + (y*width + x1)*3;
        for (x = x1; x < x2; x++)
        {
            *data++ = r;
            *data++ = g;
            *data++ = b;
        }
    }
}

unsigned char wxImage::GetRed( int x, int y ) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, "invalid image coordinates" );

    pos *= 3;

    return M_IMGDATA->m_data[pos];
}

unsigned char wxImage::GetGreen( int x, int y ) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, "invalid image coordinates" );

    pos *= 3;

    return M_IMGDATA->m_data[pos+1];
}

unsigned char wxImage::GetBlue( int x, int y ) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, "invalid image coordinates" );

    pos *= 3;

    return M_IMGDATA->m_data[pos+2];
}

bool wxImage::IsOk() const
{
    // image of 0 width or height can't be considered ok - at least because it
    // causes crashes in ConvertToBitmap() if we don't catch it in time
    const wxImageRefData *data = M_IMGDATA;
    return data && data->m_ok && data->m_width && data->m_height;
}

unsigned char *wxImage::GetData() const
{
    wxCHECK_MSG( IsOk(), (unsigned char *)nullptr, "invalid image" );

    return M_IMGDATA->m_data;
}

void wxImage::SetData( unsigned char *data, bool static_data  )
{
    wxCHECK_RET( IsOk(), "invalid image" );

    wxImageRefData *newRefData = new wxImageRefData();

    newRefData->m_width = M_IMGDATA->m_width;
    newRefData->m_height = M_IMGDATA->m_height;
    newRefData->m_data = data;
    newRefData->m_ok = true;
    newRefData->m_maskRed = M_IMGDATA->m_maskRed;
    newRefData->m_maskGreen = M_IMGDATA->m_maskGreen;
    newRefData->m_maskBlue = M_IMGDATA->m_maskBlue;
    newRefData->m_hasMask = M_IMGDATA->m_hasMask;
    newRefData->m_static = static_data;

    UnRef();

    m_refData = newRefData;
}

void wxImage::SetData( unsigned char *data, int new_width, int new_height, bool static_data )
{
    wxImageRefData *newRefData = new wxImageRefData();

    if (m_refData)
    {
        newRefData->m_width = new_width;
        newRefData->m_height = new_height;
        newRefData->m_data = data;
        newRefData->m_ok = true;
        newRefData->m_maskRed = M_IMGDATA->m_maskRed;
        newRefData->m_maskGreen = M_IMGDATA->m_maskGreen;
        newRefData->m_maskBlue = M_IMGDATA->m_maskBlue;
        newRefData->m_hasMask = M_IMGDATA->m_hasMask;
    }
    else
    {
        newRefData->m_width = new_width;
        newRefData->m_height = new_height;
        newRefData->m_data = data;
        newRefData->m_ok = true;
    }
    newRefData->m_static = static_data;

    UnRef();

    m_refData = newRefData;
}

// ----------------------------------------------------------------------------
// alpha channel support
// ----------------------------------------------------------------------------

void wxImage::SetAlpha(int x, int y, unsigned char alpha)
{
    wxCHECK_RET( HasAlpha(), "no alpha channel" );

    long pos = XYToIndex(x, y);
    wxCHECK_RET( pos != -1, "invalid image coordinates" );

    AllocExclusive();

    M_IMGDATA->m_alpha[pos] = alpha;
}

unsigned char wxImage::GetAlpha(int x, int y) const
{
    wxCHECK_MSG( HasAlpha(), 0, "no alpha channel" );

    const long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, "invalid image coordinates" );

    return M_IMGDATA->m_alpha[pos];
}

bool
wxImage::ConvertColourToAlpha(unsigned char r, unsigned char g, unsigned char b)
{
    SetAlpha(nullptr);

    const int w = M_IMGDATA->m_width;
    const int h = M_IMGDATA->m_height;

    unsigned char *alpha = GetAlpha();
    unsigned char *data = GetData();

    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            *alpha++ = *data;
            *data++ = r;
            *data++ = g;
            *data++ = b;
        }
    }

    return true;
}

void wxImage::SetAlpha( unsigned char *alpha, bool static_data )
{
    wxCHECK_RET( IsOk(), "invalid image" );

    AllocExclusive();

    if ( !alpha )
    {
        alpha = (unsigned char *)malloc(M_IMGDATA->m_width*M_IMGDATA->m_height);
    }

    if( !M_IMGDATA->m_staticAlpha )
        free(M_IMGDATA->m_alpha);

    M_IMGDATA->m_alpha = alpha;
    M_IMGDATA->m_staticAlpha = static_data;
}

unsigned char *wxImage::GetAlpha() const
{
    wxCHECK_MSG( IsOk(), (unsigned char *)nullptr, "invalid image" );

    return M_IMGDATA->m_alpha;
}

void wxImage::InitAlpha()
{
    wxCHECK_RET( !HasAlpha(), "image already has an alpha channel" );

    // initialize memory for alpha channel
    SetAlpha();

    unsigned char *alpha = M_IMGDATA->m_alpha;
    const size_t lenAlpha = M_IMGDATA->m_width * M_IMGDATA->m_height;

    if ( HasMask() )
    {
        // use the mask to initialize the alpha channel.
        const unsigned char * const alphaEnd = alpha + lenAlpha;

        const unsigned char mr = M_IMGDATA->m_maskRed;
        const unsigned char mg = M_IMGDATA->m_maskGreen;
        const unsigned char mb = M_IMGDATA->m_maskBlue;
        for ( unsigned char *src = M_IMGDATA->m_data;
              alpha < alphaEnd;
              src += 3, alpha++ )
        {
            *alpha = (src[0] == mr && src[1] == mg && src[2] == mb)
                            ? wxIMAGE_ALPHA_TRANSPARENT
                            : wxIMAGE_ALPHA_OPAQUE;
        }

        M_IMGDATA->m_hasMask = false;
    }
    else // no mask
    {
        // make the image fully opaque
        memset(alpha, wxIMAGE_ALPHA_OPAQUE, lenAlpha);
    }
}

void wxImage::ClearAlpha()
{
    wxCHECK_RET( HasAlpha(), "image already doesn't have an alpha channel" );

    AllocExclusive();

    if ( !M_IMGDATA->m_staticAlpha )
        free( M_IMGDATA->m_alpha );

    M_IMGDATA->m_alpha = nullptr;
}


// ----------------------------------------------------------------------------
// mask support
// ----------------------------------------------------------------------------

void wxImage::SetMaskColour( unsigned char r, unsigned char g, unsigned char b )
{
    wxCHECK_RET( IsOk(), "invalid image" );

    AllocExclusive();

    M_IMGDATA->m_maskRed = r;
    M_IMGDATA->m_maskGreen = g;
    M_IMGDATA->m_maskBlue = b;
    M_IMGDATA->m_hasMask = true;
}

bool wxImage::GetOrFindMaskColour( unsigned char *r, unsigned char *g, unsigned char *b ) const
{
    wxCHECK_MSG( IsOk(), false, "invalid image" );

    if (M_IMGDATA->m_hasMask)
    {
        if (r) *r = M_IMGDATA->m_maskRed;
        if (g) *g = M_IMGDATA->m_maskGreen;
        if (b) *b = M_IMGDATA->m_maskBlue;
        return true;
    }
    else
    {
        FindFirstUnusedColour(r, g, b);
        return false;
    }
}

unsigned char wxImage::GetMaskRed() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid image" );

    return M_IMGDATA->m_maskRed;
}

unsigned char wxImage::GetMaskGreen() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid image" );

    return M_IMGDATA->m_maskGreen;
}

unsigned char wxImage::GetMaskBlue() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid image" );

    return M_IMGDATA->m_maskBlue;
}

void wxImage::SetMask( bool mask )
{
    wxCHECK_RET( IsOk(), "invalid image" );

    AllocExclusive();

    M_IMGDATA->m_hasMask = mask;
}

bool wxImage::HasMask() const
{
    wxCHECK_MSG( IsOk(), false, "invalid image" );

    return M_IMGDATA->m_hasMask;
}

bool wxImage::IsTransparent(int x, int y, unsigned char threshold) const
{
    const long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, false, "invalid image coordinates" );

    // check mask
    if ( M_IMGDATA->m_hasMask )
    {
        const unsigned char *p = M_IMGDATA->m_data + 3*pos;
        if ( p[0] == M_IMGDATA->m_maskRed &&
                p[1] == M_IMGDATA->m_maskGreen &&
                    p[2] == M_IMGDATA->m_maskBlue )
        {
            return true;
        }
    }

    // then check alpha
    if ( M_IMGDATA->m_alpha )
    {
        if ( M_IMGDATA->m_alpha[pos] < threshold )
        {
            // transparent enough
            return true;
        }
    }

    // not transparent
    return false;
}

bool wxImage::SetMaskFromImage(const wxImage& mask,
                               unsigned char mr, unsigned char mg, unsigned char mb)
{
    // check that the images are the same size
    if ( (M_IMGDATA->m_height != mask.GetHeight() ) || (M_IMGDATA->m_width != mask.GetWidth () ) )
    {
        wxLogError( _("Image and mask have different sizes.") );
        return false;
    }

    // find unused colour
    unsigned char r,g,b ;
    if (!FindFirstUnusedColour(&r, &g, &b))
    {
        wxLogError( _("No unused colour in image being masked.") );
        return false ;
    }

    AllocExclusive();

    unsigned char *imgdata = GetData();
    unsigned char *maskdata = mask.GetData();

    const int w = GetWidth();
    const int h = GetHeight();

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if ((maskdata[0] == mr) && (maskdata[1]  == mg) && (maskdata[2] == mb))
            {
                imgdata[0] = r;
                imgdata[1] = g;
                imgdata[2] = b;
            }
            imgdata  += 3;
            maskdata += 3;
        }
    }

    SetMaskColour(r, g, b);
    SetMask(true);

    return true;
}

bool wxImage::ConvertAlphaToMask(unsigned char threshold)
{
    if ( !HasAlpha() )
        return false;

    unsigned char mr, mg, mb;
    if ( !FindFirstUnusedColour(&mr, &mg, &mb) )
    {
        wxLogError( _("No unused colour in image being masked.") );
        return false;
    }

    return ConvertAlphaToMask(mr, mg, mb, threshold);
}

bool wxImage::ConvertAlphaToMask(unsigned char mr,
                                 unsigned char mg,
                                 unsigned char mb,
                                 unsigned char threshold)
{
    if ( !HasAlpha() )
        return false;

    AllocExclusive();

    SetMask(true);
    SetMaskColour(mr, mg, mb);

    unsigned char *imgdata = GetData();
    unsigned char *alphadata = GetAlpha();

    const int w = GetWidth();
    const int h = GetHeight();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++, imgdata += 3, alphadata++)
        {
            if (*alphadata < threshold)
            {
                imgdata[0] = mr;
                imgdata[1] = mg;
                imgdata[2] = mb;
            }
        }
    }

    if ( !M_IMGDATA->m_staticAlpha )
        free(M_IMGDATA->m_alpha);

    M_IMGDATA->m_alpha = nullptr;
    M_IMGDATA->m_staticAlpha = false;

    return true;
}

// ----------------------------------------------------------------------------
// Palette functions
// ----------------------------------------------------------------------------

#if wxUSE_PALETTE

bool wxImage::HasPalette() const
{
    if (!IsOk())
        return false;

    return M_IMGDATA->m_palette.IsOk();
}

const wxPalette& wxImage::GetPalette() const
{
    wxCHECK_MSG( IsOk(), wxNullPalette, "invalid image" );

    return M_IMGDATA->m_palette;
}

void wxImage::SetPalette(const wxPalette& palette)
{
    wxCHECK_RET( IsOk(), "invalid image" );

    AllocExclusive();

    M_IMGDATA->m_palette = palette;
}

#endif // wxUSE_PALETTE

// ----------------------------------------------------------------------------
// Option functions (arbitrary name/value mapping)
// ----------------------------------------------------------------------------

void wxImage::SetOption(const std::string& name, const std::string& value)
{
    AllocExclusive();

    const auto match = std::ranges::find_if(M_IMGDATA->m_optionNames,
    [name](const auto& optname){
        return wx::utils::IsSameAsNoCase(optname, name);
    });

    if ( match == M_IMGDATA->m_optionNames.end() )
    {
        M_IMGDATA->m_optionNames.push_back(name);
        M_IMGDATA->m_optionValues.push_back(value);
    }
    else
    {
        const auto idx = std::distance(M_IMGDATA->m_optionNames.begin(), match);
        M_IMGDATA->m_optionNames[idx] = name;
        M_IMGDATA->m_optionValues[idx] = value;
    }
}

void wxImage::SetOption(const std::string& name, int value)
{
    std::string valStr{fmt::format("{:d}", value)};
    SetOption(name, valStr);
}

std::string wxImage::GetOption(const std::string& name) const
{
    if ( !M_IMGDATA )
        return {};

    const auto match = std::ranges::find_if(M_IMGDATA->m_optionNames,
    [name](const auto& optname){
        return wx::utils::IsSameAsNoCase(optname, name);
    });

    if ( match == M_IMGDATA->m_optionNames.end() )
        return {};
    else
    {
        const auto idx = std::distance(M_IMGDATA->m_optionNames.begin(), match);
        return M_IMGDATA->m_optionValues[idx];
    }
}

int wxImage::GetOptionInt(const std::string& name) const
{
    return wxAtoi(GetOption(name));
}

bool wxImage::HasOption(const std::string& name) const
{
    if(M_IMGDATA)
    {
        return M_IMGDATA->m_optionNames.end() != std::ranges::find_if(M_IMGDATA->m_optionNames, 
        [name](const auto& optname){
            return wx::utils::IsSameAsNoCase(optname, name);
        });
    }

    return false;
}

// ----------------------------------------------------------------------------
// image I/O
// ----------------------------------------------------------------------------

/* static */
void wxImage::SetDefaultLoadFlags(unsigned int flags)
{
    wxImageRefData::sm_defaultLoadFlags = flags;
}

/* static */
unsigned int wxImage::GetDefaultLoadFlags()
{
    return wxImageRefData::sm_defaultLoadFlags;
}

void wxImage::SetLoadFlags(unsigned int flags)
{
    AllocExclusive();

    M_IMGDATA->m_loadFlags = flags;
}

unsigned int wxImage::GetLoadFlags() const
{
    return M_IMGDATA ? M_IMGDATA->m_loadFlags : wxImageRefData::sm_defaultLoadFlags;
}

#ifdef HAS_LOAD_FROM_RESOURCE

static wxImage LoadImageFromResource(const std::string &name, wxBitmapType type)
{
    using msw::utils::unique_bitmap;

    unique_bitmap hBitmap;
    unique_bitmap hMask;

    if ( type == wxBitmapType::BMP_Resource )
    {
        hBitmap = unique_bitmap( ::LoadBitmapW(wxGetInstance(), boost::nowide::widen(name).c_str()) );

        if ( !hBitmap )
        {
            wxLogError(_("Failed to load bitmap \"%s\" from resources."), name);
        }
    }
    else if ( type == wxBitmapType::ICO_Resource )
    {
        const WXHICON hIcon = ::LoadIconW(wxGetInstance(), boost::nowide::widen(name).c_str());

        if ( !hIcon )
        {
            wxLogError(_("Failed to load icon \"%s\" from resources."), name);
        }
        else
        {
            AutoIconInfo info;
            if ( !info.GetFrom(hIcon) )
                return {};

            hBitmap = unique_bitmap(info.hbmColor);
            hMask = unique_bitmap(info.hbmMask);

            // Reset the fields to prevent them from being destroyed, we took
            // ownership of them.
            info.hbmColor =
            info.hbmMask = nullptr;
        }
    }
    else if ( type == wxBitmapType::CUR_Resource )
    {
        wxLogDebug("Loading cursors from resources is not implemented.");
    }
    else
    {
        wxFAIL_MSG("Invalid bitmap resource type.");
    }

    if ( !hBitmap )
        return {};

    wxImage image = wxDIB(hBitmap.get()).ConvertToImage();
    
    if ( hMask )
    {
        const wxImage mask = wxDIB(hMask.get()).ConvertToImage();
        image.SetMaskFromImage(mask, 255, 255, 255);
    }
    else
    {
        // Light gray colour is a default mask
        image.SetMaskColour(0xc0, 0xc0, 0xc0);
    }

    // We could have already loaded alpha from the resources, but if not,
    // initialize it now using the mask.
    if ( !image.HasAlpha() )
        image.InitAlpha();

    return image;
}

#endif // HAS_LOAD_FROM_RESOURCE

bool wxImage::LoadFile( const std::string& filename,
                        wxBitmapType type,
                        int WXUNUSED_UNLESS_STREAMS(index) )
{
#ifdef HAS_LOAD_FROM_RESOURCE
    if (   type == wxBitmapType::BMP_Resource
        || type == wxBitmapType::ICO_Resource
        || type == wxBitmapType::CUR_Resource)
    {
        const wxImage image = LoadImageFromResource(filename, type);
        if ( image.IsOk() )
        {
            *this = image;
            return true;
        }
    }
#endif // HAS_LOAD_FROM_RESOURCE

#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(filename);
    if ( stream.IsOk() )
    {
        wxBufferedInputStream bstream( stream );
        if ( LoadFile(bstream, type, index) )
            return true;
    }

    wxLogError(_("Failed to load image from file \"%s\"."), filename);
#endif // HAS_FILE_STREAMS

    return false;
}

bool wxImage::LoadFile( const std::string& WXUNUSED_UNLESS_STREAMS(filename),
                        const std::string& WXUNUSED_UNLESS_STREAMS(mimetype),
                        int WXUNUSED_UNLESS_STREAMS(index) )
{
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(filename);
    if ( stream.IsOk() )
    {
        wxBufferedInputStream bstream( stream );
        if ( LoadFile(bstream, mimetype, index) )
            return true;
    }

    wxLogError(_("Failed to load image from file \"%s\"."), filename);
#endif // HAS_FILE_STREAMS

    return false;
}


bool wxImage::SaveFile( const std::string& filename ) const
{
    std::string ext = wx::utils::ToLowerCopy(wx::utils::AfterLast(filename, '.'));

    wxImageHandler *handler = FindHandler(ext, wxBitmapType::Any);
    if ( !handler)
    {
       wxLogError(_("Can't save image to file '%s': unknown extension."),
                  filename);
       return false;
    }

    return SaveFile(filename, handler->GetType());
}

bool wxImage::SaveFile( const std::string& WXUNUSED_UNLESS_STREAMS(filename),
                        wxBitmapType WXUNUSED_UNLESS_STREAMS(type) ) const
{
#if HAS_FILE_STREAMS
    wxCHECK_MSG( IsOk(), false, "invalid image" );

    const_cast<wxImage*>(this)->SetOption(wxIMAGE_OPTION_FILENAME, filename);

    wxImageFileOutputStream stream(filename);

    if ( stream.IsOk() )
    {
        wxBufferedOutputStream bstream( stream );
        return SaveFile(bstream, type);
    }
#endif // HAS_FILE_STREAMS

    return false;
}

bool wxImage::SaveFile( const std::string& WXUNUSED_UNLESS_STREAMS(filename),
                        const std::string& WXUNUSED_UNLESS_STREAMS(mimetype) ) const
{
#if HAS_FILE_STREAMS
    wxCHECK_MSG( IsOk(), false, "invalid image" );

    const_cast<wxImage*>(this)->SetOption(wxIMAGE_OPTION_FILENAME, filename);

    wxImageFileOutputStream stream(filename);

    if ( stream.IsOk() )
    {
        wxBufferedOutputStream bstream( stream );
        return SaveFile(bstream, mimetype);
    }
#endif // HAS_FILE_STREAMS

    return false;
}

bool wxImage::CanRead( const std::string& WXUNUSED_UNLESS_STREAMS(name) )
{
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(name);
    return CanRead(stream);
#else
    return false;
#endif
}

int wxImage::GetImageCount( const std::string& WXUNUSED_UNLESS_STREAMS(name),
                            wxBitmapType WXUNUSED_UNLESS_STREAMS(type) )
{
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(name);
    if (stream.IsOk())
        return GetImageCount(stream, type);
#endif

  return 0;
}

#if wxUSE_STREAMS

bool wxImage::CanRead( wxInputStream &stream )
{
    const wxList& list = GetHandlers();

    for ( wxList::compatibility_iterator node = list.GetFirst(); node; node = node->GetNext() )
    {
        wxImageHandler *handler=(wxImageHandler*)node->GetData();
        if (handler->CanRead( stream ))
            return true;
    }

    return false;
}

int wxImage::GetImageCount( wxInputStream &stream, wxBitmapType type )
{
    wxImageHandler *handler;

    if ( type == wxBitmapType::Any )
    {
        const wxList& list = GetHandlers();

        for ( wxList::compatibility_iterator node = list.GetFirst();
              node;
              node = node->GetNext() )
        {
             handler = (wxImageHandler*)node->GetData();
             if ( handler->CanRead(stream) )
             {
                 const int count = handler->GetImageCount(stream);
                 if ( count >= 0 )
                     return count;
             }

        }

        wxLogWarning(_("No handler found for image type."));
        return 0;
    }

    handler = FindHandler(type);

    if ( !handler )
    {
        wxLogWarning(_("No image handler for type %d defined."), type);
        return false;
    }

    if ( handler->CanRead(stream) )
    {
        return handler->GetImageCount(stream);
    }
    else
    {
        wxLogError(_("Image file is not of type %d."), type);
        return 0;
    }
}

bool wxImage::DoLoad(wxImageHandler& handler, wxInputStream& stream, int index)
{
    // save the options values which can be clobbered by the handler (e.g. many
    // of them call Destroy() before trying to load the file)
    const unsigned maxWidth = GetOptionInt(wxIMAGE_OPTION_MAX_WIDTH),
                   maxHeight = GetOptionInt(wxIMAGE_OPTION_MAX_HEIGHT);

    // Preserve the original stream position if possible to rewind back to it
    // if we failed to load the file -- maybe the next handler that we try can
    // succeed after us then.
    wxFileOffset posOld = wxInvalidOffset;
    if ( stream.IsSeekable() )
        posOld = stream.TellI();

    if ( !handler.LoadFile(this, stream,
                           (M_IMGDATA->m_loadFlags & Load_Verbose) != 0, index) )
    {
        if ( posOld != wxInvalidOffset )
            stream.SeekI(posOld);

        return false;
    }

    // rescale the image to the specified size if needed
    if ( maxWidth || maxHeight )
    {
        const unsigned widthOrig = GetWidth(),
                       heightOrig = GetHeight();

        // this uses the same (trivial) algorithm as the JPEG handler
        unsigned width = widthOrig,
                 height = heightOrig;
        while ( (maxWidth && width > maxWidth) ||
                    (maxHeight && height > maxHeight) )
        {
            width /= 2;
            height /= 2;
        }

        if ( width != widthOrig || height != heightOrig )
        {
            // get the original size if it was set by the image handler
            // but also in order to restore it after Rescale
            int widthOrigOption = GetOptionInt(wxIMAGE_OPTION_ORIGINAL_WIDTH),
                heightOrigOption = GetOptionInt(wxIMAGE_OPTION_ORIGINAL_HEIGHT);

            Rescale(width, height, wxImageResizeQuality::High);

            SetOption(wxIMAGE_OPTION_ORIGINAL_WIDTH, widthOrigOption ? widthOrigOption : widthOrig);
            SetOption(wxIMAGE_OPTION_ORIGINAL_HEIGHT, heightOrigOption ? heightOrigOption : heightOrig);
        }
    }

    // Set this after Rescale, which currently does not preserve it
    M_IMGDATA->m_type = handler.GetType();

    return true;
}

bool wxImage::LoadFile( wxInputStream& stream, wxBitmapType type, int index )
{
    AllocExclusive();

    wxImageHandler *handler;

    // do we issue warning/error messages?
    const bool verbose = M_IMGDATA->m_loadFlags & Load_Verbose;

    if ( type == wxBitmapType::Any )
    {
        if ( !stream.IsSeekable() )
        {
            if ( verbose )
            {
                // The error message about image data format being unknown below
                // would be misleading in this case as we are not even going to try
                // any handlers because CanRead() never does anything for not
                // seekable stream, so try to be more precise here.
                wxLogError(_("Can't automatically determine the image format "
                             "for non-seekable input."));
            }
            return false;
        }

        const wxList& list = GetHandlers();
        for ( wxList::compatibility_iterator node = list.GetFirst();
              node;
              node = node->GetNext() )
        {
             handler = (wxImageHandler*)node->GetData();
             if ( handler->CanRead(stream) && DoLoad(*handler, stream, index) )
                 return true;
        }

        if ( verbose )
        {
            wxLogWarning( _("Unknown image data format.") );
        }

        return false;
    }
    //else: have specific type

    handler = FindHandler(type);
    if ( !handler )
    {
        if ( verbose )
        {
            wxLogWarning( _("No image handler for type %d defined."), type );
        }
        return false;
    }

    if ( stream.IsSeekable() && !handler->CanRead(stream) )
    {
        if ( verbose )
        {
            wxLogError(_("This is not a %s."), handler->GetName());
        }
        return false;
    }

    return DoLoad(*handler, stream, index);
}

bool wxImage::LoadFile( wxInputStream& stream, const std::string& mimetype, int index )
{
    UnRef();

    m_refData = new wxImageRefData;

    wxImageHandler *handler = FindHandlerMime(mimetype);

    // do we issue warning/error messages?
    const bool verbose = M_IMGDATA->m_loadFlags & Load_Verbose;

    if ( !handler )
    {
        if ( verbose )
        {
            wxLogWarning( _("No image handler for type %s defined."), mimetype );
        }
        return false;
    }

    if ( stream.IsSeekable() && !handler->CanRead(stream) )
    {
        if ( verbose )
        {
            wxLogError(_("Image is not of type %s."), mimetype);
        }
        return false;
    }

    return DoLoad(*handler, stream, index);
}

bool wxImage::DoSave(wxImageHandler& handler, wxOutputStream& stream) const
{
    wxImage * const self = const_cast<wxImage *>(this);
    if ( !handler.SaveFile(self, stream) )
        return false;

    M_IMGDATA->m_type = handler.GetType();
    return true;
}

bool wxImage::SaveFile( wxOutputStream& stream, wxBitmapType type ) const
{
    wxCHECK_MSG( IsOk(), false, "invalid image" );

    wxImageHandler *handler = FindHandler(type);
    if ( !handler )
    {
        wxLogWarning( _("No image handler for type %d defined."), type );
        return false;
    }

    return DoSave(*handler, stream);
}

bool wxImage::SaveFile( wxOutputStream& stream, const std::string& mimetype ) const
{
    wxCHECK_MSG( IsOk(), false, "invalid image" );

    wxImageHandler *handler = FindHandlerMime(mimetype);
    if ( !handler )
    {
        wxLogWarning( _("No image handler for type %s defined."), mimetype );
        return false;
    }

    return DoSave(*handler, stream);
}

#endif // wxUSE_STREAMS

// ----------------------------------------------------------------------------
// image I/O handlers
// ----------------------------------------------------------------------------

void wxImage::AddHandler( wxImageHandler *handler )
{
    // Check for an existing handler of the type being added.
    if (FindHandler( handler->GetType() ) == nullptr)
    {
        sm_handlers.Append( handler );
    }
    else
    {
        // This is not documented behaviour, merely the simplest 'fix'
        // for preventing duplicate additions.  If someone ever has
        // a good reason to add and remove duplicate handlers (and they
        // may) we should probably refcount the duplicates.
        //   also an issue in InsertHandler below.

        wxLogDebug( "Adding duplicate image handler for '%s'",
                    handler->GetName().c_str() );
        delete handler;
    }
}

void wxImage::InsertHandler( wxImageHandler *handler )
{
    // Check for an existing handler of the type being added.
    if (FindHandler( handler->GetType() ) == nullptr)
    {
        sm_handlers.Insert( handler );
    }
    else
    {
        // see AddHandler for additional comments.
        wxLogDebug( "Inserting duplicate image handler for '%s'",
                    handler->GetName().c_str() );
        delete handler;
    }
}

bool wxImage::RemoveHandler( const std::string& name )
{
    wxImageHandler *handler = FindHandler(name);
    if (handler)
    {
        sm_handlers.DeleteObject(handler);
        delete handler;
        return true;
    }
    else
        return false;
}

wxImageHandler *wxImage::FindHandler( const std::string& name )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler*)node->GetData();
        if (handler->GetName().compare(name) == 0) return handler;

        node = node->GetNext();
    }
    return nullptr;
}

wxImageHandler *wxImage::FindHandler( const std::string& extension, wxBitmapType bitmapType )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler*)node->GetData();
        if ((bitmapType == wxBitmapType::Any) || (handler->GetType() == bitmapType))
        {
            if (handler->GetExtension() == extension)
                return handler;

            const auto ext_idx = std::ranges::find_if(handler->GetAltExtensions(),
                [extension](const auto& alt_extension) { return wx::utils::IsSameAsNoCase(alt_extension, extension); });

            if (ext_idx != handler->GetAltExtensions().end())
                return handler;
        }
        node = node->GetNext();
    }
    return nullptr;
}

wxImageHandler *wxImage::FindHandler(wxBitmapType bitmapType )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler *)node->GetData();
        if (handler->GetType() == bitmapType) return handler;
        node = node->GetNext();
    }
    return nullptr;
}

wxImageHandler *wxImage::FindHandlerMime( const std::string& mimetype )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler *)node->GetData();
        if (wx::utils::IsSameAsNoCase(mimetype, handler->GetMimeType())) return handler;
        node = node->GetNext();
    }
    return nullptr;
}

void wxImage::InitStandardHandlers()
{
#if wxUSE_STREAMS
    AddHandler(new wxBMPHandler);
#endif // wxUSE_STREAMS
}

void wxImage::CleanUpHandlers()
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler *)node->GetData();
        wxList::compatibility_iterator next = node->GetNext();
        delete handler;
        node = next;
    }

    sm_handlers.Clear();
}

std::string wxImage::GetImageExtWildcard()
{
    wxString fmts;

    wxList& Handlers = wxImage::GetHandlers();
    wxList::compatibility_iterator Node = Handlers.GetFirst();
    while ( Node )
    {
        wxImageHandler* Handler = (wxImageHandler*)Node->GetData();
        fmts += "*." + Handler->GetExtension();
        for (size_t i = 0; i < Handler->GetAltExtensions().size(); i++)
            fmts += ";*." + Handler->GetAltExtensions()[i];
        Node = Node->GetNext();
        if ( Node ) fmts += ";";
    }

    return "(" + fmts + ")|" + fmts;
}

wxImage::HSVValue wxImage::RGBtoHSV(const RGBValue& rgb)
{
    const double red = rgb.red / 255.0,
                 green = rgb.green / 255.0,
                 blue = rgb.blue / 255.0;

    // find the min and max intensity (and remember which one was it for the
    // latter)
    double minimumRGB = red;
    if ( green < minimumRGB )
        minimumRGB = green;
    if ( blue < minimumRGB )
        minimumRGB = blue;

    enum { RED, GREEN, BLUE } chMax = RED;
    double maximumRGB = red;
    if ( green > maximumRGB )
    {
        chMax = GREEN;
        maximumRGB = green;
    }
    if ( blue > maximumRGB )
    {
        chMax = BLUE;
        maximumRGB = blue;
    }

    const double value = maximumRGB;

    double hue = 0.0, saturation;
    const double deltaRGB = maximumRGB - minimumRGB;
    // FIXME: Double equality
    if ( deltaRGB == 0.0 )
    {
        // Gray has no color
        hue = 0.0;
        saturation = 0.0;
    }
    else
    {
        switch ( chMax )
        {
            case RED:
                hue = (green - blue) / deltaRGB;
                break;

            case GREEN:
                hue = 2.0 + (blue - red) / deltaRGB;
                break;

            case BLUE:
                hue = 4.0 + (red - green) / deltaRGB;
                break;
        }

        hue /= 6.0;

        if ( hue < 0.0 )
            hue += 1.0;

        saturation = deltaRGB / maximumRGB;
    }

    return {hue, saturation, value};
}

RGBValue wxImage::HSVtoRGB(const HSVValue& hsv)
{
    double red{};
    double green{};
    double blue{};

    // FIXME: Double equality
    if ( hsv.saturation == 0.0 )
    {
        // Grey
        red = hsv.value;
        green = hsv.value;
        blue = hsv.value;
    }
    else // not grey
    {
        const double hue = hsv.hue * 6.0;      // sector 0 to 5
        const auto i = wx::narrow_cast<int>(std::floor(hue));
        const double f = hue - i;          // fractional part of h
        const double p = hsv.value * (1.0 - hsv.saturation);

        switch (i)
        {
            case 0:
                red = hsv.value;
                green = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                blue = p;
                break;

            case 1:
                red = hsv.value * (1.0 - hsv.saturation * f);
                green = hsv.value;
                blue = p;
                break;

            case 2:
                red = p;
                green = hsv.value;
                blue = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                break;

            case 3:
                red = p;
                green = hsv.value * (1.0 - hsv.saturation * f);
                blue = hsv.value;
                break;

            case 4:
                red = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                green = p;
                blue = hsv.value;
                break;

            default:    // case 5:
                red = hsv.value;
                green = p;
                blue = hsv.value * (1.0 - hsv.saturation * f);
                break;
        }
    }

    return {wx::narrow_cast<unsigned char>(std::lround(red * 255.0)),
            wx::narrow_cast<unsigned char>(std::lround(green * 255.0)),
            wx::narrow_cast<unsigned char>(std::lround(blue * 255.0))};
}

// TODO-C++11: Replace with a lambda function.
static void DoRotateHue([[maybe_unused]] wxImage *image, unsigned char *rgb, double angle)
{
    RGBValue rgbValue(rgb[0], rgb[1], rgb[2]);
    wxImage::HSVValue hsvValue = wxImage::RGBtoHSV(rgbValue);

    hsvValue.hue = hsvValue.hue + angle;

    if (hsvValue.hue > 1.0)
        hsvValue.hue = hsvValue.hue - 1.0;
    else if (hsvValue.hue < 0.0)
        hsvValue.hue = hsvValue.hue + 1.0;

    rgbValue = wxImage::HSVtoRGB(hsvValue);
    rgb[0] = rgbValue.red;
    rgb[1] = rgbValue.green;
    rgb[2] = rgbValue.blue;
}

// Rotates the hue of each pixel in the image by angle, which is a double in the
// range [-1.0..+1.0], where -1.0 corresponds to -360 degrees and +1.0 corresponds
// to +360 degrees.
void wxImage::RotateHue(double angle)
{
    // FIXME: Double equality
    if ( angle == 0.0 )
        return;

    wxASSERT(angle >= -1.0 && angle <= 1.0);
    ApplyToAllPixels(&DoRotateHue, angle);
}

// TODO-C++11: Replace with a lambda function.
static void DoChangeSaturation([[maybe_unused]] wxImage *image, unsigned char *rgb, double factor)
{
    RGBValue rgbValue(rgb[0], rgb[1], rgb[2]);
    wxImage::HSVValue hsvValue = wxImage::RGBtoHSV(rgbValue);

    hsvValue.saturation += hsvValue.saturation * factor;

    if (hsvValue.saturation > 1.0)
        hsvValue.saturation = 1.0;
    else if (hsvValue.saturation < 0.0)
        hsvValue.saturation = 0.0;

    rgbValue = wxImage::HSVtoRGB(hsvValue);
    rgb[0] = rgbValue.red;
    rgb[1] = rgbValue.green;
    rgb[2] = rgbValue.blue;
}

// Changes the saturation of each pixel in the image. factor is a double in the
// range [-1.0..+1.0], where -1.0 corresponds to -100 percent and +1.0 corresponds
// to +100 percent.
void wxImage::ChangeSaturation(double factor)
{
    // FIXME: Double equality.
    if ( factor == 0.0 )
        return;

    wxASSERT(factor >= -1.0 && factor <= 1.0);
    ApplyToAllPixels(&DoChangeSaturation, factor);
}

// TODO-C++11: Replace with a lambda function.
static void DoChangeBrightness([[maybe_unused]] wxImage *image, unsigned char *rgb, double factor)
{
    RGBValue rgbValue(rgb[0], rgb[1], rgb[2]);
    wxImage::HSVValue hsvValue = wxImage::RGBtoHSV(rgbValue);

    hsvValue.value += hsvValue.value * factor;

    if (hsvValue.value > 1.0)
        hsvValue.value = 1.0;
    else if (hsvValue.value < 0.0)
        hsvValue.value = 0.0;

    rgbValue = wxImage::HSVtoRGB(hsvValue);
    rgb[0] = rgbValue.red;
    rgb[1] = rgbValue.green;
    rgb[2] = rgbValue.blue;
}

// Changes the brightness (value) of each pixel in the image. factor is a double
// in the range [-1.0..+1.0], where -1.0 corresponds to -100 percent and +1.0
// corresponds to +100 percent.
void wxImage::ChangeBrightness(double factor)
{
    // FIXME: Double equality
    if (factor == 0.0)
        return;

    wxASSERT(factor >= -1.0 && factor <= 1.0);
    ApplyToAllPixels(&DoChangeBrightness, factor);
}

// TODO-C++11: Replace with a lambda function.
static void DoChangeHSV(wxImage *image, unsigned char *rgb, wxImage::HSVValue hsvValue)
{
    // FIXME: Double equality
    if ( !(hsvValue.hue == 0.0) )
        DoRotateHue(image, rgb, hsvValue.hue);

    // FIXME: Double equality
    if ( !(hsvValue.saturation == 0.0) )
        DoChangeSaturation(image, rgb, hsvValue.saturation);

    // FIXME: Double equality
    if ( !(hsvValue.value == 0.0) )
        DoChangeBrightness(image, rgb, hsvValue.value);
}

// Changes the hue, the saturation and the brightness (value) of each pixel in
// the image. angleH is a double in the range [-1.0..+1.0], where -1.0 corresponds
// to -360 degrees and +1.0 corresponds to +360 degrees, factorS is a double in
// the range [-1.0..+1.0], where -1.0 corresponds to -100 percent and +1.0
// corresponds to +100 percent and factorV is a double in the range [-1.0..+1.0],
// where -1.0 corresponds to -100 percent and +1.0 corresponds to +100 percent.
void wxImage::ChangeHSV(double angleH, double factorS, double factorV)
{
    // FIXME: Double equality
    if ( (angleH == 0.0) && (factorS == 0.0) && (factorV == 0.0) )
        return;

    wxASSERT(angleH >= -1.0 && angleH <= 1.0 && factorS >= -1.0 &&
             factorS <= 1.0 && factorV >= -1.0 && factorV <= 1.0);
    ApplyToAllPixels(&DoChangeHSV, HSVValue(angleH, factorS, factorV));
}

//-----------------------------------------------------------------------------
// wxImageHandler
//-----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxImageHandler, wxObject);

#if wxUSE_STREAMS
int wxImageHandler::GetImageCount( wxInputStream& stream )
{
    // NOTE: this code is the same of wxAnimationDecoder::CanRead and
    //       wxImageHandler::CallDoCanRead

    if ( !stream.IsSeekable() )
        return false;        // can't test unseekable stream

    wxFileOffset posOld = stream.TellI();
    int n = DoGetImageCount(stream);

    // restore the old position to be able to test other formats and so on
    if ( stream.SeekI(posOld) == wxInvalidOffset )
    {
        wxLogDebug("Failed to rewind the stream in wxImageHandler!");

        // reading would fail anyhow as we're not at the right position
        return false;
    }

    return n;
}

bool wxImageHandler::CanRead( const std::string& name )
{
    wxImageFileInputStream stream(name);
    if ( !stream.IsOk() )
    {
        wxLogError(_("Failed to check format of image file \"%s\"."), name);

        return false;
    }

    return CanRead(stream);
}

bool wxImageHandler::CallDoCanRead(wxInputStream& stream)
{
    // NOTE: this code is the same of wxAnimationDecoder::CanRead and
    //       wxImageHandler::GetImageCount

    if ( !stream.IsSeekable() )
        return false;        // can't test unseekable stream

    wxFileOffset posOld = stream.TellI();
    bool ok = DoCanRead(stream);

    // restore the old position to be able to test other formats and so on
    if ( stream.SeekI(posOld) == wxInvalidOffset )
    {
        wxLogDebug("Failed to rewind the stream in wxImageHandler!");

        // reading would fail anyhow as we're not at the right position
        return false;
    }

    return ok;
}

#endif // wxUSE_STREAMS

/* static */
wxImageResolution
wxImageHandler::GetResolutionFromOptions(const wxImage& image, int *x, int *y)
{
    wxCHECK_MSG( x && y, wxImageResolution::None, "NULL pointer" );

    if ( image.HasOption(wxIMAGE_OPTION_RESOLUTIONX) &&
         image.HasOption(wxIMAGE_OPTION_RESOLUTIONY) )
    {
        *x = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONX);
        *y = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONY);
    }
    else if ( image.HasOption(wxIMAGE_OPTION_RESOLUTION) )
    {
        *x =
        *y = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTION);
    }
    else // no resolution options specified
    {
        *x =
        *y = 0;

        return wxImageResolution::None;
    }

    // get the resolution unit too
    int resUnit = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONUNIT);
    if ( !resUnit )
    {
        // this is the default
        // FIXME: Stupid conversion.
        resUnit = static_cast<int>(wxImageResolution::Inches);
    }

    return (wxImageResolution)resUnit;
}

// ----------------------------------------------------------------------------
// image histogram stuff
// ----------------------------------------------------------------------------

bool
wxImage::FindFirstUnusedColour(unsigned char *r,
                               unsigned char *g,
                               unsigned char *b,
                               unsigned char r2,
                               unsigned char g2,
                               unsigned char b2) const
{
    wxImageHistogram histogram;

    ComputeHistogram(histogram);

    return histogram.FindFirstUnusedColour(r, g, b, r2, g2, b2);
}



// GRG, Dic/99
// Counts and returns the number of different colours. Optionally stops
// when it exceeds 'stopafter' different colours. This is useful, for
// example, to see if the image can be saved as 8-bit (256 colour or
// less, in this case it would be invoked as CountColours(256)). Default
// value for stopafter is -1 (don't care).
//
unsigned long wxImage::CountColours( unsigned long stopafter ) const
{
    wxHashTable h;
    wxObject dummy;
    unsigned char *p = GetData();

    auto size = wx::narrow_cast<unsigned long>(GetWidth()) * GetHeight();
    unsigned long nentries = 0;

    for (unsigned long j = 0; (j < size) && (nentries <= stopafter) ; j++)
    {
        const auto r = *(p++);
        const auto g = *(p++);
        const auto b = *(p++);

        const auto key = wxImageHistogram::MakeKey(r, g, b);

        if (h.Get(key) == nullptr)
        {
            h.Put(key, &dummy);
            nentries++;
        }
    }

    return nentries;
}


unsigned long wxImage::ComputeHistogram( wxImageHistogram &h ) const
{
    unsigned char *p = GetData();
    unsigned long nentries = 0;

    h.clear();

    const unsigned long size = wx::narrow_cast<unsigned long>(GetWidth()) * GetHeight();

    for ( unsigned long n = 0; n < size; n++ )
    {
        unsigned char r = *p++;
        unsigned char g = *p++;
        unsigned char b = *p++;

        wxImageHistogramEntry& entry = h[wxImageHistogram::MakeKey(r, g, b)];

        if ( entry.value++ == 0 )
            entry.index = nentries++;
    }

    return nentries;
}

/*
 * Rotation code by Carlos Moreno
 */

// FIXME: Verify that this isn't arbitrary.
constexpr double wxROTATE_EPSILON = 1e-10;

// Auxiliary function to rotate a point (x,y) with respect to point p0
// make it inline and use a straight return to facilitate optimization
// also, the function receives the sine and cosine of the angle to avoid
// repeating the time-consuming calls to these functions -- sin/cos can
// be computed and stored in the calling function.

static inline wxRealPoint
wxRotatePoint(const wxRealPoint& p, double cos_angle, double sin_angle,
              const wxRealPoint& p0)
{
    return {p0.x + (p.x - p0.x) * cos_angle - (p.y - p0.y) * sin_angle,
            p0.y + (p.y - p0.y) * cos_angle + (p.x - p0.x) * sin_angle};
}

static inline wxRealPoint
wxRotatePoint(double x, double y, double cos_angle, double sin_angle,
              const wxRealPoint & p0)
{
    return wxRotatePoint (wxRealPoint(x,y), cos_angle, sin_angle, p0);
}

wxImage wxImage::Rotate(double angle,
                        const wxPoint& centre_of_rotation,
                        bool interpolating,
                        wxPoint *offset_after_rotation) const
{
    // screen coordinates are a mirror image of "real" coordinates
    angle = -angle;

    const bool has_alpha = HasAlpha();

    const int w = GetWidth();
    const int h = GetHeight();

    // Create pointer-based array to accelerate access to wxImage's data
    auto data = std::make_unique<unsigned char*[]>(h);

    data[0] = GetData();
    for (int i = 1; i < h; i++)
        data[i] = data[i - 1] + (3 * w);

    // Same for alpha channel
    auto alpha = std::make_unique<unsigned char*[]>(h);

    if (has_alpha)
    {
        alpha[0] = GetAlpha();
        for (int i = 1; i < h; i++)
            alpha[i] = alpha[i - 1] + w;
    }

    // precompute coefficients for rotation formula
    const double cos_angle = std::cos(angle);
    const double sin_angle = std::sin(angle);

    // Create new Image to store the result
    // First, find rectangle that covers the rotated image;  to do that,
    // rotate the four corners

    const wxRealPoint p0(centre_of_rotation.x, centre_of_rotation.y);

    const wxRealPoint p1 = wxRotatePoint (0, 0, cos_angle, sin_angle, p0);
    const wxRealPoint p2 = wxRotatePoint (0, h, cos_angle, sin_angle, p0);
    const wxRealPoint p3 = wxRotatePoint (w, 0, cos_angle, sin_angle, p0);
    const wxRealPoint p4 = wxRotatePoint (w, h, cos_angle, sin_angle, p0);

    int x1a = (int) std::floor(std::min (std::min(p1.x, p2.x), std::min(p3.x, p4.x)));
    int y1a = (int) std::floor(std::min (std::min(p1.y, p2.y), std::min(p3.y, p4.y)));
    int x2a = (int) std::ceil(std::max (std::max(p1.x, p2.x), std::max(p3.x, p4.x)));
    int y2a = (int) std::ceil(std::max (std::max(p1.y, p2.y), std::max(p3.y, p4.y)));

    // Create rotated image
    wxImage rotated (x2a - x1a + 1, y2a - y1a + 1, false);
    // With alpha channel
    if (has_alpha)
        rotated.SetAlpha();

    if (offset_after_rotation != nullptr)
    {
        *offset_after_rotation = wxPoint (x1a, y1a);
    }

    // the rotated (destination) image is always accessed sequentially via this
    // pointer, there is no need for pointer-based arrays here
    unsigned char *dst = rotated.GetData();

    unsigned char *alpha_dst = has_alpha ? rotated.GetAlpha() : nullptr;

    // if the original image has a mask, use its RGB values as the blank pixel,
    // else, fall back to default (black).
    unsigned char blank_r = 0;
    unsigned char blank_g = 0;
    unsigned char blank_b = 0;

    if (HasMask())
    {
        blank_r = GetMaskRed();
        blank_g = GetMaskGreen();
        blank_b = GetMaskBlue();
        rotated.SetMaskColour( blank_r, blank_g, blank_b );
    }

    // Now, for each point of the rotated image, find where it came from, by
    // performing an inverse rotation (a rotation of -angle) and getting the
    // pixel at those coordinates

    const int rH = rotated.GetHeight();
    const int rW = rotated.GetWidth();

    // do the (interpolating) test outside of the loops, so that it is done
    // only once, instead of repeating it for each pixel.
    if (interpolating)
    {
        for (int y = 0; y < rH; y++)
        {
            for (int x = 0; x < rW; x++)
            {
                wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

                if (-0.25 < src.x && src.x < w - 0.75 &&
                    -0.25 < src.y && src.y < h - 0.75)
                {
                    // interpolate using the 4 enclosing grid-points.  Those
                    // points can be obtained using floor and ceiling of the
                    // exact coordinates of the point
                    int x1, y1, x2, y2;

                    if (0 < src.x && src.x < w - 1)
                    {
                        x1 = std::lround(std::floor(src.x));
                        x2 = std::lround(std::ceil(src.x));
                    }
                    else    // else means that x is near one of the borders (0 or width-1)
                    {
                        x1 = x2 = std::lround(src.x);
                    }

                    if (0 < src.y && src.y < h - 1)
                    {
                        y1 = std::lround(std::floor(src.y));
                        y2 = std::lround(std::ceil(src.y));
                    }
                    else
                    {
                        y1 = y2 = std::lround(src.y);
                    }

                    // get four points and the distances (square of the distance,
                    // for efficiency reasons) for the interpolation formula

                    // GRG: Do not calculate the points until they are
                    //      really needed -- this way we can calculate
                    //      just one, instead of four, if d1, d2, d3
                    //      or d4 are < wxROTATE_EPSILON

                    const double d1 = (src.x - x1) * (src.x - x1) + (src.y - y1) * (src.y - y1);
                    const double d2 = (src.x - x2) * (src.x - x2) + (src.y - y1) * (src.y - y1);
                    const double d3 = (src.x - x2) * (src.x - x2) + (src.y - y2) * (src.y - y2);
                    const double d4 = (src.x - x1) * (src.x - x1) + (src.y - y2) * (src.y - y2);

                    // Now interpolate as a weighted average of the four surrounding
                    // points, where the weights are the distances to each of those points

                    // If the point is exactly at one point of the grid of the source
                    // image, then don't interpolate -- just assign the pixel

                    // d1,d2,d3,d4 are positive -- no need for abs()
                    if (d1 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y1] + (3 * x1);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y1] + x1);
                    }
                    else if (d2 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y1] + (3 * x2);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y1] + x2);
                    }
                    else if (d3 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y2] + (3 * x2);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y2] + x2);
                    }
                    else if (d4 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y2] + (3 * x1);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y2] + x1);
                    }
                    else
                    {
                        // weights for the weighted average are proportional to the inverse of the distance
                        unsigned char *v1 = data[y1] + (3 * x1);
                        unsigned char *v2 = data[y1] + (3 * x2);
                        unsigned char *v3 = data[y2] + (3 * x2);
                        unsigned char *v4 = data[y2] + (3 * x1);

                        const double w1 = 1/d1, w2 = 1/d2, w3 = 1/d3, w4 = 1/d4;

                        // GRG: Unrolled.

                        *(dst++) = (unsigned char)
                            ( (w1 * *(v1++) + w2 * *(v2++) +
                               w3 * *(v3++) + w4 * *(v4++)) /
                              (w1 + w2 + w3 + w4) );
                        *(dst++) = (unsigned char)
                            ( (w1 * *(v1++) + w2 * *(v2++) +
                               w3 * *(v3++) + w4 * *(v4++)) /
                              (w1 + w2 + w3 + w4) );
                        *(dst++) = (unsigned char)
                            ( (w1 * *v1 + w2 * *v2 +
                               w3 * *v3 + w4 * *v4) /
                              (w1 + w2 + w3 + w4) );

                        if (has_alpha)
                        {
                            v1 = alpha[y1] + (x1);
                            v2 = alpha[y1] + (x2);
                            v3 = alpha[y2] + (x2);
                            v4 = alpha[y2] + (x1);

                            *(alpha_dst++) = (unsigned char)
                                ( (w1 * *v1 + w2 * *v2 +
                                   w3 * *v3 + w4 * *v4) /
                                  (w1 + w2 + w3 + w4) );
                        }
                    }
                }
                else
                {
                    *(dst++) = blank_r;
                    *(dst++) = blank_g;
                    *(dst++) = blank_b;

                    if (has_alpha)
                        *(alpha_dst++) = 0;
                }
            }
        }
    }
    else // not interpolating
    {
        for (int y = 0; y < rH; y++)
        {
            for (int x = 0; x < rW; x++)
            {
                wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

                const int xs = std::lround(src.x);      // wxRound rounds to the
                const int ys = std::lround(src.y);      // closest integer

                if (0 <= xs && xs < w && 0 <= ys && ys < h)
                {
                    unsigned char *p = data[ys] + (3 * xs);
                    *(dst++) = *(p++);
                    *(dst++) = *(p++);
                    *(dst++) = *p;

                    if (has_alpha)
                        *(alpha_dst++) = *(alpha[ys] + (xs));
                }
                else
                {
                    *(dst++) = blank_r;
                    *(dst++) = blank_g;
                    *(dst++) = blank_b;

                    if (has_alpha)
                        *(alpha_dst++) = 255;
                }
            }
        }
    }

    return rotated;
}

// Helper function used internally by wxImage class only.
template <typename T>
void wxImage::ApplyToAllPixels(void (*filter)(wxImage *, unsigned char *, T), T value)
{
    AllocExclusive();

    const size_t size = GetWidth() * GetHeight();
    unsigned char *data = GetData();

    for ( size_t i = 0; i < size; i++, data += 3 )
    {
        (*filter)(this, data, value);
    }
}

// A module to allow wxImage initialization/cleanup
// without calling these functions from app.cpp or from
// the user's application.

class wxImageModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxImageModule);
public:
    bool OnInit() override { wxImage::InitStandardHandlers(); return true; }
    void OnExit() override { wxImage::CleanUpHandlers(); }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxImageModule, wxModule);


#endif // wxUSE_IMAGE
