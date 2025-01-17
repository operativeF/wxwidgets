/////////////////////////////////////////////////////////////////////////////
// Name:        wx/anybutton.h
// Purpose:     wxAnyButtonBase class
// Author:      Vadim Zeitlin
// Created:     2000-08-15 (extracted from button.h)
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ANYBUTTON_H_BASE_
#define _WX_ANYBUTTON_H_BASE_

#ifdef wxHAS_ANY_BUTTON

#include "wx/bitmap.h"
#include "wx/control.h"

import WX.Cfg.Flags;

import Utils.Geometry;

// ----------------------------------------------------------------------------
// wxAnyButton specific flags
// ----------------------------------------------------------------------------

// These flags affect label alignment
inline constexpr unsigned int wxBU_LEFT            = 0x0040;
inline constexpr unsigned int wxBU_TOP             = 0x0080;
inline constexpr unsigned int wxBU_RIGHT           = 0x0100;
inline constexpr unsigned int wxBU_BOTTOM          = 0x0200;
inline constexpr unsigned int wxBU_ALIGN_MASK      = ( wxBU_LEFT | wxBU_TOP | wxBU_RIGHT | wxBU_BOTTOM );

// These two flags are obsolete and have no effect any longer.
inline constexpr unsigned int wxBU_NOAUTODRAW      = 0x0000;
inline constexpr unsigned int wxBU_AUTODRAW        = 0x0004;

// by default, the buttons will be created with some (system dependent)
// minimal size to make them look nicer, giving this style will make them as
// small as possible
inline constexpr unsigned int wxBU_EXACTFIT        = 0x0001;

// this flag can be used to disable using the text label in the button: it is
// mostly useful when creating buttons showing bitmap and having stock id as
// without it both the standard label corresponding to the stock id and the
// bitmap would be shown
inline constexpr unsigned int wxBU_NOTEXT          = 0x0002;

// ----------------------------------------------------------------------------
// wxAnyButton: common button functionality
// ----------------------------------------------------------------------------

class wxAnyButtonBase : public wxControl
{
public:
   wxAnyButtonBase& operator=(wxAnyButtonBase&&) = delete;
    
    // show the image in the button in addition to the label: this method is
    // supported on all (major) platforms
    void SetBitmap(const wxBitmap& bitmap, wxDirection dir = wxDirection::wxLEFT)
    {
        SetBitmapLabel(bitmap);
        SetBitmapPosition(dir);
    }

    wxBitmap GetBitmap() const { return DoGetBitmap(State_Normal); }

    // Methods for setting individual images for different states: normal,
    // selected (meaning pushed or pressed), focused (meaning normal state for
    // a focused button), disabled or hover (a.k.a. hot or current).
    //
    // Remember that SetBitmap() itself must be called before any other
    // SetBitmapXXX() methods (except for SetBitmapLabel() which is a synonym
    // for it anyhow) and that all bitmaps passed to these functions should be
    // of the same size.
    void SetBitmapLabel(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Normal); }
    void SetBitmapPressed(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Pressed); }
    void SetBitmapDisabled(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Disabled); }
    void SetBitmapCurrent(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Current); }
    void SetBitmapFocus(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Focused); }

    wxBitmap GetBitmapLabel() const { return DoGetBitmap(State_Normal); }
    wxBitmap GetBitmapPressed() const { return DoGetBitmap(State_Pressed); }
    wxBitmap GetBitmapDisabled() const { return DoGetBitmap(State_Disabled); }
    wxBitmap GetBitmapCurrent() const { return DoGetBitmap(State_Current); }
    wxBitmap GetBitmapFocus() const { return DoGetBitmap(State_Focused); }


    // set the margins around the image
    void SetBitmapMargins(wxCoord x, wxCoord y) { DoSetBitmapMargins(x, y); }
    void SetBitmapMargins(const wxSize& sz) { DoSetBitmapMargins(sz.x, sz.y); }
    wxSize GetBitmapMargins() { return DoGetBitmapMargins(); }

    // set the image position relative to the text, i.e. wxLEFT means that the
    // image is to the left of the text (this is the default)
    void SetBitmapPosition(wxDirection dir);


    // Buttons on MSW can look bad if they are not native colours, because
    // then they become owner-drawn and not theme-drawn.  Disable it here
    // in wxAnyButtonBase to make it consistent.
    bool ShouldInheritColours() const override { return false; }

    // backwards compatible names for pressed/current bitmaps: they're not
    // deprecated as there is nothing really wrong with using them and no real
    // advantage to using the new names but the new names are still preferred
    wxBitmap GetBitmapSelected() const { return GetBitmapPressed(); }
    wxBitmap GetBitmapHover() const { return GetBitmapCurrent(); }

    void SetBitmapSelected(const wxBitmap& bitmap) { SetBitmapPressed(bitmap); }
    void SetBitmapHover(const wxBitmap& bitmap) { SetBitmapCurrent(bitmap); }


    // this enum is not part of wx public API, it is public because it is used
    // in non wxAnyButton-derived classes internally
    //
    // also notice that MSW code relies on the values of the enum elements, do
    // not change them without revising src/msw/button.cpp
    enum State
    {
        State_Normal,
        State_Current,    // a.k.a. hot or "hovering"
        State_Pressed,    // a.k.a. "selected" in public API for some reason
        State_Disabled,
        State_Focused,
        State_Max
    };

    // return the current setting for the "normal" state of the button, it can
    // be different from State_Normal for a wxToggleButton
    virtual State GetNormalState() const
    {
        return State_Normal;
    }

    // return true if this button shouldn't show the text label, either because
    // it doesn't have it or because it was explicitly disabled with wxBU_NOTEXT
    bool DontShowLabel() const
    {
        return HasFlag(wxBU_NOTEXT) || GetLabel().empty();
    }

    // return true if we do show the label
    bool ShowsLabel() const
    {
        return !DontShowLabel();
    }

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    virtual wxBitmap DoGetBitmap([[maybe_unused]] State which) const
        { return wxBitmap(); }
    virtual void DoSetBitmap([[maybe_unused]] const wxBitmap& bitmap,
                             [[maybe_unused]] State which)
        { }

    virtual wxSize DoGetBitmapMargins() const
        { return {0, 0}; }

    virtual void DoSetBitmapMargins([[maybe_unused]] wxCoord x, [[maybe_unused]] wxCoord y)
        { }

    virtual void DoSetBitmapPosition([[maybe_unused]] wxDirection dir)
        { }

    virtual bool DoGetAuthNeeded() const { return false; }
    virtual void DoSetAuthNeeded([[maybe_unused]] bool show) { }
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/anybutton.h"
#elif defined(__WXMSW__)
    #include "wx/msw/anybutton.h"
//#elif defined(__WXMOTIF__)
//    #include "wx/motif/anybutton.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/anybutton.h"
//#elif defined(__WXGTK__)
//    #include "wx/gtk1/anybutton.h"
#elif defined(__WXMAC__)
    #include "wx/osx/anybutton.h"
#elif defined(__WXQT__)
    #include "wx/qt/anybutton.h"
#else
    using wxAnyButton = wxAnyButtonBase;
#endif

#endif // wxHAS_ANY_BUTTON

#endif // _WX_ANYBUTTON_H_BASE_
