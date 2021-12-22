/////////////////////////////////////////////////////////////////////////////
// Name:        wx/anidecod.h
// Purpose:     wxANIDecoder, ANI reader for wxImage and wxAnimation
// Author:      Francesco Montorsi
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/animdecod.h"
#include "wx/gdicmn.h"

#include <chrono>

export module WX.Image.Decoder.ANI;

import WX.Cmn.Stream;

import Utils.Geometry;

import WX.Image.Base;
import WX.Image.BMP;
import WX.Image.CUR;

#if wxUSE_STREAMS && (wxUSE_ICO_CUR || wxUSE_GIF)

//---------------------------------------------------------------------------
// wxANIFrameInfo
//---------------------------------------------------------------------------

using namespace std::chrono_literals;

class wxANIFrameInfo
{
public:
    explicit wxANIFrameInfo(std::chrono::milliseconds delay = 0ms, int idx = -1)
        : m_delay(delay),
          m_imageIndex(idx)
    {}

    std::chrono::milliseconds m_delay;
    int m_imageIndex;
};

export
{

using wxANIFrameInfoArray = std::vector<wxANIFrameInfo>;

// --------------------------------------------------------------------------
// wxANIDecoder class
// --------------------------------------------------------------------------

class wxANIDecoder : public wxAnimationDecoder
{
public:

   wxANIDecoder& operator=(wxANIDecoder&&) = delete;

    wxSize GetFrameSize(unsigned int frame) const override;
    wxPoint GetFramePosition(unsigned int frame) const override;
    wxAnimationDisposal GetDisposalMethod(unsigned int frame) const override;
    std::chrono::milliseconds GetDelay(unsigned int frame) const override;
    wxColour GetTransparentColour(unsigned int frame) const override;

    // implementation of wxAnimationDecoder's pure virtuals

    bool Load( wxInputStream& stream ) override;

    bool ConvertToImage(unsigned int frame, wxImage *image) const override;

    wxAnimationDecoder *Clone() const override
        { return new wxANIDecoder; }
    wxAnimationType GetType() const override
        { return wxANIMATION_TYPE_ANI; }

protected:
    // wxAnimationDecoder pure virtual:
    bool DoCanRead( wxInputStream& stream ) const override;
            // modifies current stream position (see wxAnimationDecoder::CanRead)

private:
    // frames stored as wxImage(s): ANI files are meant to be used mostly for animated
    // cursors and thus they do not use any optimization to encode differences between
    // two frames: they are just a list of images to display sequentially.
    std::vector<wxImage> m_images;

    // the info about each image stored in m_images.
    // NB: m_info.GetCount() may differ from m_images.GetCount()!
    wxANIFrameInfoArray m_info;

    // this is the wxCURHandler used to load the ICON chunk of the ANI files
    inline static wxCURHandler sm_handler;
};

} // export

//---------------------------------------------------------------------------
// wxANIDecoder
//---------------------------------------------------------------------------

bool wxANIDecoder::ConvertToImage(unsigned int frame, wxImage *image) const
{
    unsigned int idx = m_info[frame].m_imageIndex;
    *image = m_images[idx];       // copy
    return image->IsOk();
}


//---------------------------------------------------------------------------
// Data accessors
//---------------------------------------------------------------------------

wxSize wxANIDecoder::GetFrameSize([[maybe_unused]] unsigned int frame) const
{
    // all frames are of the same size...
    return m_szAnimation;
}

wxPoint wxANIDecoder::GetFramePosition([[maybe_unused]] unsigned int frame) const
{
    // all frames are of the same size...
    return {0, 0};
}

wxAnimationDisposal wxANIDecoder::GetDisposalMethod([[maybe_unused]] unsigned int frame) const
{
    // this disposal is implicit for all frames inside an ANI file
    return wxANIM_TOBACKGROUND;
}

// FIXME: Unsafe index into array
std::chrono::milliseconds wxANIDecoder::GetDelay(unsigned int frame) const
{
    return m_info[frame].m_delay;
}

// FIXME: Unsage index into array
wxColour wxANIDecoder::GetTransparentColour(unsigned int frame) const
{
    unsigned int idx = m_info[frame].m_imageIndex;

    if (!m_images[idx].HasMask())
        return wxNullColour;

    return {m_images[idx].GetMaskRed(),
            m_images[idx].GetMaskGreen(),
            m_images[idx].GetMaskBlue()};
}


