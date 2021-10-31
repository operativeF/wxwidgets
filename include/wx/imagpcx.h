/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagpcx.h
// Purpose:     wxImage PCX handler
// Author:      Guillermo Rodriguez Garcia <guille@iies.es>
// Copyright:   (c) 1999 Guillermo Rodriguez Garcia
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGPCX_H_
#define _WX_IMAGPCX_H_

#if wxUSE_PCX

#include "wx/gdicmn.h"
#include "wx/image.h"

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
#endif // wxUSE_PCX


#endif
  // _WX_IMAGPCX_H_

