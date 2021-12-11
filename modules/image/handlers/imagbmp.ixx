/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagbmp.h
// Purpose:     wxImage BMP, ICO, CUR and ANI handlers
// Author:      Robert Roebling, Chris Elliott
// Copyright:   (c) Robert Roebling, Chris Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/filefn.h"
#include "wx/gdicmn.h"
#include "wx/colour.h"
#include "wx/log.h"
#include "wx/palette.h"
#include "wx/intl.h"
#include "wx/wfstream.h"
#include "wx/quantize.h"
#include "wx/scopeguard.h"
#include "wx/scopedarray.h"

#include "wx/private/icondir.h"

#include <memory>


export module WX.Image.BMP;

import WX.Image.Base;
import WX.Image.PNG;

import Utils.Math;

import <array>;

export
{

// defines for saving the BMP file in different formats, Bits Per Pixel
// USE: wximage.SetOption( wxIMAGE_OPTION_BMP_FORMAT, wxBMP_xBPP );
inline constexpr char wxIMAGE_OPTION_BMP_FORMAT[] = "wxBMP_FORMAT";

enum
{
    wxBMP_24BPP        = 24, // default, do not need to set
    //wxBMP_16BPP      = 16, // wxQuantize can only do 236 colors?
    wxBMP_8BPP         =  8, // 8bpp, quantized colors
    wxBMP_8BPP_GREY    =  9, // 8bpp, rgb averaged to greys
    wxBMP_8BPP_GRAY    =  wxBMP_8BPP_GREY,
    wxBMP_8BPP_RED     = 10, // 8bpp, red used as greyscale
    wxBMP_8BPP_PALETTE = 11, // 8bpp, use the wxImage's palette
    wxBMP_4BPP         =  4, // 4bpp, quantized colors
    wxBMP_1BPP         =  1, // 1bpp, quantized "colors"
    wxBMP_1BPP_BW      =  2  // 1bpp, black & white from red
};

// ----------------------------------------------------------------------------
// wxBMPHandler
// ----------------------------------------------------------------------------

class wxBMPHandler : public wxImageHandler
{
public:
    wxBMPHandler()
    {
        m_name = "Windows bitmap file";
        m_extension = "bmp";
        m_type = wxBitmapType::BMP;
        m_mime = "image/x-bmp";
    }

#if wxUSE_STREAMS
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;

protected:
    bool DoCanRead( wxInputStream& stream ) override;
    bool SaveDib(wxImage *image, wxOutputStream& stream, bool verbose,
                 bool IsBmp, bool IsMask);
    bool DoLoadDib(wxImage *image, int width, int height, int bpp, int ncolors,
                   int comp, wxFileOffset bmpOffset, wxInputStream& stream,
                   bool verbose, bool IsBmp, bool hasPalette, int colEntrySize = 4);
    bool LoadDib(wxImage *image, wxInputStream& stream, bool verbose, bool IsBmp);
#endif // wxUSE_STREAMS
};

//-----------------------------------------------------------------------------
// wxBMPHandler
//-----------------------------------------------------------------------------

#if wxUSE_STREAMS

#ifndef BI_RGB
    #define BI_RGB       0
#endif

#ifndef BI_RLE8
#define BI_RLE8      1
#endif

#ifndef BI_RLE4
#define BI_RLE4      2
#endif

#ifndef BI_BITFIELDS
#define BI_BITFIELDS 3
#endif

#define poffset (line * width * 3 + column * 3)

bool wxBMPHandler::SaveFile(wxImage *image,
                            wxOutputStream& stream,
                            bool verbose)
{
    return SaveDib(image, stream, verbose, true/*IsBmp*/, false/*IsMask*/);
}

bool wxBMPHandler::SaveDib(wxImage *image,
                           wxOutputStream& stream,
                           bool verbose,
                           bool IsBmp,
                           bool IsMask)

{
    wxCHECK_MSG( image, false, "invalid pointer in wxBMPHandler::SaveFile" );

    if ( !image->IsOk() )
    {
        if ( verbose )
        {
            wxLogError(_("BMP: Couldn't save invalid image."));
        }
        return false;
    }

    // For icons, save alpha channel if available.
    const bool saveAlpha = !IsBmp && image->HasAlpha();

    // get the format of the BMP file to save,
    // else (and always if alpha channel is present) use 24bpp
    unsigned format = wxBMP_24BPP;
    if ( image->HasOption(wxIMAGE_OPTION_BMP_FORMAT) && !saveAlpha )
        format = image->GetOptionInt(wxIMAGE_OPTION_BMP_FORMAT);

    std::uint16_t bpp;     // # of bits per pixel
    int palette_size; // # of color map entries, ie. 2^bpp colors

    // set the bpp and appropriate palette_size, and do additional checks
    if ( (format == wxBMP_1BPP) || (format == wxBMP_1BPP_BW) )
    {
        bpp = 1;
        palette_size = 2;
    }
    else if ( format == wxBMP_4BPP )
    {
        bpp = 4;
        palette_size = 16;
    }
    else if ( (format == wxBMP_8BPP) || (format == wxBMP_8BPP_GREY) ||
              (format == wxBMP_8BPP_RED) || (format == wxBMP_8BPP_PALETTE) )
    {
        // need to set a wxPalette to use this, HOW TO CHECK IF VALID, SIZE?
        if (format == wxBMP_8BPP_PALETTE
#if wxUSE_PALETTE
                && !image->HasPalette()
#endif // wxUSE_PALETTE
            )
        {
            if ( verbose )
            {
                wxLogError(_("BMP: wxImage doesn't have own wxPalette."));
            }
            return false;
        }
        bpp = 8;
        palette_size = 256;
    }
    else  // you get 24bpp or 32bpp with alpha
    {
        format = wxBMP_24BPP;
        bpp = saveAlpha ? 32 : 24;
        palette_size = 0;
    }

    unsigned width = image->GetWidth();
    unsigned row_padding = (4 - ((width * bpp + 7) / 8) % 4) % 4; // # bytes to pad to dword
    unsigned row_width = (width * bpp + 7) / 8 + row_padding; // # of bytes per row

    struct
    {
        // BitmapHeader:
        std::uint32_t  filesize;       // total file size, inc. headers
        std::uint32_t  reserved;       // for future use
        std::uint32_t  data_offset;    // image data offset in the file
        std::uint16_t  magic;          // format magic, always 'BM'

        // BitmapInfoHeader:
        std::uint32_t  bih_size;       // 2nd part's size
        std::uint32_t  width, height;  // bitmap's dimensions
        std::uint32_t  compression;    // compression method
        std::uint32_t  size_of_bmp;    // size of the bitmap
        std::uint32_t  h_res, v_res;   // image resolution in pixels-per-meter
        std::uint32_t  num_clrs;       // number of colors used
        std::uint32_t  num_signif_clrs;// number of significant colors
        std::uint16_t  planes;         // num of planes
        std::uint16_t  bpp;            // bits per pixel
    } hdr;

    std::uint32_t hdr_size = 14/*BitmapHeader*/ + 40/*BitmapInfoHeader*/;

    hdr.magic = wxUINT16_SWAP_ON_BE(0x4D42/*'BM'*/);
    hdr.filesize = wxUINT32_SWAP_ON_BE( hdr_size + palette_size*4 +
                                        row_width * image->GetHeight() );
    hdr.reserved = 0;
    hdr.data_offset = wxUINT32_SWAP_ON_BE(hdr_size + palette_size*4);

    hdr.bih_size = wxUINT32_SWAP_ON_BE(hdr_size - 14);
    hdr.width = wxUINT32_SWAP_ON_BE(image->GetWidth());
    if ( IsBmp )
    {
        hdr.height = wxUINT32_SWAP_ON_BE(image->GetHeight());
    }
    else
    {
        hdr.height = wxUINT32_SWAP_ON_BE(2 * image->GetHeight());
    }
    hdr.planes = wxUINT16_SWAP_ON_BE(1); // always 1 plane
    hdr.bpp = wxUINT16_SWAP_ON_BE(bpp);
    hdr.compression = 0; // RGB uncompressed
    hdr.size_of_bmp = wxUINT32_SWAP_ON_BE(row_width * image->GetHeight());

    // get the resolution from the image options  or fall back to 72dpi standard
    // for the BMP format if not specified
    int hres, vres;
    switch ( GetResolutionFromOptions(*image, &hres, &vres) )
    {
        default:
            wxFAIL_MSG( "unexpected image resolution units" );
            [[fallthrough]];

        case wxImageResolution::None:
            hres =
            vres = 72;
            [[fallthrough]];// fall through to convert it to correct units

        case wxImageResolution::Inches:
            // convert resolution in inches to resolution in centimeters
            hres = (int)(10*mm2inches*hres);
            vres = (int)(10*mm2inches*vres);
            [[fallthrough]];// fall through to convert it to resolution in meters

        case wxImageResolution::Centimeters:
            // convert resolution in centimeters to resolution in meters
            hres *= 100;
            vres *= 100;
            break;
    }

    hdr.h_res = wxUINT32_SWAP_ON_BE(hres);
    hdr.v_res = wxUINT32_SWAP_ON_BE(vres);
    hdr.num_clrs = wxUINT32_SWAP_ON_BE(palette_size); // # colors in colormap
    hdr.num_signif_clrs = 0;     // all colors are significant

    if ( IsBmp )
    {
        if (// VS: looks ugly but compilers tend to do ugly things with structs,
            //     like aligning hdr.filesize's ofset to dword :(
            // VZ: we should add padding then...
            !stream.WriteAll(&hdr.magic, 2) ||
            !stream.WriteAll(&hdr.filesize, 4) ||
            !stream.WriteAll(&hdr.reserved, 4) ||
            !stream.WriteAll(&hdr.data_offset, 4)
           )
        {
            if (verbose)
            {
                wxLogError(_("BMP: Couldn't write the file (Bitmap) header."));
                }
            return false;
        }
    }
    if ( !IsMask )
    {
        if (
            !stream.WriteAll(&hdr.bih_size, 4) ||
            !stream.WriteAll(&hdr.width, 4) ||
            !stream.WriteAll(&hdr.height, 4) ||
            !stream.WriteAll(&hdr.planes, 2) ||
            !stream.WriteAll(&hdr.bpp, 2) ||
            !stream.WriteAll(&hdr.compression, 4) ||
            !stream.WriteAll(&hdr.size_of_bmp, 4) ||
            !stream.WriteAll(&hdr.h_res, 4) ||
            !stream.WriteAll(&hdr.v_res, 4) ||
            !stream.WriteAll(&hdr.num_clrs, 4) ||
            !stream.WriteAll(&hdr.num_signif_clrs, 4)
           )
        {
            if (verbose)
            {
                wxLogError(_("BMP: Couldn't write the file (BitmapInfo) header."));
            }
            return false;
        }
    }

#if wxUSE_PALETTE
    std::unique_ptr<wxPalette> palette; // entries for quantized images
#endif // wxUSE_PALETTE
    std::unique_ptr<std::uint8_t[]> rgbquad; // for the RGBQUAD bytes for the colormap
    std::unique_ptr<wxImage> q_image;   // destination for quantized image

    // if <24bpp use quantization to reduce colors for *some* of the formats
    if ( (format == wxBMP_1BPP) || (format == wxBMP_4BPP) ||
         (format == wxBMP_8BPP) || (format == wxBMP_8BPP_PALETTE) )
    {
        // make a new palette and quantize the image
        if (format != wxBMP_8BPP_PALETTE)
        {
            q_image = std::make_unique<wxImage>();

            // I get a delete error using Quantize when desired colors > 236
            int quantize = ((palette_size > 236) ? 236 : palette_size);
            // fill the destination too, it gives much nicer 4bpp images
#if wxUSE_PALETTE
            wxPalette* paletteTmp;
            wxQuantize::Quantize( *image, *q_image, &paletteTmp, quantize, nullptr,
                                  wxQUANTIZE_FILL_DESTINATION_IMAGE );
            palette.reset(paletteTmp);
#else // !wxUSE_PALETTE
            wxQuantize::Quantize( *image, *q_image, NULL, quantize, 0,
                                  wxQUANTIZE_FILL_DESTINATION_IMAGE );
#endif // wxUSE_PALETTE/!wxUSE_PALETTE
        }
        else
        {
#if wxUSE_PALETTE
            palette.reset(new wxPalette(image->GetPalette()));
#endif // wxUSE_PALETTE
        }

        unsigned char r, g, b;
        auto rgbquadTmp = std::make_unique<std::uint8_t[]>(palette_size * 4);
        rgbquad.swap(rgbquadTmp);

        for (int i = 0; i < palette_size; i++)
        {
#if wxUSE_PALETTE
            if ( !palette->GetRGB(i, &r, &g, &b) )
#endif // wxUSE_PALETTE
                r = g = b = 0;

            rgbquad[i*4] = b;
            rgbquad[i*4+1] = g;
            rgbquad[i*4+2] = r;
            rgbquad[i*4+3] = 0;
        }
    }
    // make a 256 entry greyscale colormap or 2 entry black & white
    else if ( (format == wxBMP_8BPP_GREY) || (format == wxBMP_8BPP_RED) ||
              (format == wxBMP_1BPP_BW) )
    {
        auto rgbquadTmp = std::make_unique<std::uint8_t[]>(palette_size*4);
        rgbquad.swap(rgbquadTmp);

        for ( int i = 0; i < palette_size; i++ )
        {
            // if 1BPP_BW then the value should be either 0 or 255
            std::uint8_t c = (std::uint8_t)((i > 0) && (format == wxBMP_1BPP_BW) ? 255 : i);

            rgbquad[i*4] =
            rgbquad[i*4+1] =
            rgbquad[i*4+2] = c;
            rgbquad[i*4+3] = 0;
        }
    }

    // if the colormap was made, then it needs to be written
    if (rgbquad)
    {
        if ( !IsMask )
        {
            if ( !stream.WriteAll(rgbquad.get(), palette_size*4) )
            {
                if (verbose)
                {
                    wxLogError(_("BMP: Couldn't write RGB color map."));
                }
                return false;
            }
        }
    }

    // pointer to the image data, use quantized if available
    const unsigned char* const data = q_image && q_image->IsOk()
                                        ? q_image->GetData()
                                        : image->GetData();
    const unsigned char* const alpha = saveAlpha ? image->GetAlpha() : nullptr;

    auto buffer = std::make_unique<std::uint8_t[]>(row_width);
    memset(buffer.get(), 0, row_width);

    long int pixel;
    const int dstPixLen = saveAlpha ? 4 : 3;

    for (int y = image->GetHeight() -1; y >= 0; y--)
    {
        if ( format == wxBMP_24BPP ) // 3 bytes per pixel red,green,blue
        {
            for (unsigned int x = 0; x < width; x++ )
            {
                pixel = 3*(y*width + x);

                buffer[dstPixLen*x    ] = data[pixel+2];
                buffer[dstPixLen*x + 1] = data[pixel+1];
                buffer[dstPixLen*x + 2] = data[pixel];
                if ( saveAlpha )
                    buffer[dstPixLen*x + 3] = alpha[y*width + x];
            }
        }
        else if ((format == wxBMP_8BPP) ||       // 1 byte per pixel in color
                 (format == wxBMP_8BPP_PALETTE))
        {
            for (unsigned int x = 0; x < width; x++)
            {
                pixel = 3*(y*width + x);
#if wxUSE_PALETTE
                buffer[x] = (std::uint8_t)palette->GetPixel( data[pixel],
                                                        data[pixel+1],
                                                        data[pixel+2] );
#else
                // FIXME: what should this be? use some std palette maybe?
                buffer[x] = 0;
#endif // wxUSE_PALETTE
            }
        }
        else if ( format == wxBMP_8BPP_GREY ) // 1 byte per pix, rgb ave to grey
        {
            for (unsigned int x = 0; x < width; x++)
            {
                pixel = 3*(y*width + x);
                buffer[x] = (std::uint8_t)(.299*data[pixel] +
                                      .587*data[pixel+1] +
                                      .114*data[pixel+2]);
            }
        }
        else if ( format == wxBMP_8BPP_RED ) // 1 byte per pixel, red as greys
        {
            for (unsigned int x = 0; x < width; x++)
            {
                buffer[x] = (std::uint8_t)data[3*(y*width + x)];
            }
        }
        else if ( format == wxBMP_4BPP ) // 4 bpp in color
        {
            for (unsigned int x = 0; x < width; x+=2)
            {
                pixel = 3*(y*width + x);

                // fill buffer, ignore if > width
#if wxUSE_PALETTE
                buffer[x/2] = (std::uint8_t)(
                    ((std::uint8_t)palette->GetPixel(data[pixel],
                                                data[pixel+1],
                                                data[pixel+2]) << 4) |
                    (((x+1) >= width)
                     ? 0
                     : ((std::uint8_t)palette->GetPixel(data[pixel+3],
                                                   data[pixel+4],
                                                   data[pixel+5]) ))    );
#else
                // FIXME: what should this be? use some std palette maybe?
                buffer[x/2] = 0;
#endif // wxUSE_PALETTE
            }
        }
        else if ( format == wxBMP_1BPP ) // 1 bpp in "color"
        {
            for (unsigned int x = 0; x < width; x+=8)
            {
                pixel = 3*(y*width + x);

#if wxUSE_PALETTE
                buffer[x/8] = (std::uint8_t)(
                                            ((std::uint8_t)palette->GetPixel(data[pixel],    data[pixel+1],  data[pixel+2]) << 7) |
                    (((x+1) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+3],  data[pixel+4],  data[pixel+5]) << 6)) |
                    (((x+2) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+6],  data[pixel+7],  data[pixel+8]) << 5)) |
                    (((x+3) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+9],  data[pixel+10], data[pixel+11]) << 4)) |
                    (((x+4) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+12], data[pixel+13], data[pixel+14]) << 3)) |
                    (((x+5) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+15], data[pixel+16], data[pixel+17]) << 2)) |
                    (((x+6) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+18], data[pixel+19], data[pixel+20]) << 1)) |
                    (((x+7) >= width) ? 0 : ((std::uint8_t)palette->GetPixel(data[pixel+21], data[pixel+22], data[pixel+23])     ))    );
