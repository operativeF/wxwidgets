module;

#include "wx/private/icondir.h"
#include "wx/stream.h"

export module WX.Image.ICO;

import WX.Image.BMP;

#if wxUSE_ICO_CUR

export
{

#ifdef wxUSE_STREAMS

inline bool CanReadICOOrCUR(wxInputStream *stream, std::uint16_t resourceType)
{
    // It's ok to modify the stream position in this function.
    if ( stream->IsSeekable() && stream->SeekI(0) == wxInvalidOffset )
    {
        return false;
    }

    ICONDIR iconDir;
    if ( !stream->ReadAll(&iconDir, sizeof(iconDir)) )
    {
        return false;
    }

    return !iconDir.idReserved // reserved, must be 0
        && wxUINT16_SWAP_ON_BE(iconDir.idType) == resourceType // either 1 or 2
        && iconDir.idCount; // must contain at least one image
}

#endif // wxUSE_STREAMS

// ----------------------------------------------------------------------------
// wxICOHandler
// ----------------------------------------------------------------------------

class wxICOHandler : public wxBMPHandler
{
public:
    wxICOHandler()
    {
        m_name = "Windows icon file";
        m_extension = "ico";
        m_type = wxBitmapType::ICO;
        m_mime = "image/x-ico";
    }

#if wxUSE_STREAMS
    bool SaveFile( wxImage *image, wxOutputStream& stream, bool verbose=true ) override;
    bool LoadFile( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1 ) override;
    virtual bool DoLoadFile( wxImage *image, wxInputStream& stream, bool verbose, int index );

protected:
    int DoGetImageCount( wxInputStream& stream ) override;
    bool DoCanRead( wxInputStream& stream ) override;
#endif // wxUSE_STREAMS

private:
    wxDECLARE_DYNAMIC_CLASS(wxICOHandler);
};

} // export

#endif // wxUSE_ICO_CUR
