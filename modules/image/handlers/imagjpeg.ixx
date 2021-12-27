// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagjpeg.ixx
// Purpose:     wxImage JPEG handler
// Author:      Vaclav Slavik
// Copyright:   (c) Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

export module WX.Image.JPEG;

import WX.Image.Base;

import WX.Utils.VersionInfo;

//-----------------------------------------------------------------------------
// wxJPEGHandler
//-----------------------------------------------------------------------------

#if wxUSE_LIBJPEG

export
{

class wxJPEGHandler: public wxImageHandler
{
public:
    wxJPEGHandler()
    {
        m_name = "JPEG file";
        m_extension = "jpg";
        m_altExtensions.push_back("jpeg");
        m_altExtensions.push_back("jpe");
        m_type = wxBitmapType::JPEG;
        m_mime = "image/jpeg";
    }

    static wxVersionInfo GetLibraryVersionInfo();

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif
};

} // export

#endif // wxUSE_LIBJPEG

