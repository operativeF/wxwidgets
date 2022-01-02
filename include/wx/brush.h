/////////////////////////////////////////////////////////////////////////////
// Name:        wx/brush.h
// Purpose:     Includes platform-specific wxBrush file
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BRUSH_H_BASE_
#define _WX_BRUSH_H_BASE_

#include "wx/gdiobj.h"
#include "wx/gdicmn.h"      // for wxGDIObjListBase

enum class wxBrushStyle
{
    Invalid,
    Solid,
    Transparent,
    StippleMaskOpaque,
    StippleMask,
    Stipple,
    BDiagonalHatch,
    CrossDiagHatch,
    FDiagonalHatch,
    CrossHatch,
    HorizontalHatch,
    VerticalHatch,
    FirstHatch,
    LastHatch
};


// wxBrushBase
class wxBrushBase: public wxGDIObject
{
public:
    virtual void SetColour(const wxColour& col) = 0;
    virtual void SetColour(unsigned char r, unsigned char g, unsigned char b) = 0;
    virtual void SetStyle(wxBrushStyle style) = 0;
    virtual void SetStipple(const wxBitmap& stipple) = 0;

    virtual wxColour GetColour() const = 0;
    virtual wxBrushStyle GetStyle() const = 0;
    virtual wxBitmap *GetStipple() const = 0;

    virtual bool IsHatch() const
        { return (GetStyle()>=wxBrushStyle::FirstHatch) && (GetStyle()<=wxBrushStyle::LastHatch); }

    // Convenient helpers for testing whether the brush is a transparent one:
    // unlike GetStyle() == wxBrushStyle::Transparent, they work correctly even
    // if the brush is invalid (they both return false in this case).
    bool IsTransparent() const
    {
        return IsOk() && GetStyle() == wxBrushStyle::Transparent;
    }

    bool IsNonTransparent() const
    {
        return IsOk() && GetStyle() != wxBrushStyle::Transparent;
    }
};

#if defined(__WXMSW__)
    #include "wx/msw/brush.h"
#elif defined(__WXMOTIF__) || defined(__WXX11__)
    #include "wx/x11/brush.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/brush.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/brush.h"
#elif defined(__WXDFB__)
    #include "wx/dfb/brush.h"
#elif defined(__WXMAC__)
    #include "wx/osx/brush.h"
#elif defined(__WXQT__)
    #include "wx/qt/brush.h"
#endif

class wxBrushList: public wxGDIObjListBase
{
public:
    wxBrush *FindOrCreateBrush(const wxColour& colour,
                               wxBrushStyle style = wxBrushStyle::Solid);
};

inline wxBrushList*   wxTheBrushList;

// provide comparison operators to allow code such as
//
//      if ( brush.GetStyle() == wxTRANSPARENT )
//
// to compile without warnings which it would otherwise provoke from some
// compilers as it compares elements of different enums

#endif // _WX_BRUSH_H_BASE_
