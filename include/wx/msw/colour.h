/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/colour.h
// Purpose:     wxColour class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLOUR_H_
#define _WX_COLOUR_H_

#include "wx/object.h"

#include <cstdint>

class wxColour : public wxColourBase
{
public:
    DEFINE_STD_WXCOLOUR_CONSTRUCTORS

    bool IsOk() const override { return m_isInit; }

    std::uint8_t Red() const override { return m_red; }
    std::uint8_t Green() const override { return m_green; }
    std::uint8_t Blue() const override { return m_blue; }
    std::uint8_t Alpha() const override { return m_alpha ; }

    bool operator==(const wxColour& colour) const
    {
        return m_isInit == colour.m_isInit
            && m_red == colour.m_red
            && m_green == colour.m_green
            && m_blue == colour.m_blue
            && m_alpha == colour.m_alpha;
    }

    bool operator!=(const wxColour& colour) const { return !(*this == colour); }

    WXCOLORREF GetPixel() const { return m_pixel; }

    WXCOLORREF m_pixel;

protected:
    // FIXME: Protected Init
    void Init();

    void
    InitRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) override;

private:
    bool          m_isInit;
    std::uint8_t  m_red;
    std::uint8_t  m_blue;
    std::uint8_t  m_green;
    std::uint8_t  m_alpha;

    wxDECLARE_DYNAMIC_CLASS(wxColour);
};

#endif // _WX_COLOUR_H_