#else
                // FIXME: what should this be? use some std palette maybe?
                buffer[x/8] = 0;
#endif // wxUSE_PALETTE
            }
        }
        else if ( format == wxBMP_1BPP_BW ) // 1 bpp B&W colormap from red color ONLY
        {
            for (unsigned int x = 0; x < width; x+=8)
            {
                pixel = 3*(y*width + x);

                buffer[x/8] = (std::uint8_t)(
                                           (((std::uint8_t)(data[pixel]   /128.)) << 7) |
                   (((x+1) >= width) ? 0 : (((std::uint8_t)(data[pixel+3] /128.)) << 6)) |
                   (((x+2) >= width) ? 0 : (((std::uint8_t)(data[pixel+6] /128.)) << 5)) |
                   (((x+3) >= width) ? 0 : (((std::uint8_t)(data[pixel+9] /128.)) << 4)) |
                   (((x+4) >= width) ? 0 : (((std::uint8_t)(data[pixel+12]/128.)) << 3)) |
                   (((x+5) >= width) ? 0 : (((std::uint8_t)(data[pixel+15]/128.)) << 2)) |
                   (((x+6) >= width) ? 0 : (((std::uint8_t)(data[pixel+18]/128.)) << 1)) |
                   (((x+7) >= width) ? 0 : (((std::uint8_t)(data[pixel+21]/128.))     ))    );
            }
        }

        if ( !stream.WriteAll(buffer.get(), row_width) )
        {
            if (verbose)
            {
                wxLogError(_("BMP: Couldn't write data."));
            }
            return false;
        }
    }

    return true;
}


