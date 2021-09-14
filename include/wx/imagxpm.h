/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagxpm.h
// Purpose:     wxImage XPM handler
// Author:      Vaclav Slavik
// Copyright:   (c) 2001 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGXPM_H_
#define _WX_IMAGXPM_H_

#include "wx/image.h"

#if wxUSE_XPM

//-----------------------------------------------------------------------------
// wxXPMHandler
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxXPMHandler : public wxImageHandler
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

#endif // wxUSE_XPM

#endif // _WX_IMAGXPM_H_
