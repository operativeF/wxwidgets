// Original by:
/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/imaggif.cpp
// Purpose:     wxImage GIF handler
// Author:      Vaclav Slavik, Guillermo Rodriguez Garcia, Gershon Elber, Troels K
// Copyright:   (c) 1999-2011 Vaclav Slavik, Guillermo Rodriguez Garcia, Gershon Elber, Troels K
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/defs.h"
#include "wx/log.h"
#include "wx/translation.h"

#ifdef wxUSE_PALETTE
#include "wx/palette.h"
#endif

module WX.Image.GIF;

import WX.Image.Decoder.GIF;

//-----------------------------------------------------------------------------
// wxGIFHandler
//-----------------------------------------------------------------------------

#if wxUSE_GIF

constexpr char GIF89_HDR[] = "GIF89a";
constexpr char NETSCAPE_LOOP[] = "NETSCAPE2.0";

// see members.aol.com/royalef/gifabout.htm
//     members.aol.com/royalef/gif89a.txt

constexpr auto LZ_MAX_CODE         = 4095;    // Biggest code possible in 12 bits.
constexpr auto FLUSH_OUTPUT        = 4096;    // Impossible code, to signal flush.
constexpr auto FIRST_CODE          = 4097;    // Impossible code, to signal first.

constexpr auto HT_SIZE         = 8192;        // 12bits = 4096 or twice as big!
constexpr auto HT_KEY_MASK     = 0x1FFF;      // 13bits keys

#define HT_GET_KEY(l)   (l >> 12)
#define HT_GET_CODE(l)  (l & 0x0FFF)
#define HT_PUT_KEY(l)   (l << 12)
#define HT_PUT_CODE(l)  (l & 0x0FFF)

struct wxRGB
{
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
};

struct GifHashTableType
{
    std::uint32_t HTable[HT_SIZE];
};

// ---------------------------------------------------------------------------
// implementation of global private functions
// ---------------------------------------------------------------------------

