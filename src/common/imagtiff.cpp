// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/imagtiff.cpp
// Purpose:     wxImage TIFF handler
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/defs.h"
#include "wx/log.h"
#include "wx/translation.h"
#include "wx/wxcrtvararg.h"

#include <tiff.h>
#include <tiffio.h>

module WX.Image.TIFF;

import Utils.Strings;

import <charconv>;

#ifdef wxUSE_LIBTIFF

// ============================================================================
// implementation
// ============================================================================

//-----------------------------------------------------------------------------
// wxTIFFHandler
//-----------------------------------------------------------------------------

wxTIFFHandler::wxTIFFHandler()
{
    m_name = "TIFF file";
    m_extension = "tif";
    m_altExtensions.push_back("tiff");
    m_type = wxBitmapType::TIFF;
    m_mime = "image/tiff";
    TIFFSetWarningHandler((TIFFErrorHandler) TIFFwxWarningHandler);
    TIFFSetErrorHandler((TIFFErrorHandler) TIFFwxErrorHandler);
}

#if wxUSE_STREAMS

bool wxTIFFHandler::LoadFile( wxImage *image, wxInputStream& stream, bool verbose, int index )
{
    if (index == -1)
        index = 0;

    image->Destroy();

    TIFF *tif = TIFFwxOpen( stream, "image", "r" );

    if (!tif)
    {
        if (verbose)
        {
            wxLogError( _("TIFF: Error loading image.") );
        }

        return false;
    }

    if (!TIFFSetDirectory( tif, (tdir_t)index ))
    {
        if (verbose)
        {
            wxLogError( _("Invalid TIFF image index.") );
        }

        TIFFClose( tif );

        return false;
    }

    std::uint32_t w, h;
    std::uint32_t *raster;

    TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &w );
    TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &h );

    std::uint16_t samplesPerPixel = 0;
    std::ignore = TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

    std::uint16_t bitsPerSample = 0;
    std::ignore = TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);

    std::uint16_t extraSamples;
    std::uint16_t* samplesInfo;
    TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES,
                          &extraSamples, &samplesInfo);

    std::uint16_t photometric;
    if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric))
    {
        photometric = PHOTOMETRIC_MINISWHITE;
    }
    const bool hasAlpha = (extraSamples >= 1
        && ((samplesInfo[0] == EXTRASAMPLE_UNSPECIFIED)
            || samplesInfo[0] == EXTRASAMPLE_ASSOCALPHA
            || samplesInfo[0] == EXTRASAMPLE_UNASSALPHA))
        || (extraSamples == 0 && samplesPerPixel == 4
            && photometric == PHOTOMETRIC_RGB);

    // guard against integer overflow during multiplication which could result
    // in allocating a too small buffer and then overflowing it
    const double bytesNeeded = (double)w * (double)h * sizeof(std::uint32_t);
    if ( bytesNeeded >= std::numeric_limits<std::uint32_t>::max() )
    {
        if ( verbose )
        {
            wxLogError( _("TIFF: Image size is abnormally big.") );
        }

        TIFFClose(tif);

        return false;
    }

    raster = (std::uint32_t*) _TIFFmalloc( (std::uint32_t)bytesNeeded );

    if (!raster)
    {
        if (verbose)
        {
            wxLogError( _("TIFF: Couldn't allocate memory.") );
        }

        TIFFClose( tif );

        return false;
    }

    image->Create( (int)w, (int)h );
    if (!image->IsOk())
    {
        if (verbose)
        {
            wxLogError( _("TIFF: Couldn't allocate memory.") );
        }

        _TIFFfree( raster );
        TIFFClose( tif );

        return false;
    }

    if ( hasAlpha )
        image->SetAlpha();

    std::uint16_t planarConfig = PLANARCONFIG_CONTIG;
    std::ignore = TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);

    bool ok = true;
    char msg[1024] = "";
    if
    (
        (planarConfig == PLANARCONFIG_CONTIG && samplesPerPixel == 2
            && extraSamples == 1)
        &&
        (
            ( !TIFFRGBAImageOK(tif, msg) )
            || (bitsPerSample == 8)
        )
    )
    {
        const bool isGreyScale = (bitsPerSample == 8);
        unsigned char *buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));
        std::uint32_t pos = 0;
        const bool minIsWhite = (photometric == PHOTOMETRIC_MINISWHITE);
        const int minValue =  minIsWhite ? 255 : 0;
        const int maxValue = 255 - minValue;

        /*
        Decode to ABGR format as that is what the code, that converts to
        wxImage, later on expects (normally TIFFReadRGBAImageOriented is
        used to decode which uses an ABGR layout).
        */
        for (std::uint32_t y = 0; y < h; ++y)
        {
            if (TIFFReadScanline(tif, buf, y, 0) != 1)
            {
                ok = false;
                break;
            }

            if (isGreyScale)
            {
                for (std::uint32_t x = 0; x < w; ++x)
                {
                    std::uint8_t val = minIsWhite ? 255 - buf[x*2] : buf[x*2];
                    std::uint8_t alpha = minIsWhite ? 255 - buf[x*2+1] : buf[x*2+1];
                    raster[pos] = val + (val << 8) + (val << 16)
                        + (alpha << 24);
                    pos++;
                }
            }
            else
            {
                for (std::uint32_t x = 0; x < w; ++x)
                {
                    int mask = buf[x*2/8] << ((x*2)%8);

                    std::uint8_t val = mask & 128 ? maxValue : minValue;
                    raster[pos] = val + (val << 8) + (val << 16)
                        + ((mask & 64 ? maxValue : minValue) << 24);
                    pos++;
                }
            }
        }

        _TIFFfree(buf);
    }
    else
    {
        ok = TIFFReadRGBAImageOriented( tif, w, h, raster,
            ORIENTATION_TOPLEFT, 0 ) != 0;
    }


    if (!ok)
    {
        if (verbose)
        {
            wxLogError( _("TIFF: Error reading image.") );
        }

        _TIFFfree( raster );
        image->Destroy();
        TIFFClose( tif );

        return false;
    }

    unsigned char *ptr = image->GetData();

    unsigned char *alpha = image->GetAlpha();

    std::uint32_t pos = 0;

    for (std::uint32_t i = 0; i < h; i++)
    {
        for (std::uint32_t j = 0; j < w; j++)
        {
            *(ptr++) = (unsigned char)TIFFGetR(raster[pos]);
            *(ptr++) = (unsigned char)TIFFGetG(raster[pos]);
            *(ptr++) = (unsigned char)TIFFGetB(raster[pos]);
            if ( hasAlpha )
                *(alpha++) = (unsigned char)TIFFGetA(raster[pos]);

            pos++;
        }
    }


    image->SetOption(wxIMAGE_OPTION_TIFF_PHOTOMETRIC, photometric);

    std::uint16_t compression;
    /*
    Copy some baseline TIFF tags which helps when re-saving a TIFF
    to be similar to the original image.
    */
    if (samplesPerPixel)
    {
        image->SetOption(wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL, samplesPerPixel);
    }

    if (bitsPerSample)
    {
        image->SetOption(wxIMAGE_OPTION_TIFF_BITSPERSAMPLE, bitsPerSample);
    }

    if ( TIFFGetFieldDefaulted(tif, TIFFTAG_COMPRESSION, &compression) )
    {
        image->SetOption(wxIMAGE_OPTION_TIFF_COMPRESSION, compression);
    }

    // Set the resolution unit.
    wxImageResolution resUnit = wxImageResolution::None;
    std::uint16_t tiffRes;
    if ( TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &tiffRes) )
    {
        switch (tiffRes)
        {
            default:
                wxLogWarning(_("Unknown TIFF resolution unit %d ignored"),
                    tiffRes);
                [[fallthrough]];

            case RESUNIT_NONE:
                resUnit = wxImageResolution::None;
                break;

            case RESUNIT_INCH:
                resUnit = wxImageResolution::Inches;
                break;

            case RESUNIT_CENTIMETER:
                resUnit = wxImageResolution::Centimeters;
                break;
        }
    }

    // TODO: Add formatting for wxImageResolution enum.
    image->SetOption(wxIMAGE_OPTION_RESOLUTIONUNIT, static_cast<int>(resUnit));

    /*
    Set the image resolution if it's available. Resolution tag is not
    dependent on RESOLUTIONUNIT != RESUNIT_NONE (according to TIFF spec).
    */
    float resX, resY;

    if ( TIFFGetField(tif, TIFFTAG_XRESOLUTION, &resX) )
    {
        /*
        Use a string value to not lose precision.
        rounding to int as cm and then converting to inch may
        result in whole integer rounding error, eg. 201 instead of 200 dpi.
        If an app wants an int, GetOptionInt will convert and round down.
        */
        std::string str;
        str.resize(15); // Floating point - 15 chars
        auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), resX);
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONX, static_cast<double>(resX));
    }

    if ( TIFFGetField(tif, TIFFTAG_YRESOLUTION, &resY) )
    {
        std::string str;
        str.resize(15); // Floating point - 15 chars
        auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), resY);
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONY, static_cast<double>(resY));
    }

    _TIFFfree( raster );

    TIFFClose( tif );

    return true;
}

