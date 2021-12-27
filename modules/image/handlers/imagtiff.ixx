// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagtiff.ixx
// Purpose:     wxImage TIFF handler
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/filefn.h"
#include "wx/log.h"
#include "wx/translation.h"
#include "wx/wxcrtvararg.h"

#include <fmt/core.h>

#include <tiff.h>
#include <tiffio.h>

export module WX.Image.TIFF;

import WX.Image.Base;
import WX.Utils.VersionInfo;
import WX.Cmn.Stream;

import <string_view>;

//-----------------------------------------------------------------------------
// wxTIFFHandler
//-----------------------------------------------------------------------------

#if wxUSE_LIBTIFF

#ifndef TIFFLINKAGEMODE
#define TIFFLINKAGEMODE
#endif

// ----------------------------------------------------------------------------
// TIFF library error/warning handlers
// ----------------------------------------------------------------------------

namespace
{

std::string FormatTiffMessage(const char *module, const char *fmt, va_list ap)
{
    char buf[512];
    if ( wxCRT_VsnprintfA(buf, WXSIZEOF(buf), fmt, ap) <= 0 )
    {
        // this isn't supposed to happen, but if it does, it's better
        // than nothing
        strcpy(buf, "Incorrectly formatted TIFF message");
    }
    buf[WXSIZEOF(buf)-1] = 0; // make sure it is always NULL-terminated

    std::string msg(buf, 512);
    if ( module )
        msg += fmt::format(_(" (in module \"%s\")"), module);

    return msg;
}

// helper to translate our, possibly 64 bit, wxFileOffset to TIFF, always 32
// bit, toff_t
toff_t wxFileOffsetToTIFF(wxFileOffset ofs)
{
    if ( ofs == wxInvalidOffset )
        return (toff_t)-1;

    toff_t tofs = wx::narrow_cast<toff_t>(ofs);
    wxCHECK_MSG( (wxFileOffset)tofs == ofs, (toff_t)-1,
                    "TIFF library doesn't support large files" );

    return tofs;
}

// another helper to convert standard seek mode to our
wxSeekMode wxSeekModeFromTIFF(int whence)
{
    switch ( whence )
    {
        case SEEK_SET:
            return wxSeekMode::FromStart;

        case SEEK_CUR:
            return wxSeekMode::FromCurrent;

        case SEEK_END:
            return wxSeekMode::FromEnd;

        default:
            return wxSeekMode::FromCurrent;
    }
}

tsize_t TIFFLINKAGEMODE
wxTIFFNullProc([[maybe_unused]] thandle_t handle,
    [[maybe_unused]] tdata_t buf,
    [[maybe_unused]] tsize_t size)
{
    return (tsize_t)-1;
}

tsize_t TIFFLINKAGEMODE
wxTIFFReadProc(thandle_t handle, tdata_t buf, tsize_t size)
{
    wxInputStream* stream = (wxInputStream*)handle;
    stream->Read((void*)buf, (size_t)size);
    return wx::narrow_cast<tsize_t>(stream->LastRead());
}

tsize_t TIFFLINKAGEMODE
wxTIFFWriteProc(thandle_t handle, tdata_t buf, tsize_t size)
{
    wxOutputStream* stream = (wxOutputStream*)handle;
    stream->Write((void*)buf, (size_t)size);
    return wx::narrow_cast<tsize_t>(stream->LastWrite());
}

toff_t TIFFLINKAGEMODE
wxTIFFSeekIProc(thandle_t handle, toff_t off, int whence)
{
    wxInputStream* stream = (wxInputStream*)handle;

    return wxFileOffsetToTIFF(stream->SeekI((wxFileOffset)off,
        wxSeekModeFromTIFF(whence)));
}

toff_t TIFFLINKAGEMODE
wxTIFFSeekOProc(thandle_t handle, toff_t off, int whence)
{
    wxOutputStream* stream = (wxOutputStream*)handle;

    toff_t offset = wxFileOffsetToTIFF(
        stream->SeekO((wxFileOffset)off, wxSeekModeFromTIFF(whence)));

    if (offset != (toff_t)-1 || whence != SEEK_SET)
    {
        return offset;
    }


    /*
    Try to workaround problems with libtiff seeking past the end of streams.

    This occurs when libtiff is writing tag entries past the end of a
    stream but hasn't written the directory yet (which will be placed
    before the tags and contain offsets to the just written tags).
    The behaviour for seeking past the end of a stream is not consistent
    and doesn't work with for example wxMemoryOutputStream. When this type
    of seeking fails (with SEEK_SET), fill in the gap with zeroes and try
    again.
    */

    wxFileOffset streamLength = stream->GetLength();
    if (streamLength != wxInvalidOffset && (wxFileOffset)off > streamLength)
    {
        if (stream->SeekO(streamLength, wxSeekMode::FromStart) == wxInvalidOffset)
        {
            return (toff_t)-1;
        }

        for (wxFileOffset i = 0; i < (wxFileOffset)off - streamLength; ++i)
        {
            stream->PutC(0);
        }
    }

    return wxFileOffsetToTIFF(stream->TellO());
}

int TIFFLINKAGEMODE
wxTIFFCloseIProc([[maybe_unused]] thandle_t handle)
{
    // there is no need to close the input stream
    return 0;
}

int TIFFLINKAGEMODE
wxTIFFCloseOProc(thandle_t handle)
{
    wxOutputStream* stream = (wxOutputStream*)handle;

    return stream->Close() ? 0 : -1;
}

toff_t TIFFLINKAGEMODE
wxTIFFSizeProc(thandle_t handle)
{
    wxStreamBase* stream = (wxStreamBase*)handle;
    return (toff_t)stream->GetSize();
}

int TIFFLINKAGEMODE
wxTIFFMapProc([[maybe_unused]] thandle_t handle,
    [[maybe_unused]] tdata_t* pbase,
    [[maybe_unused]] toff_t* psize)
{
    return 0;
}

void TIFFLINKAGEMODE
wxTIFFUnmapProc([[maybe_unused]] thandle_t handle,
    [[maybe_unused]] tdata_t base,
    [[maybe_unused]] toff_t size)
{
}

} // namespace anonymous