struct BMPPalette
{
    static void Free(BMPPalette* pal) { delete [] pal; }

    unsigned char r, g, b;
};

bool wxBMPHandler::DoLoadDib(wxImage * image, int width, int height,
                             int bpp, int ncolors, int comp,
                             wxFileOffset bmpOffset, wxInputStream& stream,
                             bool verbose, bool IsBmp, bool hasPalette,
                             int colEntrySize)
{
    std::int32_t         aDword, rmask = 0, gmask = 0, bmask = 0, amask = 0;
    int             rshift = 0, gshift = 0, bshift = 0, ashift = 0;
    int             rbits = 0, gbits = 0, bbits = 0;
    std::int32_t         dbuf[4];
    std::int8_t          bbuf[4];
    std::uint8_t         aByte;
    std::uint16_t        aWord;

    // allocate space for palette if needed:
    BMPPalette *cmap;

    if ( bpp < 16 )
    {
        cmap = new BMPPalette[ncolors];
        if ( !cmap )
        {
            if (verbose)
            {
                wxLogError(_("BMP: Couldn't allocate memory."));
            }
            return false;
        }
    }
    else // no palette
    {
        cmap = nullptr;
    }

    wxON_BLOCK_EXIT1(&BMPPalette::Free, cmap);

    bool isUpsideDown = true;

    if (height < 0)
    {
        isUpsideDown = false;
        height = -height;
    }

    // destroy existing here instead of:
    image->Destroy();
    image->Create(width, height);

    unsigned char *ptr = image->GetData();

    if ( !ptr )
    {
        if ( verbose )
        {
            wxLogError( _("BMP: Couldn't allocate memory.") );
        }
        return false;
    }

    unsigned char *alpha;
    if ( bpp == 32 )
    {
        // tell the image to allocate an alpha buffer
        image->SetAlpha();
        alpha = image->GetAlpha();
        if ( !alpha )
        {
            if ( verbose )
            {
                wxLogError(_("BMP: Couldn't allocate memory."));
            }
            return false;
        }
    }
    else // no alpha
    {
        alpha = nullptr;
    }

    // Reading the palette, if it exists:
    if ( bpp < 16 && ncolors != 0 )
    {
#if wxUSE_PALETTE
        auto r = std::make_unique<unsigned char[]>(ncolors);
        auto g = std::make_unique<unsigned char[]>(ncolors);
        auto b = std::make_unique<unsigned char[]>(ncolors);
#endif // wxUSE_PALETTE
        for (int j = 0; j < ncolors; j++)
        {
            if (hasPalette)
            {
                if ( !stream.ReadAll(bbuf, colEntrySize) )
                    return false;

                cmap[j].b = bbuf[0];
                cmap[j].g = bbuf[1];
                cmap[j].r = bbuf[2];
            }
            else
            {
                //used in reading .ico file mask
                cmap[j].r =
                cmap[j].g =
                cmap[j].b = ( j ? 255 : 0 );
            }
#if wxUSE_PALETTE
            r[j] = cmap[j].r;
            g[j] = cmap[j].g;
            b[j] = cmap[j].b;
#endif // wxUSE_PALETTE
        }

#if wxUSE_PALETTE
        // Set the palette for the wxImage
        image->SetPalette(wxPalette(ncolors, r.get(), g.get(), b.get()));
#endif // wxUSE_PALETTE
    }
    else if ( bpp == 16 || bpp == 32 )
    {
        if ( comp == BI_BITFIELDS )
        {
            int bit;
            if ( !stream.ReadAll(dbuf, 4 * 3) )
                return false;

            rmask = wxINT32_SWAP_ON_BE(dbuf[0]);
            gmask = wxINT32_SWAP_ON_BE(dbuf[1]);
            bmask = wxINT32_SWAP_ON_BE(dbuf[2]);
            // find shift amount (Least significant bit of mask)
            for (bit = bpp-1; bit>=0; bit--)
            {
                if (bmask & (1 << bit))
                    bshift = bit;
                if (gmask & (1 << bit))
                    gshift = bit;
                if (rmask & (1 << bit))
                    rshift = bit;
            }
            // Find number of bits in mask (MSB-LSB+1)
            for (bit = 0; bit < bpp; bit++)
            {
                if (bmask & (1 << bit))
                    bbits = bit-bshift+1;
                if (gmask & (1 << bit))
                    gbits = bit-gshift+1;
                if (rmask & (1 << bit))
                    rbits = bit-rshift+1;
            }
        }
        else if ( bpp == 16 )
        {
            rmask = 0x7C00;
            gmask = 0x03E0;
            bmask = 0x001F;
            rshift = 10;
            gshift = 5;
            bshift = 0;
            rbits = 5;
            gbits = 5;
            bbits = 5;
        }
        else if ( bpp == 32 )
        {
            rmask = 0x00FF0000;
            gmask = 0x0000FF00;
            bmask = 0x000000FF;
            amask = 0xFF000000;

            ashift = 24;
            rshift = 16;
            gshift = 8;
            bshift = 0;
            rbits = 8;
            gbits = 8;
            bbits = 8;
        }
    }

    /*
     * Reading the image data
     */
    if ( IsBmp )
    {
        // NOTE: seeking a positive amount in wxSeekMode::FromCurrent mode allows us to
        //       load even non-seekable streams (see wxInputStream::SeekI docs)!
        const wxFileOffset pos = stream.TellI();
        if ( pos == wxInvalidOffset ||
             (bmpOffset > pos &&
              stream.SeekI(bmpOffset - pos, wxSeekMode::FromCurrent) == wxInvalidOffset) )
            return false;
        //else: icon, just carry on
    }

    unsigned char *data = ptr;

    /* set the whole image to the background color */
    if ( bpp < 16 && (comp == BI_RLE4 || comp == BI_RLE8) )
    {
        for (int i = 0; i < width * height; i++)
        {
            *ptr++ = cmap[0].r;
            *ptr++ = cmap[0].g;
            *ptr++ = cmap[0].b;
        }
        ptr = data;
    }

    int linesize = ((width * bpp + 31) / 32) * 4;

    // flag indicating if we have any not fully transparent alpha values: this
    // is used to account for the bitmaps which use 32bpp format (normally
    // meaning that they have alpha channel) but have only zeroes in it so that
    // without this hack they appear fully transparent -- and as this is
    // unlikely intentional, we consider that they don't have alpha at all in
    // this case (see #10915)
    bool hasValidAlpha = false;

    for ( int row = 0; row < height; row++ )
    {
        int line = isUpsideDown ? height - 1 - row : row;

        int linepos = 0;
        for ( int column = 0; column < width ; )
        {
            if ( bpp < 16 )
            {
                linepos++;
                aByte = stream.GetC();
                if ( !stream.IsOk() )
                    return false;

                if ( bpp == 1 )
                {
                    for (int bit = 0; bit < 8 && column < width; bit++)
                    {
                        int index = ((aByte & (0x80 >> bit)) ? 1 : 0);
                        ptr[poffset] = cmap[index].r;
                        ptr[poffset + 1] = cmap[index].g;
                        ptr[poffset + 2] = cmap[index].b;
                        column++;
                    }
                }
                else if ( bpp == 4 )
                {
                    if ( comp == BI_RLE4 )
                    {
                        std::uint8_t first;
                        first = aByte;
                        aByte = stream.GetC();
                        if ( !stream.IsOk() )
                            return false;

                        if ( first == 0 )
                        {
                            if ( aByte == 0 )
                            {
                                // end of scanline marker
                                column = width;
                                row--;
                            }
                            else if ( aByte == 1 )
                            {
                                // end of RLE data marker, stop decoding
                                column = width;
                                row = height;
                            }
                            else if ( aByte == 2 )
                            {
                                // delta marker, move in image
                                aByte = stream.GetC();
                                if ( !stream.IsOk() )
                                    return false;
                                column += aByte;
                                linepos = column * bpp / 4;
                                aByte = stream.GetC();
                                if ( !stream.IsOk() )
                                    return false;
                                row += aByte; // upside down
                            }
                            else
                            {
                                int absolute = aByte;
                                std::uint8_t nibble[2] ;
                                int readBytes = 0 ;
                                for (int k = 0; k < absolute; k++)
                                {
                                    if ( !(k % 2 ) )
                                    {
                                        ++readBytes ;
                                        aByte = stream.GetC();
                                        if ( !stream.IsOk() )
                                            return false;
                                        nibble[0] = (std::uint8_t)( (aByte & 0xF0) >> 4 ) ;
                                        nibble[1] = (std::uint8_t)( aByte & 0x0F ) ;
                                    }
                                    ptr[poffset    ] = cmap[nibble[k%2]].r;
                                    ptr[poffset + 1] = cmap[nibble[k%2]].g;
                                    ptr[poffset + 2] = cmap[nibble[k%2]].b;
                                    column++;
                                    if ( k % 2 )
                                        linepos++;
                                }
                                if ( readBytes & 0x01 )
                                {
                                    aByte = stream.GetC();
                                    if ( !stream.IsOk() )
                                        return false;
                                }
                            }
                        }
                        else
                        {
                            std::uint8_t nibble[2] ;
                            nibble[0] = (std::uint8_t)( (aByte & 0xF0) >> 4 ) ;
                            nibble[1] = (std::uint8_t)( aByte & 0x0F ) ;

                            for ( int l = 0; l < first && column < width; l++ )
                            {
                                ptr[poffset    ] = cmap[nibble[l%2]].r;
                                ptr[poffset + 1] = cmap[nibble[l%2]].g;
                                ptr[poffset + 2] = cmap[nibble[l%2]].b;
                                column++;
                                if ( l % 2 )
                                    linepos++;
                            }
                        }
                    }
                    else
                    {
                        for (int nibble = 0; nibble < 2 && column < width; nibble++)
                        {
                            int index = ((aByte & (0xF0 >> (nibble * 4))) >> (!nibble * 4));
                            if ( index >= 16 )
                                index = 15;
                            ptr[poffset] = cmap[index].r;
                            ptr[poffset + 1] = cmap[index].g;
                            ptr[poffset + 2] = cmap[index].b;
                            column++;
                        }
                    }
                }
                else if ( bpp == 8 )
                {
                    if ( comp == BI_RLE8 )
                    {
                        unsigned char first;
                        first = aByte;
                        aByte = stream.GetC();
                        if ( !stream.IsOk() )
                            return false;

                        if ( first == 0 )
                        {
                            if ( aByte == 0 )
                            {
                                // end of scanline marker
                                column = width;
                                row--;
                            }
                            else if ( aByte == 1 )
                            {
                                // end of RLE data marker, stop decoding
                                column = width;
                                row = height;
                            }
                            else if ( aByte == 2 )
                            {
                                // delta marker, move in image
                                aByte = stream.GetC();
                                if ( !stream.IsOk() )
                                    return false;
                                column += aByte;
                                linepos = column * bpp / 8;
                                aByte = stream.GetC();
                                if ( !stream.IsOk() )
                                    return false;
                                row -= aByte;
                            }
                            else
                            {
                                int absolute = aByte;
                                for (int k = 0; k < absolute; k++)
                                {
                                    linepos++;
                                    aByte = stream.GetC();
                                    if ( !stream.IsOk() )
                                        return false;
                                    ptr[poffset    ] = cmap[aByte].r;
                                    ptr[poffset + 1] = cmap[aByte].g;
                                    ptr[poffset + 2] = cmap[aByte].b;
                                    column++;
                                }
                                if ( absolute & 0x01 )
                                {
                                    aByte = stream.GetC();
                                    if ( !stream.IsOk() )
                                        return false;
                                }
                            }
                        }
                        else
                        {
                            for ( int l = 0; l < first && column < width; l++ )
                            {
                                ptr[poffset    ] = cmap[aByte].r;
                                ptr[poffset + 1] = cmap[aByte].g;
                                ptr[poffset + 2] = cmap[aByte].b;
                                column++;
                                linepos++;
                            }
                        }
                    }
                    else
                    {
                        ptr[poffset    ] = cmap[aByte].r;
                        ptr[poffset + 1] = cmap[aByte].g;
                        ptr[poffset + 2] = cmap[aByte].b;
                        column++;
                        // linepos += size;    seems to be wrong, RR
                    }
                }
            }
            else if ( bpp == 24 )
            {
                if ( !stream.ReadAll(bbuf, 3) )
                    return false;
                linepos += 3;
                ptr[poffset    ] = (unsigned char)bbuf[2];
                ptr[poffset + 1] = (unsigned char)bbuf[1];
                ptr[poffset + 2] = (unsigned char)bbuf[0];
                column++;
            }
            else if ( bpp == 16 )
            {
                unsigned char temp;
                if ( !stream.ReadAll(&aWord, 2) )
                    return false;
                wxUINT16_SWAP_ON_BE_IN_PLACE(aWord);
                linepos += 2;
                /* Use the masks and calculated amount of shift
                   to retrieve the color data out of the word.  Then
                   shift it left by (8 - number of bits) such that
                   the image has the proper dynamic range */
                temp = (unsigned char)(((aWord & rmask) >> rshift) << (8-rbits));
                ptr[poffset] = temp;
                temp = (unsigned char)(((aWord & gmask) >> gshift) << (8-gbits));
                ptr[poffset + 1] = temp;
                temp = (unsigned char)(((aWord & bmask) >> bshift) << (8-bbits));
                ptr[poffset + 2] = temp;
                column++;
            }
            else
            {
                unsigned char temp;
                if ( !stream.ReadAll(&aDword, 4) )
                    return false;

                wxINT32_SWAP_ON_BE_IN_PLACE(aDword);
                linepos += 4;
                temp = (unsigned char)((aDword & rmask) >> rshift);
                ptr[poffset] = temp;
                temp = (unsigned char)((aDword & gmask) >> gshift);
                ptr[poffset + 1] = temp;
                temp = (unsigned char)((aDword & bmask) >> bshift);
                ptr[poffset + 2] = temp;
                if ( alpha )
                {
                    temp = (unsigned char)((aDword & amask) >> ashift);
                    alpha[line * width + column] = temp;

                    if ( temp != wxALPHA_TRANSPARENT )
                        hasValidAlpha = true;
                }
                column++;
            }
        }
        while ( (linepos < linesize) && (comp != 1) && (comp != 2) )
        {
            ++linepos;
            if ( !stream.ReadAll(&aByte, 1) )
                break;
        }
    }

    image->SetMask(false);

    // check if we had any valid alpha values in this bitmap
    if ( alpha && !hasValidAlpha )
    {
        // we didn't, so finally discard the alpha channel completely
        image->ClearAlpha();
    }

    const wxStreamError err = stream.GetLastError();
    return err == wxSTREAM_NO_ERROR || err == wxSTREAM_EOF;
}

