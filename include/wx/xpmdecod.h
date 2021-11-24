/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xpmdecod.h
// Purpose:     wxXPMDecoder, XPM reader for wxImage and wxBitmap
// Author:      Vaclav Slavik
// Copyright:   (c) 2001 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XPMDECOD_H_
#define _WX_XPMDECOD_H_

#if wxUSE_IMAGE && wxUSE_XPM

class wxImage;
class wxInputStream;

// --------------------------------------------------------------------------
// wxXPMDecoder class
// --------------------------------------------------------------------------

class wxXPMDecoder
{
public:
#if wxUSE_STREAMS
    // Is the stream XPM file?
    // NOTE: this function modifies the current stream position
    bool CanRead(wxInputStream& stream);

    // Read XPM file from the stream, parse it and create image from it
    wxImage ReadFile(wxInputStream& stream);
#endif

    // Read directly from XPM data (as passed to wxBitmap ctor):
    wxImage ReadData(const char* const* xpm_data);

};

#endif // wxUSE_IMAGE && wxUSE_XPM

#endif  // _WX_XPM_H_
