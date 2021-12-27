// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagxpm.ixx
// Purpose:     wxImage XPM handler
// Author:      Vaclav Slavik
// Copyright:   (c) 2001 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

export module WX.Image.XPM;

import WX.Image.Base;

#if wxUSE_XPM

export
{

//-----------------------------------------------------------------------------
// wxXPMHandler
//-----------------------------------------------------------------------------

class wxXPMHandler : public wxImageHandler
{
public:
    wxXPMHandler()
    {
        m_name = "XPM file";
        m_extension = "xpm";
        m_type = wxBitmapType::XPM;
        m_mime = "image/xpm";
    }

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif
};

} // export

#endif // wxUSE_XPM
