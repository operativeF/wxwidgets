/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/palette.h
// Purpose:     wxPalette class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALETTE_H_
#define _WX_PALETTE_H_

#include "wx/gdiobj.h"

import WX.WinDef;

class wxPalette : public wxPaletteBase
{
public:
    wxPalette() = default;
    wxPalette(int n,
              const unsigned char *red, const unsigned char *green, const unsigned char *blue)
    {
        Create(n, red, green, blue);
    }

    [[maybe_unused]] bool Create(int n,
                const unsigned char *red, const unsigned char *green, const unsigned char *blue);

    int GetColoursCount() const override;

    int
    GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;

    bool
    GetRGB(int pixel,
           unsigned char *red, unsigned char *green, unsigned char *blue) const;

    // implementation
    WXHPALETTE GetHPALETTE() const;
    void SetHPALETTE(WXHPALETTE pal);

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;
};

inline const wxPalette wxNullPalette;

#endif // _WX_PALETTE_H_
