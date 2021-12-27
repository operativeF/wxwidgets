// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagpnm.ixx
// Purpose:     wxImage PNM handler
// Author:      Sylvain Bougnoux
// Copyright:   (c) Sylvain Bougnoux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

export module WX.Image.PNM;

import WX.Image.Base;

//-----------------------------------------------------------------------------
// wxPNMHandler
//-----------------------------------------------------------------------------

#if wxUSE_PNM

export
{

class wxPNMHandler : public wxImageHandler
{
public:
    wxPNMHandler()
    {
        m_name = "PNM file";
        m_extension = "pnm";
        m_altExtensions.push_back("ppm");
        m_altExtensions.push_back("pgm");
        m_altExtensions.push_back("pbm");
        m_type = wxBitmapType::PNM;
        m_mime = "image/pnm";
    }

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif
};

} // export

#endif // wxUSE_PNM