namespace
{

constexpr int wxGIFHandler_KeyItem(unsigned long item)
{
    return ((item >> 12) ^ item) & HT_KEY_MASK;
}

#if wxUSE_STREAMS

constexpr int wxGIFHandler_BitSize(int n)
{
    int i;
    for (i = 1; i <= 8; i++)
    {
        if ((1 << i) >= n)
        {
            break;
        }
    }
    return i;
}

int wxGIFHandler_PaletteFind(const wxRGB& clr, const wxRGB* array, int count)
{
    for (int i = 0; i < count; i++)
    {
        if ((clr.red == array[i].red)
            && (clr.green == array[i].green)
            && (clr.blue == array[i].blue))
        {
            return i;
        }
    }

    return wxNOT_FOUND;
}

#if wxUSE_PALETTE
bool wxGIFHandler_GetPalette(const wxImage& image,
    wxRGB* pal, int* pPalCount, int* pMaskIndex)
{
    if (!image.HasPalette())
    {
        return false;
    }

    const wxPalette& palette = image.GetPalette();
    int palCount = palette.GetColoursCount();

    for (int i = 0; i < palCount; ++i)
    {
        if (!palette.GetRGB(i, &pal[i].red, &pal[i].green, &pal[i].blue))
        {
            break;
        }
    }
    if (image.HasMask())
    {
        wxRGB mask;

        mask.red = image.GetMaskRed();
        mask.green = image.GetMaskGreen();
        mask.blue = image.GetMaskBlue();
        *pMaskIndex = wxGIFHandler_PaletteFind(mask, pal, palCount);
        if ((*pMaskIndex == wxNOT_FOUND) && (palCount < 256))
        {
            *pMaskIndex = palCount;
            pal[palCount++] = mask;
        }
    }
    else
    {
        *pMaskIndex = wxNOT_FOUND;
    }
    *pPalCount = palCount;

    return true;
}
#endif // wxUSE_PALETTE

bool wxGIFHandler_Write(wxOutputStream* stream, const void* buf, size_t len)
{
    return (len == stream->Write(buf, len).LastWrite());
}

bool wxGIFHandler_WriteByte(wxOutputStream* stream, std::uint8_t byte)
{
    return wxGIFHandler_Write(stream, &byte, sizeof(byte));
}

bool wxGIFHandler_WriteWord(wxOutputStream* stream, std::uint16_t word)
{
    std::uint8_t buf[2];

    buf[0] = word & 0xff;
    buf[1] = (word >> 8) & 0xff;
    return wxGIFHandler_Write(stream, &buf, sizeof(buf));
}

bool wxGIFHandler_WritePalette(wxOutputStream* stream,
    const wxRGB* array, size_t count, int bpp)
{
    std::uint8_t buf[3];
    for (int i = 0; (i < (1 << bpp)); i++)
    {
        if (i < (int)count)
        {
            buf[0] = array[i].red;
            buf[1] = array[i].green;
            buf[2] = array[i].blue;
        }
        else
        {
            buf[0] = buf[1] = buf[2] = 0;
        }

        if (!wxGIFHandler_Write(stream, buf, sizeof(buf)))
        {
            return false;
        }
    }

    return true;
}

bool wxGIFHandler_WriteZero(wxOutputStream* stream)
{
    return wxGIFHandler_WriteByte(stream, 0);
}

bool wxGIFHandler_WriteLoop(wxOutputStream* stream)
{
    std::uint8_t buf[4];
    static constexpr int loopcount = 0; // infinite

    buf[0] = GIF_MARKER_EXT;
    buf[1] = GIF_MARKER_EXT_APP;
    buf[2] = 0x0B;
    bool ok = wxGIFHandler_Write(stream, buf, 3)
        && wxGIFHandler_Write(stream, NETSCAPE_LOOP, sizeof(NETSCAPE_LOOP) - 1);

    buf[0] = 3;
    buf[1] = 1;
    buf[2] = loopcount & 0xFF;
    buf[3] = loopcount >> 8;

    return ok && wxGIFHandler_Write(stream, buf, 4)
        && wxGIFHandler_WriteZero(stream);
}


bool wxGIFHandler_WriteHeader(wxOutputStream* stream, int width, int height,
    bool loop, const wxRGB* pal, int palCount)
{
    const int bpp = wxGIFHandler_BitSize(palCount);
    std::uint8_t buf[3];

    bool ok = wxGIFHandler_Write(stream, GIF89_HDR, sizeof(GIF89_HDR) - 1)
        && wxGIFHandler_WriteWord(stream, (std::uint16_t)width)
        && wxGIFHandler_WriteWord(stream, (std::uint16_t)height);

    buf[0] = 0x80;
    buf[0] |= (bpp - 1) << 5;
    buf[0] |= (bpp - 1);
    buf[1] = 0; // background color == entry 0
    buf[2] = 0; // aspect ratio 1:1
    ok = ok && wxGIFHandler_Write(stream, buf, sizeof(buf))
        && wxGIFHandler_WritePalette(stream, pal, palCount, bpp);

    if (loop)
    {
        ok = ok && wxGIFHandler_WriteLoop(stream);
    }

    return ok;
}

bool wxGIFHandler_WriteRect(wxOutputStream* stream, int width, int height)
{
    return wxGIFHandler_WriteWord(stream, 0) // left
        && wxGIFHandler_WriteWord(stream, 0) // top
        && wxGIFHandler_WriteWord(stream, (std::uint16_t)width)
        && wxGIFHandler_WriteWord(stream, (std::uint16_t)height);
}

#if wxUSE_PALETTE
bool wxGIFHandler_WriteTerm(wxOutputStream* stream)
{
    return wxGIFHandler_WriteByte(stream, GIF_MARKER_ENDOFDATA);
}
#endif

bool wxGIFHandler_WriteControl(wxOutputStream* stream,
    int maskIndex, int delayMilliSecs)
{
    std::uint8_t buf[8];
    const std::uint16_t delay = delayMilliSecs / 10;

    buf[0] = GIF_MARKER_EXT;    // extension marker
    buf[1] = GIF_MARKER_EXT_GRAPHICS_CONTROL;
    buf[2] = 4;     // length of block
    buf[3] = (maskIndex != wxNOT_FOUND) ? 1 : 0;   // has transparency
    buf[4] = delay & 0xff;  // delay time
    buf[5] = (delay >> 8) & 0xff;   // delay time second byte
    buf[6] = (maskIndex != wxNOT_FOUND) ? (std::uint8_t)maskIndex : 0;
    buf[7] = 0;
    return wxGIFHandler_Write(stream, buf, sizeof(buf));
}

bool wxGIFHandler_WriteComment(wxOutputStream* stream, const std::string& comment)
{
    if (comment.empty())
    {
        return true;
    }

    // Write comment header.
    std::uint8_t buf[2];
    buf[0] = GIF_MARKER_EXT;
    buf[1] = GIF_MARKER_EXT_COMMENT;
    if (!wxGIFHandler_Write(stream, buf, sizeof(buf)))
    {
        return false;
    }

    /*
    If comment is longer than 255 bytes write it in blocks of maximum 255
    bytes each.
    */
    size_t pos = 0;
    size_t fullLength = comment.length();

    do
    {
        size_t blockLength = std::min(fullLength - pos, std::size_t{ 255 });

        if (!wxGIFHandler_WriteByte(stream, (std::uint8_t)blockLength)
            || !wxGIFHandler_Write(stream, &comment.data()[pos], blockLength))
        {
            return false;
        }

        pos += blockLength;
    } while (pos < fullLength);


    // Write comment footer.
    return wxGIFHandler_WriteZero(stream);
}

bool wxGIFHandler_BufferedOutput(wxOutputStream* stream, std::uint8_t* buf, int c)
{
    bool ok = true;

    if (c == FLUSH_OUTPUT)
    {
        // Flush everything out.
        if (buf[0])
        {
            ok = wxGIFHandler_Write(stream, buf, buf[0] + 1);
        }
        // Mark end of compressed data, by an empty block (see GIF doc):
        wxGIFHandler_WriteZero(stream);
    }
    else
    {
        if (buf[0] == 255)
        {
            // Dump out this buffer - it is full:
            ok = wxGIFHandler_Write(stream, buf, buf[0] + 1);
            buf[0] = 0;
        }
        buf[++buf[0]] = c;
    }

    return ok;
}

#endif // wxUSE_STREAMS

} // namespace anonymous


