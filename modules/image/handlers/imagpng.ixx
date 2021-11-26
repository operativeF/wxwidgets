/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagpng.h
// Purpose:     wxImage PNG handler
// Author:      Robert Roebling
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"
#include "wx/versioninfo.h"

export module WX.Image.PNG;

import WX.Image.Base;

//-----------------------------------------------------------------------------
// wxPNGHandler
//-----------------------------------------------------------------------------

#if wxUSE_LIBPNG

export
{

class wxInputStream;
class wxOutputStream;

inline constexpr char wxIMAGE_OPTION_PNG_FORMAT[]                   = "PngFormat";
inline constexpr char wxIMAGE_OPTION_PNG_BITDEPTH[]                 = "PngBitDepth";
inline constexpr char wxIMAGE_OPTION_PNG_FILTER[]                   = "PngF";
inline constexpr char wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL[]        = "PngZL";
inline constexpr char wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL[]    = "PngZM";
inline constexpr char wxIMAGE_OPTION_PNG_COMPRESSION_STRATEGY[]     = "PngZS";
inline constexpr char wxIMAGE_OPTION_PNG_COMPRESSION_BUFFER_SIZE[]  = "PngZB";

class wxPNGHandler: public wxImageHandler
{
public:
    wxPNGHandler()
    {
        m_name = "PNG file";
        m_extension = "png";
        m_type = wxBitmapType::PNG;
        m_mime = "image/png";
    }

    static wxVersionInfo GetLibraryVersionInfo();

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif

private:
    wxDECLARE_DYNAMIC_CLASS(wxPNGHandler);
};

} // export

#endif // wxUSE_LIBPNG
