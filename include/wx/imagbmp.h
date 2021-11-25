/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagbmp.h
// Purpose:     wxImage BMP, ICO, CUR and ANI handlers
// Author:      Robert Roebling, Chris Elliott
// Copyright:   (c) Robert Roebling, Chris Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGBMP_H_
#define _WX_IMAGBMP_H_

#include "wx/image.h"

// defines for saving the BMP file in different formats, Bits Per Pixel
// USE: wximage.SetOption( wxIMAGE_OPTION_BMP_FORMAT, wxBMP_xBPP );
inline constexpr char wxIMAGE_OPTION_BMP_FORMAT[] = "wxBMP_FORMAT";

// These two options are filled in upon reading CUR file and can (should) be
// specified when saving a CUR file - they define the hotspot of the cursor:
inline constexpr char wxIMAGE_OPTION_CUR_HOTSPOT_X[]  = "HotSpotX";
inline constexpr char wxIMAGE_OPTION_CUR_HOTSPOT_Y[]  = "HotSpotY";


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

private:
    wxDECLARE_DYNAMIC_CLASS(wxBMPHandler);
};

#if wxUSE_ICO_CUR
// ----------------------------------------------------------------------------
// wxICOHandler
// ----------------------------------------------------------------------------

class wxICOHandler : public wxBMPHandler
{
public:
    wxICOHandler()
    {
        m_name = "Windows icon file";
        m_extension = "ico";
        m_type = wxBitmapType::ICO;
        m_mime = "image/x-ico";
    }

#if wxUSE_STREAMS
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    virtual bool DoLoadFile( wxImage *image, wxInputStream& stream, bool verbose, int index );

protected:
    int DoGetImageCount( wxInputStream& stream ) override;
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS

private:
    wxDECLARE_DYNAMIC_CLASS(wxICOHandler);
};


// ----------------------------------------------------------------------------
// wxCURHandler
// ----------------------------------------------------------------------------

class wxCURHandler : public wxICOHandler
{
public:
    wxCURHandler()
    {
        m_name = "Windows cursor file";
        m_extension = "cur";
        m_type = wxBitmapType::CUR;
        m_mime = "image/x-cur";
    }

    // VS: This handler's meat is implemented inside wxICOHandler (the two
    //     formats are almost identical), but we hide this fact at
    //     the API level, since it is a mere implementation detail.

protected:
#if wxUSE_STREAMS
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS

private:
    wxDECLARE_DYNAMIC_CLASS(wxCURHandler);
};
// ----------------------------------------------------------------------------
// wxANIHandler
// ----------------------------------------------------------------------------

class wxANIHandler : public wxCURHandler
{
public:
    wxANIHandler()
    {
        m_name = "Windows animated cursor file";
        m_extension = "ani";
        m_type = wxBitmapType::ANI;
        m_mime = "image/x-ani";
    }


#if wxUSE_STREAMS
    bool SaveFile( [[maybe_unused]] wxImage *image, [[maybe_unused]] wxOutputStream& stream, [[maybe_unused]] bool verbose ) override{return false ;}
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;

protected:
    int DoGetImageCount( wxInputStream& stream ) override;
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS

private:
    wxDECLARE_DYNAMIC_CLASS(wxANIHandler);
};

#endif // wxUSE_ICO_CUR
#endif // _WX_IMAGBMP_H_
