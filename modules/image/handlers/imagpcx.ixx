/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagpcx.h
// Purpose:     wxImage PCX handler
// Author:      Guillermo Rodriguez Garcia <guille@iies.es>
// Copyright:   (c) 1999 Guillermo Rodriguez Garcia
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"
#include "wx/stream.h"

export module WX.Image.PCX;

import WX.Image.Base;

#if wxUSE_PCX

export
{

//-----------------------------------------------------------------------------
// wxPCXHandler
//-----------------------------------------------------------------------------

class wxPCXHandler : public wxImageHandler
{
public:
    wxPCXHandler()
    {
        m_name = "PCX file";
        m_extension = "pcx";
        m_type = wxBitmapType::PCX;
        m_mime = "image/pcx";
    }

#if wxUSE_STREAMS
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
protected:
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS

private:
    wxDECLARE_DYNAMIC_CLASS(wxPCXHandler);
};

} // export

#endif // wxUSE_PCX
