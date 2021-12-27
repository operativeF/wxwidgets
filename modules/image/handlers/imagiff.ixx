// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagiff.ixx
// Purpose:     wxImage handler for Amiga IFF images
// Author:      Steffen Gutmann
// Copyright:   (c) Steffen Gutmann, 2002
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

export module WX.Image.IFF;

import WX.Image.Base;

//-----------------------------------------------------------------------------
// wxIFFHandler
//-----------------------------------------------------------------------------

#if wxUSE_IMAGE && wxUSE_IFF

export
{

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
};

} // export

#endif // wxUSE_IMAGE && wxUSE_IFF
