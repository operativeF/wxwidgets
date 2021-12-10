export module WX.Image;

export import WX.Image.ANI;
export import WX.Image.BMP;
export import WX.Image.CUR;
export import WX.Image.GIF;
export import WX.Image.IFF;
export import WX.Image.ICO;
export import WX.Image.JPEG;
export import WX.Image.PCX;
export import WX.Image.PNG;
export import WX.Image.PNM;
export import WX.Image.TGA;
export import WX.Image.TIFF;
export import WX.Image.XPM;

export import WX.Image.Base;

export import WX.Image.Decoder.ANI;
export import WX.Image.Decoder.GIF;

export
{
    //-----------------------------------------------------------------------------
// This function allows dynamic access to all image handlers compile within
// the library. This function should be in a separate file as some compilers
// link against the whole object file as long as just one of is function is called!

void wxInitAllImageHandlers()
{
    #if wxUSE_LIBPNG
    wxImage::AddHandler( new wxPNGHandler );
    #endif
    #if wxUSE_LIBJPEG
    wxImage::AddHandler( new wxJPEGHandler );
    #endif
    #if wxUSE_LIBTIFF
    wxImage::AddHandler( new wxTIFFHandler );
    #endif
    #if wxUSE_GIF
    wxImage::AddHandler( new wxGIFHandler );
    #endif
    #if wxUSE_PNM
    wxImage::AddHandler( new wxPNMHandler );
    #endif
    #if wxUSE_PCX
    wxImage::AddHandler( new wxPCXHandler );
    #endif
    #if wxUSE_IFF
    wxImage::AddHandler( new wxIFFHandler );
    #endif
    #if wxUSE_ICO_CUR
    wxImage::AddHandler( new wxICOHandler );
    wxImage::AddHandler( new wxCURHandler );
    wxImage::AddHandler( new wxANIHandler );
    #endif
    #if wxUSE_TGA
    wxImage::AddHandler( new wxTGAHandler );
    #endif
    #if wxUSE_XPM
    wxImage::AddHandler( new wxXPMHandler );
    #endif
}

} // export