//-----------------------------------------------------------------------------
// wxGIFHandler
//-----------------------------------------------------------------------------

#if wxUSE_STREAMS

bool wxGIFHandler::LoadFile(wxImage *image, wxInputStream& stream,
    bool verbose, int index)
{
    wxGIFDecoder decod;
    switch ( decod.LoadGIF(stream) )
    {
        case wxGIFErrorCode::OK:
            break;

        case wxGIFErrorCode::InvFormat:
            if ( verbose )
                wxLogError(_("GIF: error in GIF image format."));
            return false;

        case wxGIFErrorCode::MemErr:
            if ( verbose )
                wxLogError(_("GIF: not enough memory."));
            return false;

        case wxGIFErrorCode::Truncated:
            if ( verbose )
                wxLogError(_("GIF: data stream seems to be truncated."));

            // go on; image data is OK
            break;
    }

    return decod.ConvertToImage(index != -1 ? (size_t)index : 0, image);
}

bool wxGIFHandler::SaveFile(wxImage *image,
    wxOutputStream& stream, bool verbose)
{
#if wxUSE_PALETTE
    wxRGB pal[256];
    int palCount;
    int maskIndex;

    return wxGIFHandler_GetPalette(*image, pal, &palCount, &maskIndex)
        && DoSaveFile(*image, &stream, verbose, true /*first?*/, 0,
            false /*loop?*/, pal, palCount, maskIndex)
        && wxGIFHandler_WriteTerm(&stream);
#else
    wxUnusedVar(image);
    wxUnusedVar(stream);
    wxUnusedVar(verbose);
    return false;
#endif
}

bool wxGIFHandler::DoCanRead( wxInputStream& stream )
{
    wxGIFDecoder decod;
    return decod.CanRead(stream);
         // it's ok to modify the stream position here
}

int wxGIFHandler::DoGetImageCount( wxInputStream& stream )
{
    wxGIFDecoder decod;
    wxGIFErrorCode error = decod.LoadGIF(stream);
    if ( (error != wxGIFErrorCode::OK) && (error != wxGIFErrorCode::Truncated) )
        return -1;

    // NOTE: this function modifies the current stream position but it's ok
    //       (see wxImageHandler::GetImageCount)

    return decod.GetFrameCount();
}

