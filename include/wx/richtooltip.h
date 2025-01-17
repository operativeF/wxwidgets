///////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtooltip.h
// Purpose:     Declaration of wxRichToolTip class.
// Author:      Vadim Zeitlin
// Created:     2011-10-07
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RICHTOOLTIP_H_
#define _WX_RICHTOOLTIP_H_

#if wxUSE_RICHTOOLTIP

#include "wx/colour.h"

import Utils.Geometry;

#include <chrono>
import <string>;

class wxFont;
class wxIcon;
class wxWindow;

class wxRichToolTipImpl;

// This enum describes the kind of the tip shown which combines both the tip
// position and appearance because the two are related (when the tip is
// positioned asymmetrically, a right handed triangle is used but an
// equilateral one when it's in the middle of a side).
//
// Automatic selects the tip appearance best suited for the current platform
// and the position best suited for the window the tooltip is shown for, i.e.
// chosen in such a way that the tooltip is always fully on screen.
//
// Other values describe the position of the tooltip itself, not the window it
// relates to. E.g. wxTipKind_Top places the tip on the top of the tooltip and
// so the tooltip itself is located beneath its associated window.
enum class wxTipKind
{
    None,
    TopLeft,
    Top,
    TopRight,
    BottomLeft,
    Bottom,
    BottomRight,
    Auto
};

// ----------------------------------------------------------------------------
// wxRichToolTip: a customizable but not necessarily native tooltip.
// ----------------------------------------------------------------------------

// Notice that this class does not inherit from wxWindow.
class wxRichToolTip
{
public:
    // Ctor must specify the tooltip title and main message, additional
    // attributes can be set later.
    wxRichToolTip(const std::string& title, const std::string& message);

    // Set the background colour: if two colours are specified, the background
    // is drawn using a gradient from top to bottom, otherwise a single solid
    // colour is used.
    void SetBackgroundColour(const wxColour& col,
                             const wxColour& colEnd = wxColour());

    // Set the small icon to show: either one of the standard information/
    // warning/error ones (the question icon doesn't make sense for a tooltip)
    // or a custom icon.
    void SetIcon(int icon = wxICON_INFORMATION);
    void SetIcon(const wxIcon& icon);

    // Set timeout after which the tooltip should disappear, in milliseconds.
    // By default the tooltip is hidden after system-dependent interval of time
    // elapses but this method can be used to change this or also disable
    // hiding the tooltip automatically entirely by passing 0 in this parameter
    // (but doing this can result in native version not being used).
    // Optionally specify a show delay.
    void SetTimeout(std::chrono::milliseconds milliseconds, std::chrono::milliseconds millisecondsShowdelay = {});

    // Choose the tip kind, possibly none. By default the tip is positioned
    // automatically, as if wxTipKind_Auto was used.
    void SetTipKind(wxTipKind tipKind);

    // Set the title text font. By default it's emphasized using the font style
    // or colour appropriate for the current platform.
    void SetTitleFont(const wxFont& font);

    // Show the tooltip for the given window and optionally a specified area.
    void ShowFor(wxWindow* win, const wxRect* rect = nullptr);

private:
    std::unique_ptr<wxRichToolTipImpl> m_impl;
};

#endif // wxUSE_RICHTOOLTIP

#endif // _WX_RICHTOOLTIP_H_
