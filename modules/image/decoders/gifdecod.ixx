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
#include "wx/filefn.h"
#include "wx/gdicmn.h"
#include "wx/palette.h"
#include "wx/intl.h"
#include "wx/log.h"

#include <chrono>

export module WX.Image.Decoder.GIF;

import WX.Image.Base;

import WX.Utils.Cast;

import <array>;
import <string>;
import <tuple>;
import <vector>;

#if wxUSE_STREAMS && wxUSE_GIF

using namespace std::chrono_literals;

export
{

enum
{
    GIF_MARKER_EXT = '!', // 0x21
    GIF_MARKER_SEP = ',', // 0x2C
    GIF_MARKER_ENDOFDATA = ';', // 0x3B

    GIF_MARKER_EXT_GRAPHICS_CONTROL = 0xF9,
    GIF_MARKER_EXT_COMMENT = 0xFE,
    GIF_MARKER_EXT_APP = 0xFF
};

inline constexpr char wxIMAGE_OPTION_GIF_COMMENT[] = "GifComment";

inline constexpr char wxIMAGE_OPTION_GIF_TRANSPARENCY[]           = "Transparency";
inline constexpr char wxIMAGE_OPTION_GIF_TRANSPARENCY_HIGHLIGHT[] = "Highlight";
inline constexpr char wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED[] = "Unchanged";

// internal utility used to store a frame in 8bit-per-pixel format

// internal class for storing GIF image data
struct GIFImage
{
    GIFImage& operator=(GIFImage&&) = delete;

    std::string comment;

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

// This function was designed by Vaclav Slavik

bool wxGIFDecoder::ConvertToImage(unsigned int frame, wxImage* image) const
{
    unsigned char* src, * dst, * pal;
    unsigned long i;
    int      transparent;

    // Store the original value of the transparency option, before it is reset
    // by Create().
    const std::string&
        transparency = image->GetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY);

    // create the image
    const wxSize sz = GetFrameSize(frame);
    image->Create(sz.x, sz.y);
    image->SetType(wxBitmapType::GIF);

    if (!image->IsOk())
        return false;

    pal = GetPalette(frame);
    src = GetData(frame);
    dst = image->GetData();
    transparent = GetTransparentColourIndex(frame);

    // set transparent colour mask
    if (transparent != -1)
    {
        if (transparency.empty() ||
            transparency == wxIMAGE_OPTION_GIF_TRANSPARENCY_HIGHLIGHT)
        {
            // By default, we assign bright pink to transparent pixels to make
            // them perfectly noticeable if someone accidentally draws the
            // image without taking transparency into account. Due to this use
            // of pink, we need to change any existing image pixels with this
            // colour to use something different.
            for (i = 0; i < GetNcolours(frame); i++)
            {
                if ((pal[3 * i + 0] == 255) &&
                    (pal[3 * i + 1] == 0) &&
                    (pal[3 * i + 2] == 255))
                {
                    pal[3 * i + 2] = 254;
                }
            }

            pal[3 * transparent + 0] = 255;
            pal[3 * transparent + 1] = 0;
            pal[3 * transparent + 2] = 255;

            image->SetMaskColour(255, 0, 255);
        }
        else if (transparency == wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED)
        {
            // Leave the GIF exactly as it was, just adjust (in the least
            // noticeable way, by just flipping a single bit) non-transparent
            // pixels colour,
            for (i = 0; i < GetNcolours(frame); i++)
            {
                if ((pal[3 * i + 0] == pal[3 * transparent + 0]) &&
                    (pal[3 * i + 1] == pal[3 * transparent + 1]) &&
                    (pal[3 * i + 2] == pal[3 * transparent + 2]))
                {
                    pal[3 * i + 2] ^= 1;
                }
            }

            image->SetMaskColour(pal[3 * transparent + 0],
                pal[3 * transparent + 1],
                pal[3 * transparent + 2]);
        }
        else
        {
            wxFAIL_MSG("Unknown wxIMAGE_OPTION_GIF_TRANSPARENCY value");
        }
    }
    else
    {
        image->SetMask(false);
    }

#if wxUSE_PALETTE
    unsigned char r[256];
    unsigned char g[256];
    unsigned char b[256];

    for (i = 0; i < 256; i++)
    {
        r[i] = pal[3*i + 0];
        g[i] = pal[3*i + 1];
        b[i] = pal[3*i + 2];
    }

    image->SetPalette(wxPalette(GetNcolours(frame), r, g, b));
#endif // wxUSE_PALETTE

    // copy image data
    const unsigned long npixel = sz.x * sz.y;
    for (i = 0; i < npixel; i++, src++)
    {
        *(dst++) = pal[3 * (*src) + 0];
        *(dst++) = pal[3 * (*src) + 1];
        *(dst++) = pal[3 * (*src) + 2];
    }

    if ( !m_frames[frame]->comment.empty() )
    {
        image->SetOption(wxIMAGE_OPTION_GIF_COMMENT, m_frames[frame]->comment);
    }

    return true;
}


//---------------------------------------------------------------------------
// Data accessors
//---------------------------------------------------------------------------

// Get data for current frame

wxSize wxGIFDecoder::GetFrameSize(unsigned int frame) const
{
    // FIXME: Narrowing
    return {wx::narrow_cast<int>(m_frames[frame]->w), wx::narrow_cast<int>(m_frames[frame]->h)};
}

wxPoint wxGIFDecoder::GetFramePosition(unsigned int frame) const
{
    return { wx::narrow_cast<int>(m_frames[frame]->left), wx::narrow_cast<int>(m_frames[frame]->top)};
}

wxAnimationDisposal wxGIFDecoder::GetDisposalMethod(unsigned int frame) const
{
    return m_frames[frame]->disposal;
}

std::chrono::milliseconds wxGIFDecoder::GetDelay(unsigned int frame) const
{
    return m_frames[frame]->delay;
}

wxColour wxGIFDecoder::GetTransparentColour(unsigned int frame) const
{
    unsigned char *pal = m_frames[frame]->pal;
    const int n = m_frames[frame]->transparent;
    if (n == -1)
        return wxNullColour;

    return { pal[n*3 + 0],
             pal[n*3 + 1],
             pal[n*3 + 2] };
}

unsigned char* wxGIFDecoder::GetData(unsigned int frame) const    { return (m_frames[frame]->p.data()); }
unsigned char* wxGIFDecoder::GetPalette(unsigned int frame) const { return (m_frames[frame]->pal); }
unsigned int wxGIFDecoder::GetNcolours(unsigned int frame) const  { return (m_frames[frame]->ncolours); }
int wxGIFDecoder::GetTransparentColourIndex(unsigned int frame) const  { return (m_frames[frame]->transparent); }



//---------------------------------------------------------------------------
// GIF reading and decoding
//---------------------------------------------------------------------------

// getcode:
//  Reads the next code from the file stream, with size 'bits'
//
int wxGIFDecoder::getcode(wxInputStream& stream, int bits, int ab_fin)
{
    unsigned int mask;          // bit mask
    unsigned int code;          // code (result)

    // get remaining bits from last byte read
    mask = (1 << bits) - 1;
    code = (m_lastbyte >> (8 - m_restbits)) & mask;

    // keep reading new bytes while needed
    while (bits > m_restbits)
    {
        // if no bytes left in this block, read the next block
        if (m_restbyte == 0)
        {
            m_restbyte = stream.GetC();

            /* Some encoders are a bit broken: instead of issuing
             * an end-of-image symbol (ab_fin) they come up with
             * a zero-length subblock!! We catch this here so
             * that the decoder sees an ab_fin code.
             */
            if (m_restbyte == 0)
            {
                code = ab_fin;
                break;
            }

            // prefetch data
            stream.Read((void *) m_buffer, m_restbyte);
            if (stream.LastRead() != m_restbyte)
            {
                code = ab_fin;
                return code;
            }
            m_bufp = m_buffer;
        }

        // read next byte and isolate the bits we need
        m_lastbyte = (unsigned char) (*m_bufp++);
        mask       = (1 << (bits - m_restbits)) - 1;
        code       = code + ((m_lastbyte & mask) << m_restbits);
        m_restbyte--;

        // adjust total number of bits extracted from the buffer
        m_restbits = m_restbits + 8;
    }

    // find number of bits remaining for next code
    m_restbits = (m_restbits - bits);

    return code;
}


// dgif:
//  GIF decoding function. The initial code size (aka root size)
//  is 'bits'. Supports interlaced images (interl == 1).
//  Returns wxGIFErrorCode::OK (== 0) on success, or an error code if something
// fails (see header file for details)
wxGIFErrorCode
wxGIFDecoder::dgif(wxInputStream& stream, GIFImage *img, int interl, int bits)
{
    static constexpr int allocSize = 4096 + 1;

    std::vector<int> ab_prefix(allocSize);  // alphabet (prefixes)

    std::vector<int> ab_tail(allocSize);     // alphabet (tails)

    std::vector<int> decomp_stack(allocSize); // decompression stack

    int ab_clr;                     // clear code
    int ab_fin;                     // end of info code
    int ab_bits;                    // actual symbol width, in bits
    int ab_free;                    // first free position in alphabet
    int ab_max;                     // last possible character in alphabet
    int pass;                       // pass number in interlaced images
    int pos;                        // index into decompresion stack
    unsigned int x, y;              // position in image buffer

    int code, lastcode, abcabca;

    // these won't change
    ab_clr = (1 << bits);
    ab_fin = (1 << bits) + 1;

    // these will change through the decompression process
    ab_bits  = bits + 1;
    ab_free  = (1 << bits) + 2;
    ab_max   = (1 << ab_bits) - 1;
    lastcode = -1;
    abcabca  = -1;
    pass     = 1;
    pos = x = y = 0;

    // reset decoder vars
    m_restbits = 0;
    m_restbyte = 0;
    m_lastbyte = 0;

    do
    {
        // get next code
        int readcode;
        readcode = code = getcode(stream, ab_bits, ab_fin);

        // end of image?
        if (code == ab_fin) break;

        // reset alphabet?
        if (code == ab_clr)
        {
            // reset main variables
            ab_bits  = bits + 1;
            ab_free  = (1 << bits) + 2;
            ab_max   = (1 << ab_bits) - 1;
            lastcode = -1;
            abcabca  = -1;

            // skip to next code
            continue;
        }

        // unknown code: special case (like in ABCABCA)
        if (code >= ab_free)
        {
            code = lastcode;            // take last string
            decomp_stack[pos++] = abcabca;     // add first character
        }

        // build the string for this code in the decomp_stack
        while (code > ab_clr)
        {
            decomp_stack[pos++] = ab_tail[code];
            code         = ab_prefix[code];

            // Don't overflow. This shouldn't happen with normal
            // GIF files, the allocSize of 4096+1 is enough. This
            // will only happen with badly formed GIFs.
            if (pos >= allocSize)
                return wxGIFErrorCode::InvFormat;
        }

        if (pos >= allocSize)
            return wxGIFErrorCode::InvFormat;

        decomp_stack[pos] = code;              // push last code into the decomp_stack
        abcabca    = code;              // save for special case

        // make new entry in alphabet (only if NOT just cleared)
        if (lastcode != -1)
        {
            // Normally, after the alphabet is full and can't grow any
            // further (ab_free == 4096), encoder should (must?) emit CLEAR
            // to reset it. This checks whether we really got it, otherwise
            // the GIF is damaged.
            if (ab_free > ab_max)
                return wxGIFErrorCode::InvFormat;

            // This assert seems unnecessary since the condition above
            // eliminates the only case in which it went false. But I really
            // don't like being forced to ask "Who in .text could have
            // written there?!" And I wouldn't have been forced to ask if
            // this line had already been here.
            wxASSERT(ab_free < allocSize);

            ab_prefix[ab_free] = lastcode;
            ab_tail[ab_free]   = code;
            ab_free++;

            if ((ab_free > ab_max) && (ab_bits < 12))
            {
                ab_bits++;
                ab_max = (1 << ab_bits) - 1;
            }
        }

        // dump decomp_stack data to the image buffer
        while (pos >= 0)
        {
            (img->p)[x + (y * (img->w))] = (char) decomp_stack[pos];
            pos--;

            if (++x >= (img->w))
            {
                x = 0;

                if (interl)
                {
                    // support for interlaced images
                    switch (pass)
                    {
                        case 1: y += 8; break;
                        case 2: y += 8; break;
                        case 3: y += 4; break;
                        case 4: y += 2; break;
                    }

                    /* loop until a valid y coordinate has been
                    found, Or if the maximum number of passes has
                    been reached, exit the loop, and stop image
                    decoding (At this point the image is successfully
                    decoded).
                    If we don't loop, but merely set y to some other
                    value, that new value might still be invalid depending
                    on the height of the image. This would cause out of
                    bounds writing.
                    */
                    while (y >= (img->h))
                    {
                        switch (++pass)
                        {
                            case 2: y = 4; break;
                            case 3: y = 2; break;
                            case 4: y = 1; break;

                            default:
                                /*
                                It's possible we arrive here. For example this
                                happens when the image is interlaced, and the
                                height is 1. Looking at the above cases, the
                                lowest possible y is 1. While the only valid
                                one would be 0 for an image of height 1. So
                                'eventually' the loop will arrive here.
                                This case makes sure this while loop is
                                exited, as well as the 2 other ones.
                                */

                                // Set y to a valid coordinate so the local
                                // while loop will be exited. (y = 0 always
                                // is >= img->h since if img->h == 0 the
                                // image is never decoded)
                                y = 0;

                                // This will exit the other outer while loop
                                pos = -1;

                                // This will halt image decoding.
                                code = ab_fin;

                                break;
                        }
                    }
                }
                else
                {
                    // non-interlaced
                    y++;
/*
Normally image decoding is finished when an End of Information code is
encountered (code == ab_fin) however some broken encoders write wrong
"block byte counts" (The first byte value after the "code size" byte),
being one value too high. It might very well be possible other variants
of this problem occur as well. The only sensible solution seems to
be to check for clipping.
Example of wrong encoding:
(1 * 1 B/W image, raster data stream follows in hex bytes)

02  << B/W images have a code size of 2
02  << Block byte count
44  << LZW packed
00  << Zero byte count (terminates data stream)

Because the block byte count is 2, the zero byte count is used in the
decoding process, and decoding is continued after this byte. (While it
should signal an end of image)

It should be:
02
02
44
01  << When decoded this correctly includes the End of Information code
00

Or (Worse solution):
02
01
44
00
(The 44 doesn't include an End of Information code, but at least the
decoder correctly skips to 00 now after decoding, and signals this
as an End of Information itself)
*/
                    if (y >= img->h)
                    {
                        code = ab_fin;
                        break;
                    }
                }
            }
        }

        pos = 0;
        lastcode = readcode;
    }
    while (code != ab_fin);

    return wxGIFErrorCode::OK;
}


// CanRead:
//  Returns true if the file looks like a valid GIF, false otherwise.
//
bool wxGIFDecoder::DoCanRead(wxInputStream &stream) const
{
    std::array<unsigned char, 3> buf;

    // FIXME: Use span
    if ( !stream.Read(buf.data(), buf.size()) )
        return false;

    // FIXME: Change to better comparison.
    return memcmp(buf.data(), "GIF", buf.size()) == 0;
}


// LoadGIF:
//  Reads and decodes one or more GIF images, depending on whether
//  animated GIF support is enabled. Can read GIFs with any bit
//  size (color depth), but the output images are always expanded
//  to 8 bits per pixel. Also, the image palettes always contain
//  256 colors, although some of them may be unused. Returns wxGIFErrorCode::OK
//  (== 0) on success, or an error code if something fails (see
//  header file for details)
//
wxGIFErrorCode wxGIFDecoder::LoadGIF(wxInputStream& stream)
{
    unsigned int  global_ncolors = 0;
    int           bits, interl, i;
    wxAnimationDisposal disposal;
    std::chrono::milliseconds delay;
    unsigned char type = 0;
    unsigned char pal[768];
    unsigned char buf[16];
    bool anim = true;

    // check GIF signature
    if (!CanRead(stream))
        return wxGIFErrorCode::InvFormat;

    // check for animated GIF support (ver. >= 89a)

    static constexpr unsigned int headerSize = (3 + 3);
    stream.Read(buf, headerSize);
    if (stream.LastRead() != headerSize)
    {
        return wxGIFErrorCode::InvFormat;
    }

    if (memcmp(buf + 3, "89a", 3) < 0)
    {
        anim = false;
    }

    // read logical screen descriptor block (LSDB)
    static constexpr unsigned int lsdbSize = (2 + 2 + 1 + 1 + 1);
    stream.Read(buf, lsdbSize);
    if (stream.LastRead() != lsdbSize)
    {
        return wxGIFErrorCode::InvFormat;
    }

    m_szAnimation.x = buf[0] + 256 * buf[1];
    m_szAnimation.y = buf[2] + 256 * buf[3];

    if (anim && ((m_szAnimation.x == 0) || (m_szAnimation.y == 0)))
    {
        return wxGIFErrorCode::InvFormat;
    }

    // load global color map if available
    if ((buf[4] & 0x80) == 0x80)
    {
        const int backgroundColIndex = buf[5];

        global_ncolors = 2 << (buf[4] & 0x07);
        const unsigned int numBytes = 3 * global_ncolors;
        stream.Read(pal, numBytes);
        if (stream.LastRead() != numBytes)
        {
            return wxGIFErrorCode::InvFormat;
        }

        m_background.Set(pal[backgroundColIndex*3 + 0],
                         pal[backgroundColIndex*3 + 1],
                         pal[backgroundColIndex*3 + 2]);
    }

    // transparent colour, disposal method and delay default to unused
    int transparent = -1;
    disposal = wxANIM_UNSPECIFIED;
    delay = -1ms;
    
    std::string comment;

    bool done = false;
    while (!done)
    {
        type = stream.GetC();

        /*
        If the end of file has been reached (or an error) and a ";"
        (GIF_MARKER_ENDOFDATA) hasn't been encountered yet, exit the loop. (Without this
        check the while loop would loop endlessly.) Later on, in the next while
        loop, the file will be treated as being truncated (But still
        be decoded as far as possible). returning wxGIFErrorCode::Truncated is not
        possible here since some init code is done after this loop.
        */
        if (stream.Eof())// || !stream.IsOk())
        {
            /*
            type is set to some bogus value, so there's no
            need to continue evaluating it.
            */
            break; // Alternative : "return wxGIFErrorCode::InvFormat;"
        }

        switch (type)
        {
            case GIF_MARKER_ENDOFDATA:
                done = true;
                break;
            case GIF_MARKER_EXT:
                switch (stream.GetC())
                {
                    case GIF_MARKER_EXT_GRAPHICS_CONTROL:
                    {
                        // graphics control extension, parse it

                        static constexpr unsigned int gceSize = 6;
                        stream.Read(buf, gceSize);
                        if (stream.LastRead() != gceSize)
                        {
                            m_frames.clear();
                            return wxGIFErrorCode::InvFormat;
                        }

                        // read delay and convert from 1/100 of a second to ms
                        // FIXME: Make this a better conversion; works but it's not techinically correct.
                        delay = 10 * std::chrono::milliseconds((buf[2] + 256 * buf[3]));

                        // read transparent colour index, if used
                        transparent = buf[1] & 0x01 ? buf[4] : -1;

                        // read disposal method
                        disposal = (wxAnimationDisposal)(((buf[1] & 0x1C) >> 2) - 1);
                        break;
                    }
                    case GIF_MARKER_EXT_COMMENT:
                    {
                        int len = stream.GetC();
                        while (len)
                        {
                            if ( stream.Eof() )
                            {
                                done = true;
                                break;
                            }

                            std::vector<char> charbuf(len);

                            stream.Read(charbuf.data(), len);

                            if ( (int) stream.LastRead() != len )
                            {
                                done = true;
                                break;
                            }

                            comment += std::string(charbuf.data(), len);

                            len = stream.GetC();
                        }

                        break;
                    }
                    default:
                        // other extension, skip
                        while ((i = stream.GetC()) != 0)
                        {
                            if (stream.Eof() || (stream.LastRead() == 0) ||
                                stream.SeekI(i, wxSeekMode::FromCurrent) == wxInvalidOffset)
                            {
                                done = true;
                                break;
                            }
                        }
                        break;
                }
                break;
            case GIF_MARKER_SEP:
            {
                // allocate memory for IMAGEN struct
                auto pimg = std::make_unique<GIFImage>();

                if ( !pimg.get() )
                    return wxGIFErrorCode::MemErr;

                // fill in the data
                static constexpr unsigned int idbSize = (2 + 2 + 2 + 2 + 1);
                stream.Read(buf, idbSize);
                if (stream.LastRead() != idbSize)
                    return wxGIFErrorCode::InvFormat;

                pimg->comment = comment;
                comment.clear();
                pimg->left = buf[0] + 256 * buf[1];
                pimg->top = buf[2] + 256 * buf[3];
    /*
                pimg->left = buf[4] + 256 * buf[5];
                pimg->top = buf[4] + 256 * buf[5];
    */
                pimg->w = buf[4] + 256 * buf[5];
                pimg->h = buf[6] + 256 * buf[7];

                if ( anim )
                {
                    // some GIF images specify incorrect animation size but we can
                    // still open them if we fix up the animation size, see #9465
                    if ( m_nFrames == 0 )
                    {
                        if ( pimg->w > (unsigned)m_szAnimation.x )
                            m_szAnimation.x = pimg->w;
                        if ( pimg->h > (unsigned)m_szAnimation.y )
                            m_szAnimation.y = pimg->h;
                    }
                    else // subsequent frames
                    {
                        // check that we have valid size
                        if ( (!pimg->w || pimg->w > (unsigned)m_szAnimation.x) ||
                                (!pimg->h || pimg->h > (unsigned)m_szAnimation.y) )
                        {
                            wxLogError(_("Incorrect GIF frame size (%u, %d) for "
                                         "the frame #%u"),
                                       pimg->w, pimg->h, m_nFrames);
                            return wxGIFErrorCode::InvFormat;
                        }
                    }
                }

                interl = ((buf[8] & 0x40)? 1 : 0);
                auto pimg_size = pimg->w * pimg->h;

                pimg->transparent = transparent;
                pimg->disposal = disposal;
                pimg->delay = delay;

                // allocate memory for image and palette
                pimg->p.resize(pimg_size);
                pimg->pal = (unsigned char *) malloc(768);

                if (pimg->p.empty() || (!pimg->pal))
                    return wxGIFErrorCode::MemErr;

                // load local color map if available, else use global map
                if ((buf[8] & 0x80) == 0x80)
                {
                    const unsigned int local_ncolors = 2 << (buf[8] & 0x07);
                    const unsigned int numBytes = 3 * local_ncolors;
                    stream.Read(pimg->pal, numBytes);
                    pimg->ncolours = local_ncolors;
                    if (stream.LastRead() != numBytes)
                        return wxGIFErrorCode::InvFormat;
                }
                else
                {
                    memcpy(pimg->pal, pal, 768);
                    pimg->ncolours = global_ncolors;
                }

                // get initial code size from first byte in raster data
                bits = stream.GetC();
                if (bits == 0)
                    return wxGIFErrorCode::InvFormat;

                // decode image
                const wxGIFErrorCode result = dgif(stream, pimg.get(), interl, bits);
                if (result != wxGIFErrorCode::OK)
                    return result;

                // add the image to our frame array
                m_frames.push_back(std::move(pimg));
                m_nFrames++;

                // if this is not an animated GIF, exit after first image
                if (!anim)
                    done = true;
                break;
            }
        }
    }

    if (m_nFrames == 0)
    {
        m_frames.clear();
        return wxGIFErrorCode::InvFormat;
    }

    // try to read to the end of the stream
    while (type != GIF_MARKER_ENDOFDATA)
    {
        if (!stream.IsOk())
            return wxGIFErrorCode::Truncated;

        type = stream.GetC();

        switch (type)
        {
            case GIF_MARKER_EXT:
                // extension type
                std::ignore = stream.GetC();

                // skip all data
                while ((i = stream.GetC()) != 0)
                {
                    if (stream.Eof() || (stream.LastRead() == 0) ||
                        stream.SeekI(i, wxSeekMode::FromCurrent) == wxInvalidOffset)
                    {
                        m_frames.clear();
                        return wxGIFErrorCode::InvFormat;
                    }
                }
                break;
            case GIF_MARKER_SEP:
            {
                // image descriptor block
                static constexpr unsigned int idbSize = (2 + 2 + 2 + 2 + 1);
                stream.Read(buf, idbSize);
                if (stream.LastRead() != idbSize)
                {
                    m_frames.clear();
                    return wxGIFErrorCode::InvFormat;
                }

                // local color map
                if ((buf[8] & 0x80) == 0x80)
                {
                    unsigned int local_ncolors = 2 << (buf[8] & 0x07);
                    wxFileOffset numBytes = 3 * local_ncolors;
                    if (stream.SeekI(numBytes, wxSeekMode::FromCurrent) == wxInvalidOffset)
                    {
                        m_frames.clear();
                        return wxGIFErrorCode::InvFormat;
                    }
                }

                // initial code size
                std::ignore = stream.GetC();
                if (stream.Eof() || (stream.LastRead() == 0))
                {
                    m_frames.clear();
                    return wxGIFErrorCode::InvFormat;
                }

                // skip all data
                while ((i = stream.GetC()) != 0)
                {
                    if (stream.Eof() || (stream.LastRead() == 0) ||
                        stream.SeekI(i, wxSeekMode::FromCurrent) == wxInvalidOffset)
                    {
                        m_frames.clear();
                        return wxGIFErrorCode::InvFormat;
                    }
                }
                break;
            }
            default:
                if ((type != GIF_MARKER_ENDOFDATA) && (type != 00)) // testing
                {
                    // images are OK, but couldn't read to the end of the stream
                    return wxGIFErrorCode::Truncated;
                }
                break;
        }
    }

    return wxGIFErrorCode::OK;
}

#endif // wxUSE_STREAMS && wxUSE_GIF
