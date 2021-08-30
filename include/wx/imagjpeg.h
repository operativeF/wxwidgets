/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagjpeg.h
// Purpose:     wxImage JPEG handler
// Author:      Vaclav Slavik
// Copyright:   (c) Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGJPEG_H_
#define _WX_IMAGJPEG_H_

#include "wx/defs.h"

//-----------------------------------------------------------------------------
// wxJPEGHandler
//-----------------------------------------------------------------------------

#if wxUSE_LIBJPEG

#include "wx/image.h"
#include "wx/versioninfo.h"

class WXDLLIMPEXP_CORE wxJPEGHandler: public wxImageHandler
{
public:
    wxJPEGHandler() noexcept
    {
        m_name = "JPEG file";
        m_extension = "jpg";
        m_altExtensions.push_back("jpeg");
        m_altExtensions.push_back("jpe");
        m_type = wxBITMAP_TYPE_JPEG;
        m_mime = "image/jpeg";
    }

    static wxVersionInfo GetLibraryVersionInfo();

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif

private:
    wxDECLARE_DYNAMIC_CLASS(wxJPEGHandler);
};

#endif // wxUSE_LIBJPEG

#endif // _WX_IMAGJPEG_H_

