/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/palette.h
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_PALETTE_H_
#define _WX_QT_PALETTE_H_

class wxPalette : public wxPaletteBase
{
public:
    wxPalette();
    wxPalette(int n, unsigned char *red, unsigned char *green, unsigned char *blue);

    bool Create(int n, unsigned char *red, unsigned char *green, unsigned char *blue);

    bool GetRGB(int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) const;
    int GetPixel(unsigned char red, unsigned char green, unsigned char blue) const;

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPalette);

};

#endif // _WX_QT_PALETTE_H_