int wxTIFFHandler::DoGetImageCount( wxInputStream& stream )
{
    TIFF *tif = TIFFwxOpen( stream, "image", "r" );

    if (!tif)
        return 0;

    int dircount = 0;  // according to the libtiff docs, dircount should be set to 1 here???
    do {
        dircount++;
    } while (TIFFReadDirectory(tif));

    TIFFClose( tif );

    // NOTE: this function modifies the current stream position but it's ok
    //       (see wxImageHandler::GetImageCount)

    return dircount;
}

bool wxTIFFHandler::SaveFile( wxImage *image, wxOutputStream& stream, bool verbose )
{
    TIFF *tif = TIFFwxOpen( stream, "image", "w" );

    if (!tif)
    {
        if (verbose)
        {
            wxLogError( _("TIFF: Error saving image.") );
        }

        return false;
    }

    const int imageWidth = image->GetWidth();
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH,  (std::uint32_t) imageWidth);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (std::uint32_t)image->GetHeight());
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // save the image resolution if we have it
    int xres, yres;
    const wxImageResolution res = GetResolutionFromOptions(*image, &xres, &yres);
    std::uint16_t tiffRes;
    switch ( res )
    {
        default:
            wxFAIL_MSG( "unknown image resolution units" );
            [[fallthrough]];

        case wxImageResolution::None:
            tiffRes = RESUNIT_NONE;
            break;

        case wxImageResolution::Inches:
            tiffRes = RESUNIT_INCH;
            break;

        case wxImageResolution::Centimeters:
            tiffRes = RESUNIT_CENTIMETER;
            break;
    }

    if ( tiffRes != RESUNIT_NONE )
    {
        TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, tiffRes);
        TIFFSetField(tif, TIFFTAG_XRESOLUTION, xres);
        TIFFSetField(tif, TIFFTAG_YRESOLUTION, yres);
    }


    int spp = image->GetOptionInt(wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL);
    if ( !spp )
        spp = 3;

    int bps = image->GetOptionInt(wxIMAGE_OPTION_TIFF_BITSPERSAMPLE);
    if ( !bps )
    {
        bps = 8;
    }
    else if (bps == 1)
    {
        // One bit per sample combined with 3 samples per pixel is
        // not allowed and crashes libtiff.
        spp = 1;
    }

    int photometric = PHOTOMETRIC_RGB;

    if ( image->HasOption(wxIMAGE_OPTION_TIFF_PHOTOMETRIC) )
    {
        photometric = image->GetOptionInt(wxIMAGE_OPTION_TIFF_PHOTOMETRIC);
        if (photometric == PHOTOMETRIC_MINISWHITE
            || photometric == PHOTOMETRIC_MINISBLACK)
        {
            // either b/w or greyscale
            spp = 1;
        }
    }
    else if (spp <= 2)
    {
        photometric = PHOTOMETRIC_MINISWHITE;
    }

    const bool hasAlpha = image->HasAlpha();

    int compression = image->GetOptionInt(wxIMAGE_OPTION_TIFF_COMPRESSION);
    if ( !compression || (compression == COMPRESSION_JPEG && hasAlpha) )
    {
        // We can't use COMPRESSION_LZW because current version of libtiff
        // doesn't implement it ("no longer implemented due to Unisys patent
        // enforcement") and other compression methods are lossy so we
        // shouldn't use them by default -- and the only remaining one is none.
        // Also JPEG compression for alpha images is not a good idea (viewers
        // not opening the image properly).
        compression = COMPRESSION_NONE;
    }

    if
    (
        (photometric == PHOTOMETRIC_RGB && spp == 4)
        || (photometric <= PHOTOMETRIC_MINISBLACK && spp == 2)
    )
    {
        // Compensate for user passing a SamplesPerPixel that includes
        // the alpha channel.
        spp--;
    }


    int extraSamples = hasAlpha ? 1 : 0;

    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp + extraSamples);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, compression);

    if (extraSamples)
    {
        std::uint16_t extra[] = { EXTRASAMPLE_UNSPECIFIED };
        TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, (long) 1, &extra);
    }

    // scanlinesize is determined by spp+extraSamples and bps
    const tsize_t linebytes =
        (tsize_t)((imageWidth * (spp + extraSamples) * bps + 7) / 8);

    unsigned char *buf;

    const bool isColouredImage = (spp > 1)
        && (photometric != PHOTOMETRIC_MINISWHITE)
        && (photometric != PHOTOMETRIC_MINISBLACK);


    if (TIFFScanlineSize(tif) > linebytes || !isColouredImage || hasAlpha)
    {
        buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));
        if (!buf)
        {
            if (verbose)
            {
                wxLogError( _("TIFF: Couldn't allocate memory.") );
            }

            TIFFClose( tif );

            return false;
        }
    }
    else
    {
        buf = nullptr;
    }

    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,TIFFDefaultStripSize(tif, (std::uint32_t) -1));

    const int bitsPerPixel = (spp + extraSamples) * bps;
    const int bytesPerPixel = (bitsPerPixel + 7) / 8;
    const int pixelsPerByte = 8 / bitsPerPixel;
    int remainingPixelCount = 0;

    if (pixelsPerByte)
    {
        // How many pixels to write in the last byte column?
        remainingPixelCount = imageWidth % pixelsPerByte;
        if (!remainingPixelCount) remainingPixelCount = pixelsPerByte;
    }

    const bool minIsWhite = (photometric == PHOTOMETRIC_MINISWHITE);
    unsigned char *ptr = image->GetData();
    for ( int row = 0; row < image->GetHeight(); row++ )
    {
        if ( buf )
        {
            if (isColouredImage)
            {
                // colour image
                if (hasAlpha)
                {
                    for ( int column = 0; column < imageWidth; column++ )
                    {
                        buf[column*4    ] = ptr[column*3    ];
                        buf[column*4 + 1] = ptr[column*3 + 1];
                        buf[column*4 + 2] = ptr[column*3 + 2];
                        buf[column*4 + 3] = image->GetAlpha(column, row);
                    }
                }
                else
                {
                    memcpy(buf, ptr, imageWidth * 3);
                }
            }
            else if (spp * bps == 8) // greyscale image
            {
                for ( int column = 0; column < imageWidth; column++ )
                {
                    std::uint8_t value = ptr[column*3 + 1];
                    if (minIsWhite)
                    {
                        value = 255 - value;
                    }

                    buf[column * bytesPerPixel] = value;

                    if (hasAlpha)
                    {
                        value = image->GetAlpha(column, row);
                        buf[column*bytesPerPixel+1] = minIsWhite ? 255 - value
                                                                 : value;
                    }
                }
            }
            else // black and white image
            {
                for ( int column = 0; column < linebytes; column++ )
                {
                    std::uint8_t reverse = 0;
                    int pixelsPerByteCount = (column + 1 != linebytes)
                        ? pixelsPerByte
                        : remainingPixelCount;
                    for ( int bp = 0; bp < pixelsPerByteCount; bp++ )
                    {
                        if ( (ptr[column * 3 * pixelsPerByte + bp*3 + 1] <=127)
                            == minIsWhite )
                        {
                            // check only green as this is sufficient
                            reverse |= (std::uint8_t) (128 >> (bp * bitsPerPixel));
                        }

                        if (hasAlpha
                            && (image->GetAlpha(column * pixelsPerByte + bp,
                                    row) <= 127) == minIsWhite)
                        {
                            reverse |= (std::uint8_t) (64 >> (bp * bitsPerPixel));
                        }
                    }

                    buf[column] = reverse;
                }
            }
        }

        if ( TIFFWriteScanline(tif, buf ? buf : ptr, (std::uint32_t)row, 0) < 0 )
        {
            if (verbose)
            {
                wxLogError( _("TIFF: Error writing image.") );
            }

            TIFFClose( tif );
            if (buf)
                _TIFFfree(buf);

            return false;
        }

        ptr += imageWidth * 3;
    }

    (void) TIFFClose(tif);

    if (buf)
        _TIFFfree(buf);

    return true;
}

bool wxTIFFHandler::DoCanRead( wxInputStream& stream )
{
    unsigned char hdr[2];

    if ( !stream.Read(&hdr[0], WXSIZEOF(hdr)) )     // it's ok to modify the stream position here
        return false;

    return (hdr[0] == 'I' && hdr[1] == 'I') ||
           (hdr[0] == 'M' && hdr[1] == 'M');
}

#endif  // wxUSE_STREAMS

/*static*/ wxVersionInfo wxTIFFHandler::GetLibraryVersionInfo()
{
    VersionNumbering versioning;

    const std::string ver(::TIFFGetVersion());

    if ( wxSscanf(ver, "LIBTIFF, Version %d.%d.%d", &versioning.major, &versioning.minor, &versioning.micro) != 3 )
    {
        wxLogDebug("Unrecognized libtiff version string \"%s\"", ver);

        versioning = {0, 0, 0};
    }

    std::string copyright = wx::utils::AfterFirst(ver, '\n');
    const auto desc = wx::utils::BeforeFirst(ver, '\n');
    std::erase(copyright, '\n');

    return {"libtiff", versioning, desc, copyright};
}

#endif // wxUSE_LIBTIFF