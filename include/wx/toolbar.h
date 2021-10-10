/////////////////////////////////////////////////////////////////////////////
// Name:        wx/toolbar.h
// Purpose:     wxToolBar interface declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.11.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOOLBAR_H_BASE_
#define _WX_TOOLBAR_H_BASE_

#include "wx/borderflags.h"

// ----------------------------------------------------------------------------
// wxToolBar style flags
// ----------------------------------------------------------------------------

enum class wxToolBarStyleFlags
{
    // lay out the toolbar horizontally
    Horizontal,    // == 0x0004
    Top = Horizontal,

    // lay out the toolbar vertically
    Vertical,      // == 0x0008
    Left = Vertical,

    // "flat" buttons (Win32/GTK only)
    Flat,

    // dockable toolbar (GTK only)
    Dockable,

    // don't show the icons (they're shown by default)
    NoIcons,

    // show the text (not shown by default)
    Text,

    // don't show the divider between toolbar and the window (Win32 only)
    NoDivider,

    // no automatic alignment (Win32 only, useless)
    NoAlign,

    // show the text and the icons alongside, not vertically stacked (Win32/GTK)
    HorzLayout,

    // don't show the toolbar short help tooltips
    NoTooltips,

    // lay out toolbar at the bottom of the window
    Bottom,

    // lay out toolbar at the right edge of the window
    Right,

    _max_size
};

using ToolBarStyleFlags = CombineBitfield<wxToolBarStyleFlags, wxBorder>;

constexpr wxToolBarStyleFlags wxTB_DEFAULT_STYLE = wxToolBarStyleFlags::Horizontal;

#if wxUSE_TOOLBAR
    #include "wx/tbarbase.h"     // the base class for all toolbars

    #if defined(__WXUNIVERSAL__)
       #include "wx/univ/toolbar.h"
    #elif defined(__WXMSW__)
       #include "wx/msw/toolbar.h"
    #elif defined(__WXMOTIF__)
       #include "wx/motif/toolbar.h"
    #elif defined(__WXGTK20__)
        #include "wx/gtk/toolbar.h"
    #elif defined(__WXGTK__)
        #include "wx/gtk1/toolbar.h"
    #elif defined(__WXMAC__)
       #include "wx/osx/toolbar.h"
    #elif defined(__WXQT__)
        #include "wx/qt/toolbar.h"
    #endif
#endif // wxUSE_TOOLBAR

#endif
    // _WX_TOOLBAR_H_BASE_
