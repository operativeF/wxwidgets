module;

#include "wx/defs.h"

export module WX.Image.ANI;

import WX.Image.CUR;
import WX.Image.Base;
import WX.Image.Decoder.ANI;

import WX.Cmn.Stream;

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

#ifdef wxUSE_STREAMS

bool wxANIHandler::LoadFile(wxImage *image, wxInputStream& stream,
                            [[maybe_unused]] bool verbose, int index)
{
    wxANIDecoder decoder;
    if (!decoder.Load(stream))
        return false;

    return decoder.ConvertToImage(index != -1 ? (size_t)index : 0, image);
}

bool wxANIHandler::DoCanRead(wxInputStream& stream)
{
    wxANIDecoder decod;
    return decod.CanRead(stream);
             // it's ok to modify the stream position here
}

int wxANIHandler::DoGetImageCount(wxInputStream& stream)
{
    wxANIDecoder decoder;
    if (!decoder.Load(stream))  // it's ok to modify the stream position here
        return wxNOT_FOUND;

    return decoder.GetFrameCount();
}

#endif // wxUSE_STREAMS

#endif // wxUSE_ICO_CUR;