//---------------------------------------------------------------------------
// ANI reading and decoding
//---------------------------------------------------------------------------

bool wxANIDecoder::DoCanRead(wxInputStream& stream) const
{
    std::int32_t FCC1, FCC2;
    std::uint32_t datalen;

    std::int32_t riff32;
    memcpy( &riff32, "RIFF", 4 );
    std::int32_t list32;
    memcpy( &list32, "LIST", 4 );
    std::int32_t ico32;
    memcpy( &ico32, "icon", 4 );
    std::int32_t anih32;
    memcpy( &anih32, "anih", 4 );

    if ( stream.IsSeekable() && stream.SeekI(0) == wxInvalidOffset )
    {
        return false;
    }

    if ( !stream.Read(&FCC1, 4) )
        return false;

    if ( FCC1 != riff32 )
        return false;

    // we have a riff file:
    while ( stream.IsOk() )
    {
        if ( FCC1 == anih32 )
            return true;        // found the ANIH chunk - this should be an ANI file

        // we always have a data size:
        stream.Read(&datalen, 4);
        datalen = wxINT32_SWAP_ON_BE(datalen) ;

        // data should be padded to make even number of bytes
        if (datalen % 2 == 1) datalen ++ ;

        // now either data or a FCC:
        if ( (FCC1 == riff32) || (FCC1 == list32) )
        {
            stream.Read(&FCC2, 4);
        }
        else
        {
            if ( stream.SeekI(stream.TellI() + datalen) == wxInvalidOffset )
                return false;
        }

        // try to read next data chunk:
        if ( !stream.Read(&FCC1, 4) )
        {
            // reading failed -- either EOF or IO error, bail out anyhow
            return false;
        }
    }

    return false;
}

// the "anih" RIFF chunk
struct wxANIHeader
{
    std::int32_t cbSizeOf;     // Num bytes in AniHeader (36 bytes)
    std::int32_t cFrames;      // Number of unique Icons in this cursor
    std::int32_t cSteps;       // Number of Blits before the animation cycles
    std::int32_t cx;           // width of the frames
    std::int32_t cy;           // height of the frames
    std::int32_t cBitCount;    // bit depth
    std::int32_t cPlanes;      // 1
    std::int32_t JifRate;      // Default Jiffies (1/60th of a second) if rate chunk not present.
    std::int32_t flags;        // Animation Flag (see AF_ constants)

    // ANI files are always little endian so we need to swap bytes on big
    // endian architectures
#ifdef WORDS_BIGENDIAN
    void AdjustEndianness()
    {
        // this works because all our fields are std::int32_t and they must be
        // packed without holes between them (if they're not, they wouldn't map
        // to the file header!)
        std::int32_t * const start = (std::int32_t *)this;
        std::int32_t * const end = start + sizeof(wxANIHeader)/sizeof(std::int32_t);
        for ( std::int32_t *p = start; p != end; p++ )
        {
            *p = wxINT32_SWAP_ALWAYS(*p);
        }
    }
#else
    void AdjustEndianness() { }
#endif
};