bool wxGIFHandler::DoSaveFile(const wxImage& image, wxOutputStream *stream,
    [[maybe_unused]] bool verbose, bool first, int delayMilliSecs, bool loop,
    const wxRGB *pal, int palCount, int maskIndex)
{
    const unsigned long colorcount = image.CountColours(256+1);
    bool ok = colorcount && (colorcount <= 256);
    if (!ok)
    {
        return false;
    }

    int width = image.GetWidth();
    int height = image.GetHeight();
    wxCHECK_MSG( width && height, false, "can't save 0-sized file" );

    int width_even = width + ((width % 2) ? 1 : 0);

    if (first)
    {
        ok = wxGIFHandler_WriteHeader(stream, width, height, loop,
            pal, palCount);
    }

    ok = ok
        && wxGIFHandler_WriteComment(stream,
            image.GetOption(wxIMAGE_OPTION_GIF_COMMENT))
        && wxGIFHandler_WriteControl(stream, maskIndex, delayMilliSecs)
        && wxGIFHandler_WriteByte(stream, GIF_MARKER_SEP)
        && wxGIFHandler_WriteRect(stream, width, height);

    // local palette
    if (first)
    {
        // we already saved the (global) palette
        ok = ok && wxGIFHandler_WriteZero(stream);
    }
    else
    {
        const int bpp = wxGIFHandler_BitSize(palCount);
        std::uint8_t b;

        b = 0x80;
        b |=(bpp - 1) << 5;
        b |=(bpp - 1);
        b &=~0x40; // clear interlaced

        ok = ok && wxGIFHandler_WriteByte(stream, b)
            && wxGIFHandler_WritePalette(stream, pal, palCount, bpp);
    }

    if (!ok)
    {
        return false;
    }

    if (!InitHashTable())
    {
        wxLogError(_("Couldn't initialize GIF hash table."));
        return false;
    }

    const std::uint8_t *src = image.GetData();
    auto eightBitData = std::make_unique<std::uint8_t[]>(width);

    SetupCompress(stream, 8);

    m_pixelCount = height * width_even;
    for (int y = 0; y < height; y++)
    {
        m_pixelCount -= width_even;
        for (int x = 0; x < width; x++)
        {
            wxRGB rgb;
            rgb.red   = src[0];
            rgb.green = src[1];
            rgb.blue  = src[2];
            int index = wxGIFHandler_PaletteFind(rgb, pal, palCount);
            wxASSERT(index != wxNOT_FOUND);
            eightBitData[x] = (std::uint8_t)index;
            src+=3;
        }

        ok = CompressLine(stream, eightBitData.get(), width);
        if (!ok)
        {
            break;
        }
    }

    wxDELETE(m_hashTable);

    return ok;
}

bool wxGIFHandler::SaveAnimation(const wxImageArray& images,
    wxOutputStream *stream, bool verbose, int delayMilliSecs)
{
#if wxUSE_PALETTE
    bool ok = true;
    size_t i;

    wxSize size(0,0);
    for (i = 0; (i < images.size()) && ok; i++)
    {
        const wxImage& image = images[i];
        wxSize temp(image.GetWidth(), image.GetHeight());
        ok = ok && image.HasPalette();
        if (i)
        {
           ok = ok && (size == temp);
        }
        else
        {
           size = temp;
        }
    }

    for (i = 0; (i < images.size()) && ok; i++)
    {
        const wxImage& image = images[i];

        wxRGB pal[256];
        int palCount;
        int maskIndex;

        ok = wxGIFHandler_GetPalette(image, pal, &palCount, &maskIndex)
          && DoSaveFile(image, stream, verbose, i == 0 /*first?*/, delayMilliSecs,
            true /*loop?*/, pal, palCount, maskIndex);
    }

    return ok && wxGIFHandler_WriteTerm(stream);
#else
    wxUnusedVar(images);
    wxUnusedVar(stream);
    wxUnusedVar(verbose);
    wxUnusedVar(delayMilliSecs);

    return false;
#endif
}

bool wxGIFHandler::CompressOutput(wxOutputStream *stream, int code)
{
    if (code == FLUSH_OUTPUT)
    {
        while (m_crntShiftState > 0)
        {
            // Get rid of what is left in DWord, and flush it.
            if (!wxGIFHandler_BufferedOutput(stream, m_LZBuf,
                m_crntShiftDWord & 0xff))
            {
                return false;
            }
            m_crntShiftDWord >>= 8;
            m_crntShiftState -= 8;
        }
        m_crntShiftState = 0;                       // For next time.
        if (!wxGIFHandler_BufferedOutput(stream, m_LZBuf, FLUSH_OUTPUT))
        {
            return false;
        }
    }
    else
    {
        m_crntShiftDWord |= ((long) code) << m_crntShiftState;
        m_crntShiftState += m_runningBits;
        while (m_crntShiftState >= 8)
        {
            // Dump out full bytes:
            if (!wxGIFHandler_BufferedOutput(stream, m_LZBuf,
                m_crntShiftDWord & 0xff))
            {
                return false;
            }
            m_crntShiftDWord >>= 8;
            m_crntShiftState -= 8;
        }
    }

    // If code can't fit into RunningBits bits, must raise its size. Note
    // however that codes above LZ_MAX_CODE are used for special signaling.
    if ( (m_runningCode >= m_maxCode1) && (code <= LZ_MAX_CODE))
    {
        m_maxCode1 = 1 << ++m_runningBits;
    }
    return true;
}

