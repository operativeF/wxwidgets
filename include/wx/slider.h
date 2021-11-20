///////////////////////////////////////////////////////////////////////////////
// Name:        wx/slider.h
// Purpose:     wxSlider interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     09.02.01
// Copyright:   (c) 1996-2001 Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SLIDER_H_BASE_
#define _WX_SLIDER_H_BASE_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/defs.h"

#if wxUSE_SLIDER

#include "wx/control.h"

import <string>;

// ----------------------------------------------------------------------------
// wxSlider flags
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxSL_HORIZONTAL = wxHORIZONTAL; /* 0x0004 */
inline constexpr unsigned int wxSL_VERTICAL   = wxVERTICAL;   /* 0x0008 */

inline constexpr unsigned int wxSL_TICKS           = 0x0010;
inline constexpr unsigned int wxSL_AUTOTICKS       = wxSL_TICKS; // we don't support manual ticks
inline constexpr unsigned int wxSL_LEFT            = 0x0040;
inline constexpr unsigned int wxSL_TOP             = 0x0080;
inline constexpr unsigned int wxSL_RIGHT           = 0x0100;
inline constexpr unsigned int wxSL_BOTTOM          = 0x0200;
inline constexpr unsigned int wxSL_BOTH            = 0x0400;
inline constexpr unsigned int wxSL_SELRANGE        = 0x0800;
inline constexpr unsigned int wxSL_INVERSE         = 0x1000;
inline constexpr unsigned int wxSL_MIN_MAX_LABELS  = 0x2000;
inline constexpr unsigned int wxSL_VALUE_LABEL     = 0x4000;
inline constexpr unsigned int wxSL_LABELS          = wxSL_MIN_MAX_LABELS | wxSL_VALUE_LABEL;

inline constexpr std::string_view wxSliderNameStr = "slider";

// ----------------------------------------------------------------------------
// wxSliderBase: define wxSlider interface
// ----------------------------------------------------------------------------

class wxSliderBase : public wxControl
{
public:
    /* the ctor of the derived class should have the following form:

    wxSlider(wxWindow *parent,
             wxWindowID id,
             int value, int minValue, int maxValue,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             unsigned int style = wxSL_HORIZONTAL,
             const wxValidator& validator = wxDefaultValidator,
             std::string_view name = wxSliderNameStr);
    */
    wxSliderBase& operator=(wxSliderBase&&) = delete;

    // get/set the current slider value (should be in range)
    virtual int GetValue() const = 0;
    virtual void SetValue(int value) = 0;

    // retrieve/change the range
    virtual void SetRange(int minValue, int maxValue) = 0;
    virtual int GetMin() const = 0;
    virtual int GetMax() const = 0;
    void SetMin( int minValue ) { SetRange( minValue , GetMax() ) ; }
    void SetMax( int maxValue ) { SetRange( GetMin() , maxValue ) ; }

    // the line/page size is the increment by which the slider moves when
    // cursor arrow key/page up or down are pressed (clicking the mouse is like
    // pressing PageUp/Down) and are by default set to 1 and 1/10 of the range
    virtual void SetLineSize(int lineSize) = 0;
    virtual void SetPageSize(int pageSize) = 0;
    virtual int GetLineSize() const = 0;
    virtual int GetPageSize() const = 0;

    // these methods get/set the length of the slider pointer in pixels
    virtual void SetThumbLength(int lenPixels) = 0;
    virtual int GetThumbLength() const = 0;

    // warning: most of subsequent methods are currently only implemented in
    //          wxMSW and are silently ignored on other platforms

    void SetTickFreq(int freq) { DoSetTickFreq(freq); }
    virtual int GetTickFreq() const { return 0; }
    virtual void ClearTicks() { }
    virtual void SetTick(int WXUNUSED(tickPos)) { }

    virtual void ClearSel() { }
    virtual int GetSelEnd() const { return GetMin(); }
    virtual int GetSelStart() const { return GetMax(); }
    virtual void SetSelection(int WXUNUSED(min), int WXUNUSED(max)) { }

protected:
    // Platform-specific implementation of SetTickFreq
    virtual void DoSetTickFreq(int WXUNUSED(freq)) { /* unsupported by default */ }

    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // adjust value according to wxSL_INVERSE style
    virtual int ValueInvertOrNot(int value) const
    {
        if (HasFlag(wxSL_INVERSE))
            return (GetMax() + GetMin()) - value;
        else
            return value;
    }
};

// ----------------------------------------------------------------------------
// include the real class declaration
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/slider.h"
#elif defined(__WXMSW__)
    #include "wx/msw/slider.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/slider.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/slider.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/slider.h"
#elif defined(__WXMAC__)
    #include "wx/osx/slider.h"
#elif defined(__WXQT__)
    #include "wx/qt/slider.h"
#endif

#endif // wxUSE_SLIDER

#endif
    // _WX_SLIDER_H_BASE_
