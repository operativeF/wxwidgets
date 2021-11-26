/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imagtga.h
// Purpose:     wxImage TGA handler
// Author:      Seth Jackson
// Copyright:   (c) 2005 Seth Jackson
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/gdicmn.h"

export module WX.Image.TGA;

import WX.Image.Base;

//-----------------------------------------------------------------------------
// wxTGAHandler
//-----------------------------------------------------------------------------

#if wxUSE_TGA

export
{

class wxInputStream;
class wxOutputStream;

class wxTGAHandler : public wxImageHandler
{
public:
    wxTGAHandler()
    {
        m_name = "TGA file";
        m_extension = "tga";
        m_altExtensions.push_back("tpic");
        m_type = wxBitmapType::TGA;
        m_mime = "image/tga";
    }

#if wxUSE_STREAMS
    bool LoadFile(wxImage* image, wxInputStream& stream,
                            bool verbose = true, int index = -1) override;
    bool SaveFile(wxImage* image, wxOutputStream& stream,
                             bool verbose = true) override;
protected:
    bool DoCanRead(wxInputStream& stream) override;
#endif // wxUSE_STREAMS

    wxDECLARE_DYNAMIC_CLASS(wxTGAHandler);
};

} // export

#endif // wxUSE_TGA
