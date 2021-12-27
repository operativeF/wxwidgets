// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/imagani.cpp
// Purpose:     wxImage BMP, ICO, CUR and ANI handlers
// Author:      Robert Roebling, Chris Elliott
// Copyright:   (c) Robert Roebling, Chris Elliott
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

module WX.Image.ANI;

import WX.Image.Decoder.ANI;

#if wxUSE_ICO_CUR && wxUSE_STREAMS

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
        return -1; // FIXME: optional?

    return decoder.GetFrameCount();
}

#endif // wxUSE_ICO_CUR && wxUSE_STREAMS