export
{

extern "C"
{

void TIFFwxWarningHandler(const char* module, const char* fmt, va_list ap)
{
    wxLogWarning("{:s}", FormatTiffMessage(module, fmt, ap));
}

void TIFFwxErrorHandler(const char* module, const char* fmt, va_list ap)
{
    wxLogError("{:s}", FormatTiffMessage(module, fmt, ap));
}

} // extern "C"

TIFF* TIFFwxOpen(wxInputStream& stream, const char* name, const char* mode)
{
    TIFF* tif = TIFFClientOpen(name, mode,
        (thandle_t)&stream,
        wxTIFFReadProc, wxTIFFNullProc,
        wxTIFFSeekIProc, wxTIFFCloseIProc, wxTIFFSizeProc,
        wxTIFFMapProc, wxTIFFUnmapProc);

    return tif;
}

TIFF* TIFFwxOpen(wxOutputStream& stream, const char* name, const char* mode)
{
    TIFF* tif = TIFFClientOpen(name, mode,
        (thandle_t)&stream,
        wxTIFFNullProc, wxTIFFWriteProc,
        wxTIFFSeekOProc, wxTIFFCloseOProc, wxTIFFSizeProc,
        wxTIFFMapProc, wxTIFFUnmapProc);

    return tif;
}

// defines for wxImage::SetOption
inline constexpr std::string_view wxIMAGE_OPTION_TIFF_BITSPERSAMPLE   = "BitsPerSample";
inline constexpr std::string_view wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL = "SamplesPerPixel";
inline constexpr std::string_view wxIMAGE_OPTION_TIFF_COMPRESSION     = "Compression";
inline constexpr std::string_view wxIMAGE_OPTION_TIFF_PHOTOMETRIC     = "Photometric";
inline constexpr std::string_view wxIMAGE_OPTION_TIFF_IMAGEDESCRIPTOR = "ImageDescriptor";

class wxTIFFHandler: public wxImageHandler
{
public:
    wxTIFFHandler();

    static wxVersionInfo GetLibraryVersionInfo();

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;

protected:
    int DoGetImageCount( wxInputStream& stream ) override;
    bool DoCanRead( wxInputStream& stream ) override;
#endif
};

} // export

#endif // wxUSE_LIBTIFF