bool wxANIDecoder::Load( wxInputStream& stream )
{
    std::int32_t FCC1, FCC2;
    std::uint32_t datalen;
    std::chrono::milliseconds globaldelay{0ms};

    std::int32_t riff32;
    memcpy( &riff32, "RIFF", 4 );
    std::int32_t list32;
    memcpy( &list32, "LIST", 4 );
    std::int32_t ico32;
    memcpy( &ico32, "icon", 4 );
    std::int32_t anih32;
    memcpy( &anih32, "anih", 4 );
    std::int32_t rate32;
    memcpy( &rate32, "rate", 4 );
    std::int32_t seq32;
    memcpy( &seq32, "seq ", 4 );

    if ( stream.IsSeekable() && stream.SeekI(0) == wxInvalidOffset )
    {
        return false;
    }

    if ( !stream.Read(&FCC1, 4) )
        return false;
    if ( FCC1 != riff32 )
        return false;

    m_nFrames = 0;
    m_szAnimation = wxDefaultSize;

    m_images.clear();
    m_info.clear();

    // we have a riff file:
    while ( !stream.Eof() )
    {
        // we always have a data size:
        if (!stream.Read(&datalen, 4))
            return false;

        datalen = wxINT32_SWAP_ON_BE(datalen);

        //data should be padded to make even number of bytes
        if (datalen % 2 == 1) datalen++;

        // now either data or a FCC:
        if ( (FCC1 == riff32) || (FCC1 == list32) )
        {
            if (!stream.Read(&FCC2, 4))
                return false;
        }
        else if ( FCC1 == anih32 )
        {
            if ( datalen != sizeof(wxANIHeader) )
                return false;

            if (m_nFrames > 0)
                return false;       // already parsed an ani header?

            struct wxANIHeader header;
            if (!stream.Read(&header, sizeof(wxANIHeader)))
                return false;
            header.AdjustEndianness();

            // we should have a global frame size
            m_szAnimation = wxSize(header.cx, header.cy);

            // save interesting info from the header
            m_nFrames = header.cSteps;   // NB: not cFrames!!
            if ( m_nFrames == 0 )
                return false;

            // FIXME: Use better conversion. Not technically correct.
            globaldelay = std::chrono::milliseconds{header.JifRate * 1000 / 60};

            m_images.reserve(header.cFrames);
            m_info.resize(m_nFrames);
        }
        else if ( FCC1 == rate32 )
        {
            // did we already process the anih32 chunk?
            if (m_nFrames == 0)
                return false;       // rate chunks should always be placed after anih chunk

            wxASSERT(m_info.size() == m_nFrames);
            for (unsigned int i=0; i<m_nFrames; i++)
            {
                if (!stream.Read(&FCC2, 4))
                    return false;
                // FIXME: Use better conversion. Not technically correct.
                m_info[i].m_delay = std::chrono::milliseconds{wxINT32_SWAP_ON_BE(FCC2) * 1000 / 60};
            }
        }
        else if ( FCC1 == seq32 )
        {
            // did we already process the anih32 chunk?
            if (m_nFrames == 0)
                return false;       // seq chunks should always be placed after anih chunk

            wxASSERT(m_info.size() == m_nFrames);
            for (unsigned int i=0; i<m_nFrames; i++)
            {
                if (!stream.Read(&FCC2, 4))
                    return false;
                m_info[i].m_imageIndex = wxINT32_SWAP_ON_BE(FCC2);
            }
        }
        else if ( FCC1 == ico32 )
        {
            // use DoLoadFile() and not LoadFile()!
            wxImage image;
            if (!sm_handler.DoLoadFile(&image, stream, false /* verbose */, -1))
                return false;

            image.SetType(wxBitmapType::ANI);
            m_images.push_back(image);
        }
        else
        {
            if ( stream.SeekI(stream.TellI() + datalen) == wxInvalidOffset )
                return false;
        }

        // try to read next data chunk:
        if ( !stream.Read(&FCC1, 4) && !stream.Eof())
        {
            // we didn't reach the EOF! An other kind of error has occurred...
            return false;
        }
        //else: proceed with the parsing of the next header block or
        //      exiting this loop (if stream.Eof() == true)
    }

    if (m_nFrames==0)
        return false;

    if (m_nFrames==m_images.size())
    {
        // if no SEQ chunk is available, display the frames in the order
        // they were loaded
        for (unsigned int i=0; i<m_nFrames; i++)
            if (m_info[i].m_imageIndex == -1)
                m_info[i].m_imageIndex = i;
    }

    // if some frame has an invalid delay, use the global delay given in the
    // ANI header
    for (unsigned int i=0; i<m_nFrames; i++)
        if (m_info[i].m_delay == 0ms)
            m_info[i].m_delay = globaldelay;

    // if the header did not contain a valid frame size, try to grab
    // it from the size of the first frame (all frames are of the same size)
    if (m_szAnimation.x == 0 ||
        m_szAnimation.y == 0)
        m_szAnimation = m_images[0].GetSize();

    return m_szAnimation != wxDefaultSize;
}

#endif  // wxUSE_STREAMS && (wxUSE_ICO_CUR || wxUSE_GIF)