bool wxBMPHandler::LoadDib(wxImage *image, wxInputStream& stream,
                           bool verbose, bool IsBmp)
{
    std::uint16_t        aWord;
    std::int32_t         dbuf[4];

    // offset to bitmap data
    wxFileOffset offset;
    // DIB header size (used to distinguish different versions of DIB header)
    std::int32_t hdrSize;
    if ( IsBmp )
    {
        std::int8_t bbuf[4];
        // read the header off the .BMP format file
        if ( !stream.ReadAll(bbuf, 2) ||
             !stream.ReadAll(dbuf, 16) )
            return false;

        #if 0 // unused
            std::int32_t size = wxINT32_SWAP_ON_BE(dbuf[0]);
        #endif
        offset = wxINT32_SWAP_ON_BE(dbuf[2]);
        hdrSize = wxINT32_SWAP_ON_BE(dbuf[3]);
    }
    else
    {
        if ( !stream.ReadAll(dbuf, 4) )
            return false;

        offset = wxInvalidOffset; // not used in loading ICO/CUR DIBs
        hdrSize = wxINT32_SWAP_ON_BE(dbuf[0]);
    }

    // Bitmap files come in old v1 format using BITMAPCOREHEADER or a newer
    // format (typically BITMAPV5HEADER, but we don't
    // really support any features specific to later formats such as gamma
    // correction or ICC profiles, so it doesn't matter much to us).
    const bool usesV1 = hdrSize == 12;

    int width;
    int height;
    if ( usesV1 )
    {
        std::int16_t buf[2];
        if ( !stream.ReadAll(buf, sizeof(buf)) )
            return false;

        width = wxINT16_SWAP_ON_BE((short)buf[0]);
        height = wxINT16_SWAP_ON_BE((short)buf[1]);
    }
    else // We have at least BITMAPINFOHEADER
    {
        if ( !stream.ReadAll(dbuf, 4 * 2) )
            return false;

        width = wxINT32_SWAP_ON_BE((int)dbuf[0]);
        height = wxINT32_SWAP_ON_BE((int)dbuf[1]);
    }
    if ( !IsBmp) height /= 2; // for icons divide by 2

    if ( width > 32767 )
    {
        if (verbose)
        {
            wxLogError( _("DIB Header: Image width > 32767 pixels for file.") );
        }
        return false;
    }
    if ( height > 32767 )
    {
        if (verbose)
        {
            wxLogError( _("DIB Header: Image height > 32767 pixels for file.") );
        }
        return false;
    }

    if ( !stream.ReadAll(&aWord, 2) )
        return false;

    /*
            TODO
            int planes = (int)wxUINT16_SWAP_ON_BE( aWord );
        */
    if ( !stream.ReadAll(&aWord, 2) )
        return false;

    const int bpp = wxUINT16_SWAP_ON_BE((int)aWord);
    if ( bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32 )
    {
        if (verbose)
        {
            wxLogError( _("DIB Header: Unknown bitdepth in file.") );
        }
        return false;
    }

    class Resolution
    {
    public:
        void Init(int x, int y)
        {
            m_x = x;
            m_y = y;
            m_valid = true;
        }

        bool IsValid() const { return m_valid; }

        int GetX() const { return m_x; }
        int GetY() const { return m_y; }

    private:
        int m_x{0};
        int m_y{0};
        bool m_valid{false};
    } res;

    int comp;
    int ncolors;

    if ( usesV1 )
    {
        // The only possible format is BI_RGB and colours count is not used.
        comp = BI_RGB;
        ncolors = 0;
    }
    else // We have at least BITMAPINFOHEADER
    {
        if ( !stream.ReadAll(dbuf, 4 * 4) )
            return false;

        comp = wxINT32_SWAP_ON_BE((int)dbuf[0]);
        if ( comp != BI_RGB && comp != BI_RLE4 && comp != BI_RLE8 &&
             comp != BI_BITFIELDS )
        {
            if (verbose)
            {
                wxLogError( _("DIB Header: Unknown encoding in file.") );
            }
            return false;
        }

        if ( !stream.ReadAll(dbuf, 4 * 2) )
            return false;

        ncolors = wxINT32_SWAP_ON_BE( (int)dbuf[0] );
        res.Init(dbuf[2]/100, dbuf[3]/100);

        // We've read BITMAPINFOHEADER data but for BITMAPV4HEADER or BITMAPV5HEADER
        // we have to forward stream position to after the actual bitmap header.
        //
        // Note: hardcode its size as struct BITMAPINFOHEADER is not defined on
        // non-MSW platforms.
        static constexpr std::int32_t sizeBITMAPINFOHEADER = 40;
        if ( hdrSize > sizeBITMAPINFOHEADER )
        {
            if ( stream.SeekI(hdrSize - sizeBITMAPINFOHEADER, wxSeekMode::FromCurrent) == wxInvalidOffset )
                return false;
        }
    }
    if (ncolors == 0)
        ncolors = 1 << bpp;
    /* some more sanity checks */
    if (((comp == BI_RLE4) && (bpp != 4)) ||
        ((comp == BI_RLE8) && (bpp != 8)) ||
        ((comp == BI_BITFIELDS) && (bpp != 16 && bpp != 32)))
    {
        if (verbose)
        {
            wxLogError( _("DIB Header: Encoding doesn't match bitdepth.") );
        }
        return false;
    }

    //read DIB; this is the BMP image or the XOR part of an icon image
    if ( !DoLoadDib(image, width, height, bpp, ncolors, comp, offset, stream,
                    verbose, IsBmp, true,
                    usesV1 ? 3 : 4) )
    {
        if (verbose)
        {
            wxLogError( _("Error in reading image DIB.") );
        }
        return false;
    }

    if ( !IsBmp )
    {
        //read Icon mask which is monochrome
        //there is no palette, so we will create one
        wxImage mask;
        if ( !DoLoadDib(&mask, width, height, 1, 2, BI_RGB, offset, stream,
                        verbose, IsBmp, false) )
        {
            if (verbose)
            {
                wxLogError( _("ICO: Error in reading mask DIB.") );
            }
            return false;
        }
        image->SetMaskFromImage(mask, 255, 255, 255);

    }

    // the resolution in the bitmap header is in meters, convert to centimeters
    if ( res.IsValid() )
    {
        // FIXME: Stupid solution.
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONUNIT, static_cast<int>(wxImageResolution::Centimeters));
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONX, res.GetX());
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONY, res.GetY());
    }

    return true;
}

bool wxBMPHandler::LoadFile(wxImage *image, wxInputStream& stream,
                            bool verbose, [[maybe_unused]] int index)
{
    // Read a single DIB fom the file:
    return LoadDib(image, stream, verbose, true/*isBmp*/);
}

bool wxBMPHandler::DoCanRead(wxInputStream& stream)
{
    std::array<unsigned char, 2> hdr;

    // TODO: Use span
    if ( !stream.ReadAll(hdr.data(), hdr.size()) )     // it's ok to modify the stream position here
        return false;

    // do we have the BMP file signature?
    return hdr[0] == 'B' && hdr[1] == 'M';
}

#endif // wxUSE_STREAMS

} // export
