/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dfb/brush.h
// Purpose:     wxBrush class declaration
// Author:      Vaclav Slavik
// Created:     2006-08-04
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DFB_BRUSH_H_
#define _WX_DFB_BRUSH_H_

#include "wx/object.h"
#include "wx/gdiobj.h"
#include "wx/bitmap.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxBitmap;
class wxBrush;

//-----------------------------------------------------------------------------
// wxBrush
//-----------------------------------------------------------------------------

class wxBrush : public wxBrushBase
{
public:
    wxBrush() {}
    wxBrush(const wxColour &colour, wxBrushStyle style = wxBrushStyle::Solid);
    wxBrush(const wxBitmap &stippleBitmap);

    bool operator==(const wxBrush& brush) const;
    bool operator!=(const wxBrush& brush) const { return !(*this == brush); }

    wxBrushStyle GetStyle() const;
    wxColour GetColour() const;
    wxBitmap *GetStipple() const;

    void SetColour(const wxColour& col);
    void SetColour(unsigned char r, unsigned char g, unsigned char b);
    void SetStyle(wxBrushStyle style);
    void SetStipple(const wxBitmap& stipple);

protected:
    virtual wxGDIRefData *CreateGDIRefData() const;
    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const;

    wxDECLARE_DYNAMIC_CLASS(wxBrush);
};

#endif // _WX_DFB_BRUSH_H_
