module;

#include "wx/stream.h"

export module WX.Image.ANI;

import WX.Image.CUR;

#ifdef wxUSE_ICO_CUR

export
{

// ----------------------------------------------------------------------------
// wxANIHandler
// ----------------------------------------------------------------------------

class wxANIHandler : public wxCURHandler
{
public:
    wxANIHandler()
    {
        m_name = "Windows animated cursor file";
        m_extension = "ani";
        m_type = wxBitmapType::ANI;
        m_mime = "image/x-ani";
    }


#if wxUSE_STREAMS
    bool SaveFile( [[maybe_unused]] wxImage *image, [[maybe_unused]] wxOutputStream& stream, [[maybe_unused]] bool verbose ) override{return false ;}
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;

protected:
    int DoGetImageCount( wxInputStream& stream ) override;
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS
};

} // export

#endif // wxUSE_ICO_CUR;