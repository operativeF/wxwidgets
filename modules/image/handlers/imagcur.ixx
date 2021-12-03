module;

#include "wx/stream.h"

export module WX.Image.CUR;

import WX.Image.ICO;

#ifdef wxUSE_ICO_CUR

export
{

// ----------------------------------------------------------------------------
// wxCURHandler
// ----------------------------------------------------------------------------

class wxCURHandler : public wxICOHandler
{
public:
    wxCURHandler()
    {
        m_name = "Windows cursor file";
        m_extension = "cur";
        m_type = wxBitmapType::CUR;
        m_mime = "image/x-cur";
    }

    // VS: This handler's meat is implemented inside wxICOHandler (the two
    //     formats are almost identical), but we hide this fact at
    //     the API level, since it is a mere implementation detail.

protected:
#if wxUSE_STREAMS
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS
};

} // export

#endif // wxUSE_ICO_CUR