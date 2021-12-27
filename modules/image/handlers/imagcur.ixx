// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        modules/image/handlers/imagcur.ixx
// Purpose:     wxImage BMP, ICO, CUR and ANI handlers
// Author:      Robert Roebling, Chris Elliott
// Copyright:   (c) Robert Roebling, Chris Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

export module WX.Image.CUR;

import WX.Cmn.Stream;
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
    bool DoCanRead( wxInputStream& stream ) override
    {
        return CanReadICOOrCUR(&stream, 2 /*for identifying a cursor*/);
    }
#endif // wxUSE_STREAMS
};

} // export

#endif // wxUSE_ICO_CUR