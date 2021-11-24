/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/paletteg.h
// Purpose:
// Author:      Robert Roebling
// Created:     01/02/97
// Copyright:   (c) 1998 Robert Roebling and Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __WX_PALETTEG_H__
#define __WX_PALETTEG_H__

#include "wx/object.h"
#include "wx/gdiobj.h"
#include "wx/gdicmn.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxPalette;

//-----------------------------------------------------------------------------
// wxPalette
//-----------------------------------------------------------------------------

class wxPalette: public wxPaletteBase
{
public:
    wxPalette();
    wxPalette( int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue );
    ~wxPalette();

    bool Create( int n, const unsigned char *red, const unsigned char *green, const unsigned char *blue);
    int GetPixel( unsigned char red, unsigned char green, unsigned char blue ) const;
    bool GetRGB( int pixel, unsigned char *red, unsigned char *green, unsigned char *blue ) const;

    int GetColoursCount() const override;

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPalette);
};

#endif // __WX_PALETTEG_H__
