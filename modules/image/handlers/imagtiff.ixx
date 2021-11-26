/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagtiff.h
// Purpose:     wxImage TIFF handler
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"
#include "wx/versioninfo.h"

export module WX.Image.TIFF;

import WX.Image.Base;

//-----------------------------------------------------------------------------
// wxTIFFHandler
//-----------------------------------------------------------------------------

#if wxUSE_LIBTIFF

export
{

class wxInputStream;
class wxOutputStream;

// defines for wxImage::SetOption
inline const wxString wxIMAGE_OPTION_TIFF_BITSPERSAMPLE   = "BitsPerSample";
inline const wxString wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL = "SamplesPerPixel";
inline const wxString wxIMAGE_OPTION_TIFF_COMPRESSION     = "Compression";
inline const wxString wxIMAGE_OPTION_TIFF_PHOTOMETRIC     = "Photometric";
inline const wxString wxIMAGE_OPTION_TIFF_IMAGEDESCRIPTOR = "ImageDescriptor";

// for backwards compatibility
inline const wxString wxIMAGE_OPTION_BITSPERSAMPLE        = wxIMAGE_OPTION_TIFF_BITSPERSAMPLE;
inline const wxString wxIMAGE_OPTION_SAMPLESPERPIXEL      = wxIMAGE_OPTION_TIFF_SAMPLESPERPIXEL;
inline const wxString wxIMAGE_OPTION_COMPRESSION          = wxIMAGE_OPTION_TIFF_COMPRESSION;
inline const wxString wxIMAGE_OPTION_IMAGEDESCRIPTOR      = wxIMAGE_OPTION_TIFF_IMAGEDESCRIPTOR;

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

private:
    wxDECLARE_DYNAMIC_CLASS(wxTIFFHandler);
};

} // export

#endif // wxUSE_LIBTIFF
