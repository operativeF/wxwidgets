/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gifdecod.h
// Purpose:     wxGIFDecoder, GIF reader for wxImage and wxAnimation
// Author:      Guillermo Rodriguez Garcia <guille@iies.es>
// Version:     3.02
// Copyright:   (c) 1999 Guillermo Rodriguez Garcia
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GIFDECOD_H_
#define _WX_GIFDECOD_H_

#include "wx/defs.h"

#if wxUSE_STREAMS && wxUSE_GIF

#include "wx/stream.h"
#include "wx/animdecod.h"

#include <chrono>
#include <vector>

// internal utility used to store a frame in 8bit-per-pixel format
struct GIFImage;
class WXDLLIMPEXP_FWD_CORE wxImage;



// --------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------

// Error codes:
//  Note that the error code wxGIFErrorCode::Truncated means that the image itself
//  is most probably OK, but the decoder didn't reach the end of the data
//  stream; this means that if it was not reading directly from file,
//  the stream will not be correctly positioned.
//
enum class wxGIFErrorCode
{
    OK,                   // everything was OK
    InvFormat,            // error in GIF header
    MemErr,               // error allocating memory
    Truncated             // file appears to be truncated
};

// --------------------------------------------------------------------------
// wxGIFDecoder class
// --------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxGIFDecoder : public wxAnimationDecoder
{
public:
    ~wxGIFDecoder();

    wxGIFDecoder& operator=(wxGIFDecoder&&) = delete;

    // get data of current frame
    unsigned char* GetData(unsigned int frame) const;
    unsigned char* GetPalette(unsigned int frame) const;
    unsigned int GetNcolours(unsigned int frame) const;
    int GetTransparentColourIndex(unsigned int frame) const;
    wxColour GetTransparentColour(unsigned int frame) const override;

    wxSize GetFrameSize(unsigned int frame) const override;
    wxPoint GetFramePosition(unsigned int frame) const override;
    wxAnimationDisposal GetDisposalMethod(unsigned int frame) const override;
    std::chrono::milliseconds GetDelay(unsigned int frame) const override;

    // GIFs can contain both static images and animations
    bool IsAnimation() const
        { return m_nFrames > 1; }

    // load function which returns more info than just Load():
    wxGIFErrorCode LoadGIF( wxInputStream& stream );

    // free all internal frames
    void Destroy();

    // implementation of wxAnimationDecoder's pure virtuals
    bool Load( wxInputStream& stream ) override
        { return LoadGIF(stream) == wxGIFErrorCode::OK; }

    bool ConvertToImage(unsigned int frame, wxImage *image) const override;

    wxAnimationDecoder *Clone() const override
        { return new wxGIFDecoder; }
    wxAnimationType GetType() const override
        { return wxANIMATION_TYPE_GIF; }

protected:
    // wxAnimationDecoder pure virtual
    bool DoCanRead( wxInputStream& stream ) const override;
        // modifies current stream position (see wxAnimationDecoder::CanRead)

private:
    int getcode(wxInputStream& stream, int bits, int abfin);
    wxGIFErrorCode dgif(wxInputStream& stream,
                        GIFImage *img, int interl, int bits);

    unsigned char m_buffer[256];    // buffer for reading

    // array of all frames
    std::vector<void*> m_frames;

    unsigned char *m_bufp{nullptr};          // pointer to next byte in buffer

    // decoder state vars
    int           m_restbits{};       // remaining valid bits
    unsigned int  m_restbyte{};       // remaining bytes in this block
    unsigned int  m_lastbyte{};       // last byte read
};

#endif // wxUSE_STREAMS && wxUSE_GIF

#endif // _WX_GIFDECOD_H_
