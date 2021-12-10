/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gifdecod.h
// Purpose:     wxGIFDecoder, GIF reader for wxImage and wxAnimation
// Author:      Guillermo Rodriguez Garcia <guille@iies.es>
// Version:     3.02
// Copyright:   (c) 1999 Guillermo Rodriguez Garcia
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/stream.h"
#include "wx/animdecod.h"

#include <chrono>

export module WX.Image.Decoder.GIF;

import <vector>;

#if wxUSE_STREAMS && wxUSE_GIF

using namespace std::chrono_literals;

export
{

// internal utility used to store a frame in 8bit-per-pixel format
class wxImage;

// internal class for storing GIF image data
struct GIFImage
{
    GIFImage& operator=(GIFImage&&) = delete;

    wxString comment;

    std::vector<unsigned char> p;      // bitmap

    std::chrono::milliseconds delay{ -1ms };                 // delay in ms (-1 = unused)

    unsigned char* pal{ nullptr };    // palette

    unsigned int w{0};                 // width
    unsigned int h{0};                 // height
    unsigned int left{0};              // x coord (in logical screen)
    unsigned int top{0};               // y coord (in logical screen
    unsigned int ncolours{ 0 };       // number of colours

    int transparent{0};                // transparent color index (-1 = none)

    wxAnimationDisposal disposal{ wxANIM_DONOTREMOVE };   // disposal method
};

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

class wxGIFDecoder : public wxAnimationDecoder
{
public:
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
    std::vector<std::unique_ptr<GIFImage>> m_frames;

    unsigned char *m_bufp{nullptr};          // pointer to next byte in buffer

    // decoder state vars
    int           m_restbits{};       // remaining valid bits
    unsigned int  m_restbyte{};       // remaining bytes in this block
    unsigned int  m_lastbyte{};       // last byte read
};

} // export

#endif // wxUSE_STREAMS && wxUSE_GIF