bool wxGIFHandler::SetupCompress(wxOutputStream *stream, int bpp)
{
    m_LZBuf[0] = 0;           // Nothing was output yet.
    m_clearCode = (1 << bpp);
    m_EOFCode = m_clearCode + 1;
    m_runningCode = m_EOFCode + 1;
    m_runningBits = bpp + 1;     // Number of bits per code.
    m_maxCode1 = 1 << m_runningBits;       // Max. code + 1.
    m_crntCode = FIRST_CODE;       // Signal that this is first one!
    m_crntShiftState = 0;      // No information in CrntShiftDWord.
    m_crntShiftDWord = 0;

    // Clear hash table and send Clear to make sure the decoder does the same.
    ClearHashTable();

    return wxGIFHandler_WriteByte(stream, (std::uint8_t)bpp)
        && CompressOutput(stream, m_clearCode);
}

bool wxGIFHandler::CompressLine(wxOutputStream *stream,
    const std::uint8_t *line, int lineLen)
{
    int i = 0, crntCode;
    if (m_crntCode == FIRST_CODE)                  // It's first time!
        crntCode = line[i++];
    else
        crntCode = m_crntCode;     // Get last code in compression.

    while (i < lineLen)
    {
        // Decode lineLen items.
        std::uint8_t pixel;
        pixel = line[i++];                    // Get next pixel from stream.
        // Form a new unique key to search hash table for the code combines
        // crntCode as Prefix string with Pixel as postfix char.
        unsigned long newKey;
        newKey = (((unsigned long) crntCode) << 8) + pixel;
        int newCode;
        if ((newCode = ExistsHashTable(newKey)) >= 0)
        {
            // This Key is already there, or the string is old one, so
            // simply take new code as our crntCode:
            crntCode = newCode;
        }
        else
        {
            // Put it in hash table, output the prefix code, and make our
            // crntCode equal to Pixel.
            if (!CompressOutput(stream, crntCode))
            {
                return false;
            }

            crntCode = pixel;

            // If however the HashTable is full, we send a clear first and
            // Clear the hash table.
            if (m_runningCode >= LZ_MAX_CODE)
            {
                // Time to do some clearance:
                if (!CompressOutput(stream, m_clearCode))
                {
                    return false;
                }

                m_runningCode = m_EOFCode + 1;
                m_runningBits = 8 + 1;
                m_maxCode1 = 1 << m_runningBits;
                ClearHashTable();
            }
            else
            {
                // Put this unique key with its relative Code in hash table:
                InsertHashTable(newKey, m_runningCode++);
            }
        }
    }
    // Preserve the current state of the compression algorithm:
    m_crntCode = crntCode;
    if (m_pixelCount == 0)
    {
        // We are done - output last Code and flush output buffers:
        if (!CompressOutput(stream, crntCode)
            || !CompressOutput(stream, m_EOFCode)
            || !CompressOutput(stream, FLUSH_OUTPUT))
        {
            return false;
        }
    }

    return true;
}

#endif  // wxUSE_STREAMS

bool wxGIFHandler::InitHashTable()
{
    if (!m_hashTable)
    {
        m_hashTable = new GifHashTableType();
    }

    if (!m_hashTable)
    {
        return false;
    }

    ClearHashTable();

    return true;
}

void wxGIFHandler::ClearHashTable()
{
    int index = HT_SIZE;
    std::uint32_t *HTable = m_hashTable->HTable;

    while (--index>=0)
    {
        HTable[index] = 0xfffffffful;
    }
}

void wxGIFHandler::InsertHashTable(unsigned long key, int code)
{
    int hKey = wxGIFHandler_KeyItem(key);
    std::uint32_t *HTable = m_hashTable->HTable;

    while (HT_GET_KEY(HTable[hKey]) != 0xFFFFFL)
    {
        hKey = (hKey + 1) & HT_KEY_MASK;
    }
    HTable[hKey] = HT_PUT_KEY(key) | HT_PUT_CODE(code);
}


int wxGIFHandler::ExistsHashTable(unsigned long key)
{
    int hKey = wxGIFHandler_KeyItem(key);
    std::uint32_t *HTable = m_hashTable->HTable, HTKey;

    while ((HTKey = HT_GET_KEY(HTable[hKey])) != 0xFFFFFL)
    {
        if (key == HTKey)
        {
            return HT_GET_CODE(HTable[hKey]);
        }
        hKey = (hKey + 1) & HT_KEY_MASK;
    }
    return -1;
}

#endif // wxUSE_GIF
