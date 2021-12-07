/////////////////////////////////////////////////////////////////////////////
// Name:        wx/anidecod.h
// Purpose:     wxANIDecoder, ANI reader for wxImage and wxAnimation
// Author:      Francesco Montorsi
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/animdecod.h"

#include <chrono>

export module WX.Image.Decoder.ANI;

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

class wxInputStream;

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

#endif  // wxUSE_STREAMS && (wxUSE_ICO_CUR || wxUSE_GIF)
