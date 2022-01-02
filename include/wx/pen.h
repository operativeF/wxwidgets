/////////////////////////////////////////////////////////////////////////////
// Name:        wx/pen.h
// Purpose:     Base header for wxPen
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PEN_H_BASE_
#define _WX_PEN_H_BASE_

#include "wx/gdiobj.h"
#include "wx/peninfobase.h"

// Possible values for pen quality.
enum class wxPenQuality
{
    Default,  // Select the appropriate quality automatically.
    Low,      // Less good looking but faster.
    High      // Best looking, at the expense of speed.
};

// ----------------------------------------------------------------------------
// wxPenInfo contains all parameters describing a wxPen
// ----------------------------------------------------------------------------

class wxPenInfo : public wxPenInfoBase<wxPenInfo>
{
public:
    explicit wxPenInfo(const wxColour& colour = wxColour(),
                       int width = 1,
                       wxPenStyle style = wxPenStyle::Solid)
        : wxPenInfoBase<wxPenInfo>(colour, style),
          m_width(width),
          m_quality(wxPenQuality::Default)
    {
    }

    // Setters

    wxPenInfo& Width(int width)
        { m_width = width; return *this; }

    wxPenInfo& Quality(wxPenQuality quality)
        { m_quality = quality; return *this; }
    wxPenInfo& LowQuality() { return Quality(wxPenQuality::Low); }
    wxPenInfo& HighQuality() { return Quality(wxPenQuality::High); }

    // Accessors

    int GetWidth() const { return m_width; }

    wxPenQuality GetQuality() const { return m_quality; }

private:
    int m_width;
    wxPenQuality m_quality;
};


class wxPenBase : public wxGDIObject
{
public:
    virtual void SetColour(const wxColour& col) = 0;
    virtual void SetColour(unsigned char r, unsigned char g, unsigned char b) = 0;

    virtual void SetWidth(int width) = 0;
    virtual void SetStyle(wxPenStyle style) = 0;
    virtual void SetStipple(const wxBitmap& stipple) = 0;
    virtual void SetDashes(int nb_dashes, const wxDash *dash) = 0;
    virtual void SetJoin(wxPenJoin join) = 0;
    virtual void SetCap(wxPenCap cap) = 0;
    virtual void SetQuality(wxPenQuality quality) { wxUnusedVar(quality); }

    virtual wxColour GetColour() const = 0;
    virtual wxBitmap *GetStipple() const = 0;
    virtual wxPenStyle GetStyle() const = 0;
    virtual wxPenJoin GetJoin() const = 0;
    virtual wxPenCap GetCap() const = 0;
    virtual wxPenQuality GetQuality() const { return wxPenQuality::Default; }
    virtual int GetWidth() const = 0;
    virtual int GetDashes(wxDash **ptr) const = 0;

    // Convenient helpers for testing whether the pen is a transparent one:
    // unlike GetStyle() == wxPenStyle::Transparent, they work correctly even if
    // the pen is invalid (they both return false in this case).
    bool IsTransparent() const
    {
        return IsOk() && GetStyle() == wxPenStyle::Transparent;
    }

    bool IsNonTransparent() const
    {
        return IsOk() && GetStyle() != wxPenStyle::Transparent;
    }
};

#if defined(__WXMSW__)
    #include "wx/msw/pen.h"
#elif defined(__WXMOTIF__) || defined(__WXX11__)
    #include "wx/x11/pen.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/pen.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/pen.h"
#elif defined(__WXDFB__)
    #include "wx/dfb/pen.h"
#elif defined(__WXMAC__)
    #include "wx/osx/pen.h"
#elif defined(__WXQT__)
    #include "wx/qt/pen.h"
#endif

class wxPenList: public wxGDIObjListBase
{
public:
    wxPen *FindOrCreatePen(const wxColour& colour,
                           int width = 1,
                           wxPenStyle style = wxPenStyle::Solid);
};

inline wxPenList*   wxThePenList;

#endif // _WX_PEN_H_BASE_
