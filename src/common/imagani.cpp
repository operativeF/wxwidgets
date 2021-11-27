module;

#include "wx/stream.h"

module WX.Image.ANI;

import WX.Image.Base;
import WX.Image.Decoder.ANI;

wxIMPLEMENT_DYNAMIC_CLASS(wxANIHandler, wxCURHandler);

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
