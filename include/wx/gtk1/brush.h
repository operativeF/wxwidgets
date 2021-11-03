/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk1/brush.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __GTKBRUSHH__
#define __GTKBRUSHH__

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/string.h"
#include "wx/gdiobj.h"
#include "wx/bitmap.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_CORE wxBrush;

//-----------------------------------------------------------------------------
// wxBrush
//-----------------------------------------------------------------------------

class wxBrush: public wxBrushBase
{
public:
    wxBrush() { }

    wxBrush( const wxColour &colour, wxBrushStyle style = wxBrushStyle::Solid );
    wxBrush( const wxBitmap &stippleBitmap );
    virtual ~wxBrush();

    bool operator==(const wxBrush& brush) const;
    bool operator!=(const wxBrush& brush) const { return !(*this == brush); }

    wxBrushStyle GetStyle() const;
    wxColour GetColour() const;
    wxBitmap *GetStipple() const;

    void SetColour( const wxColour& col );
    void SetColour( unsigned char r, unsigned char g, unsigned char b );
    void SetStyle( wxBrushStyle style );
    void SetStipple( const wxBitmap& stipple );

private:
    virtual wxGDIRefData *CreateGDIRefData() const;
    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const;

    wxDECLARE_DYNAMIC_CLASS(wxBrush);
};

#endif // __GTKBRUSHH__
