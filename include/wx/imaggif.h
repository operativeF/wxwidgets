/////////////////////////////////////////////////////////////////////////////
// Name:        wx/imaggif.h
// Purpose:     wxImage GIF handler
// Author:      Vaclav Slavik, Guillermo Rodriguez Garcia, Gershon Elber, Troels K
// Copyright:   (c) 1999-2011 Vaclav Slavik, Guillermo Rodriguez Garcia, Gershon Elber, Troels K
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_IMAGGIF_H_
#define _WX_IMAGGIF_H_

#include "wx/image.h"

#include <cstdint>


//-----------------------------------------------------------------------------
// wxGIFHandler
//-----------------------------------------------------------------------------

#if wxUSE_GIF

constexpr char wxIMAGE_OPTION_GIF_COMMENT[] = "GifComment";

constexpr char wxIMAGE_OPTION_GIF_TRANSPARENCY[]           = "Transparency";
constexpr char wxIMAGE_OPTION_GIF_TRANSPARENCY_HIGHLIGHT[] = "Highlight";
constexpr char wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED[] = "Unchanged";

struct wxRGB;
struct GifHashTableType;
class WXDLLIMPEXP_FWD_CORE wxImageArray; // anidecod.h

class wxGIFHandler : public wxImageHandler
{
public:
    wxGIFHandler()
    {
        m_name = "GIF file";
        m_extension = "gif";
        m_type = wxBitmapType::GIF;
        m_mime = "image/gif";
        m_hashTable = nullptr;
    }

#if wxUSE_STREAMS
    bool LoadFile(wxImage *image, wxInputStream& stream,
                          bool verbose = true, int index = -1) override;
    bool SaveFile(wxImage *image, wxOutputStream& stream,
                          bool verbose=true) override;

    // Save animated gif
    bool SaveAnimation(const wxImageArray& images, wxOutputStream *stream,
        bool verbose = true, int delayMilliSecs = 1000);

protected:
    int DoGetImageCount(wxInputStream& stream) override;
    bool DoCanRead(wxInputStream& stream) override;

    bool DoSaveFile(const wxImage&, wxOutputStream *, bool verbose,
        bool first, int delayMilliSecs, bool loop,
        const wxRGB *pal, int palCount,
        int mask_index);
#endif // wxUSE_STREAMS
protected:

    // Declarations for saving

    unsigned long m_crntShiftDWord;   /* For bytes decomposition into codes. */
    int m_pixelCount;
    struct GifHashTableType *m_hashTable;
    std::int16_t
      m_EOFCode,     /* The EOF LZ code. */
      m_clearCode,   /* The CLEAR LZ code. */
      m_runningCode, /* The next code algorithm can generate. */
      m_runningBits, /* The number of bits required to represent RunningCode. */
      m_maxCode1,    /* 1 bigger than max. possible code, in RunningBits bits. */
      m_crntCode,    /* Current algorithm code. */
      m_crntShiftState;    /* Number of bits in CrntShiftDWord. */
    std::uint8_t m_LZBuf[256];   /* Compressed input is buffered here. */

    bool InitHashTable();
    void ClearHashTable();
    void InsertHashTable(unsigned long key, int code);
    int  ExistsHashTable(unsigned long key);

#if wxUSE_STREAMS
    bool CompressOutput(wxOutputStream *, int code);
    bool SetupCompress(wxOutputStream *, int bpp);
    bool CompressLine(wxOutputStream *, const std::uint8_t *line, int lineLen);
#endif

private:
    wxDECLARE_DYNAMIC_CLASS(wxGIFHandler);
};

#endif // wxUSE_GIF

#endif // _WX_IMAGGIF_H_

