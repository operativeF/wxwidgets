/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagiff.h
// Purpose:     wxImage handler for Amiga IFF images
// Author:      Steffen Gutmann
// Copyright:   (c) Steffen Gutmann, 2002
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGE_IFF_H_
#define _WX_IMAGE_IFF_H_

//-----------------------------------------------------------------------------
// wxIFFHandler
//-----------------------------------------------------------------------------

#if wxUSE_IMAGE && wxUSE_IFF

#include "wx/gdicmn.h"
#include "wx/image.h"

class wxIFFHandler : public wxImageHandler
{
public:
    wxIFFHandler()
    {
        m_name = "IFF file";
        m_extension = "iff";
        m_type = wxBitmapType::IFF;
        m_mime = "image/x-iff";
    }

#if wxUSE_STREAMS
    bool LoadFile(wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1) override;
    bool SaveFile(wxImage *image, wxOutputStream& stream, bool verbose=true) override;
protected:
    bool DoCanRead(wxInputStream& stream) override;
#endif

    wxDECLARE_DYNAMIC_CLASS(wxIFFHandler);
};

#endif // wxUSE_IMAGE && wxUSE_IFF

#endif // _WX_IMAGE_IFF_H_
