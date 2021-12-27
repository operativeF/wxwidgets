// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagbmp.ixx
// Purpose:     wxImage BMP, ICO, CUR and ANI handlers
// Author:      Robert Roebling, Chris Elliott
// Copyright:   (c) Robert Roebling, Chris Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/filefn.h"

export module WX.Image.BMP;

import WX.Image.Base;

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

} // export
