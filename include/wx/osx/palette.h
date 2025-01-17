/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/palette.h
// Purpose:     wxPalette class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PALETTE_H_
#define _WX_PALETTE_H_

#include "wx/gdiobj.h"

#define M_PALETTEDATA ((wxPaletteRefData *)m_refData)

class wxPalette : public wxPaletteBase
{
public:
    wxPalette();

    wxPalette(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    virtual ~wxPalette();
    bool Create(int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);

    int GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;
    bool GetRGB(int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) const;

    int GetColoursCount() const override;

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPalette);
};

#endif // _WX_PALETTE_H_
