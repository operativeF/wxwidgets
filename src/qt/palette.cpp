/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/palette.cpp
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#include "wx/palette.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxPalette,wxGDIObject)

wxPalette::wxPalette()
{
}

wxPalette::wxPalette(int WXUNUSED(n), unsigned char *WXUNUSED(red), unsigned char *WXUNUSED(green), unsigned char *WXUNUSED(blue))
{
}

bool wxPalette::Create(int WXUNUSED(n), unsigned char *WXUNUSED(red), unsigned char *WXUNUSED(green), unsigned char *WXUNUSED(blue))
{
    return false;
}

bool wxPalette::GetRGB(int WXUNUSED(pixel), unsigned char *WXUNUSED(red), unsigned char *WXUNUSED(green), unsigned char *WXUNUSED(blue)) const
{
    return false;
}

int wxPalette::GetPixel(unsigned char WXUNUSED(red), unsigned char WXUNUSED(green), unsigned char WXUNUSED(blue)) const
{
    return false;
}


wxGDIRefData *wxPalette::CreateGDIRefData() const
{
    return NULL;
}

wxGDIRefData *wxPalette::CloneGDIRefData(const wxGDIRefData *WXUNUSED(data)) const
{
    return NULL;
}

