/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/palette.cpp
// Purpose:     wxPalette
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_PALETTE

#include "wx/msw/private.h"

#include "wx/palette.h"

import WX.Win.UniqueHnd;

// ============================================================================
// wxPaletteRefData
// ============================================================================

using namespace msw::utils;

class wxPaletteRefData: public wxGDIRefData
{
public:
    wxPaletteRefData() = default;

    // FIXME: Negative n poses hazard.
    wxPaletteRefData(int n,
                     const unsigned char *red,
                     const unsigned char *green,
                     const unsigned char *blue)
    {
        LOGPALETTE *pPal = Alloc(n);
        if ( !pPal )
            return;

        for ( int i = 0; i < n; i++ )
        {
            pPal->palPalEntry[i].peRed = red[i];
            pPal->palPalEntry[i].peGreen = green[i];
            pPal->palPalEntry[i].peBlue = blue[i];
            pPal->palPalEntry[i].peFlags = 0;
        }

        m_hPalette = unique_palette(::CreatePalette(pPal));
        free(pPal);
    }

    wxPaletteRefData(const wxPaletteRefData& data)
    {
        const WXUINT n = data.GetEntries();
        if ( !n )
            return;

        LOGPALETTE *pPal = Alloc(n);
        if ( !pPal )
            return;

        if ( ::GetPaletteEntries(data.m_hPalette.get(), 0, n, pPal->palPalEntry) )
            m_hPalette = unique_palette(::CreatePalette(pPal));

        free(pPal);
    }

    bool IsOk() const override { return m_hPalette.get() != nullptr; }

    WXUINT GetEntries() const
    {
        return ::GetPaletteEntries(m_hPalette.get(), 0, 0, nullptr);
    }

private:
    // caller must free() the pointer
    static LOGPALETTE *Alloc(WXUINT numEntries)
    {
        LOGPALETTE *pPal = (LOGPALETTE *)
            malloc(sizeof(LOGPALETTE) + numEntries*sizeof(PALETTEENTRY));
        if ( pPal )
        {
            pPal->palVersion = 0x300;
            pPal->palNumEntries = numEntries;
        }

        return pPal;
    }

    unique_palette m_hPalette;

    friend class wxPalette;
};

// ============================================================================
// wxPalette
// ============================================================================

#define M_PALETTEDATA ((wxPaletteRefData *)m_refData)

bool wxPalette::Create(int n,
                       const unsigned char *red,
                       const unsigned char *green,
                       const unsigned char *blue)
{
    m_refData = new wxPaletteRefData(n, red, green, blue);

    return IsOk();
}

wxGDIRefData *wxPalette::CreateGDIRefData() const
{
    return new wxPaletteRefData;
}

wxGDIRefData *wxPalette::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxPaletteRefData(*dynamic_cast<const wxPaletteRefData *>(data));
}

int wxPalette::GetColoursCount() const
{
    return IsOk() ? M_PALETTEDATA->GetEntries() : 0;
}

int wxPalette::GetPixel(unsigned char red,
                        unsigned char green,
                        unsigned char blue) const
{
    if ( !m_refData )
        return wxNOT_FOUND;

    return ::GetNearestPaletteIndex(M_PALETTEDATA->m_hPalette.get(),
                                    PALETTERGB(red, green, blue));
}

bool wxPalette::GetRGB(int index,
                       unsigned char *red,
                       unsigned char *green,
                       unsigned char *blue) const
{
    if ( !m_refData )
        return false;

    if (index < 0 || index > 255)
        return false;

    PALETTEENTRY entry;
    if ( !::GetPaletteEntries(M_PALETTEDATA->m_hPalette.get(), index, 1, &entry) )
        return false;

    *red = entry.peRed;
    *green = entry.peGreen;
    *blue = entry.peBlue;

    return true;
}

WXHPALETTE wxPalette::GetHPALETTE() const
{
    return M_PALETTEDATA ? (WXHPALETTE)M_PALETTEDATA->m_hPalette.get() : nullptr;
}

void wxPalette::SetHPALETTE(WXHPALETTE pal)
{
    AllocExclusive();

    M_PALETTEDATA->m_hPalette = unique_palette(pal);
}

#endif // wxUSE_PALETTE
