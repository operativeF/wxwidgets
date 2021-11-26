/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagxpm.h
// Purpose:     wxImage XPM handler
// Author:      Vaclav Slavik
// Copyright:   (c) 2001 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"

export module WX.Image.XPM;

import WX.Image.Base;

#if wxUSE_XPM

export
{

class wxInputStream;
class wxOutputStream;

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

private:
    wxDECLARE_DYNAMIC_CLASS(wxXPMHandler);
};

} // export

#endif // wxUSE_XPM
