///////////////////////////////////////////////////////////////////////////////
// Name:        modules/aui/auiflags.ixx
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by: Thomas Figueroa
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/defs.h"

export module WX.AUI.Flags;

import WX.Cfg.Flags;

export
{

enum wxAuiManagerDock
{
    wxAUI_DOCK_NONE = 0,
    wxAUI_DOCK_TOP = 1,
    wxAUI_DOCK_RIGHT = 2,
    wxAUI_DOCK_BOTTOM = 3,
    wxAUI_DOCK_LEFT = 4,
    wxAUI_DOCK_CENTER = 5,
    wxAUI_DOCK_CENTRE = wxAUI_DOCK_CENTER
};

enum wxAuiButtonId
{
    wxAUI_BUTTON_CLOSE = 101,
    wxAUI_BUTTON_MAXIMIZE_RESTORE = 102,
    wxAUI_BUTTON_MINIMIZE = 103,
    wxAUI_BUTTON_PIN = 104,
    wxAUI_BUTTON_OPTIONS = 105,
    wxAUI_BUTTON_WINDOWLIST = 106,
    wxAUI_BUTTON_LEFT = 107,
    wxAUI_BUTTON_RIGHT = 108,
    wxAUI_BUTTON_UP = 109,
    wxAUI_BUTTON_DOWN = 110,
    wxAUI_BUTTON_CUSTOM1 = 201,
    wxAUI_BUTTON_CUSTOM2 = 202,
    wxAUI_BUTTON_CUSTOM3 = 203
};

enum wxAuiToolBarStyle
{
    wxAUI_TB_TEXT          = 1 << 0,
    wxAUI_TB_NO_TOOLTIPS   = 1 << 1,
    wxAUI_TB_NO_AUTORESIZE = 1 << 2,
    wxAUI_TB_GRIPPER       = 1 << 3,
    wxAUI_TB_OVERFLOW      = 1 << 4,
    // using this style forces the toolbar to be vertical and
    // be only dockable to the left or right sides of the window
    // whereas by default it can be horizontal or vertical and
    // be docked anywhere
    wxAUI_TB_VERTICAL      = 1 << 5,
    wxAUI_TB_HORZ_LAYOUT   = 1 << 6,
    // analogous to wxAUI_TB_VERTICAL, but forces the toolbar
    // to be horizontal
    wxAUI_TB_HORIZONTAL    = 1 << 7,
    wxAUI_TB_PLAIN_BACKGROUND = 1 << 8,
    wxAUI_TB_HORZ_TEXT     = (wxAUI_TB_HORZ_LAYOUT | wxAUI_TB_TEXT),
    wxAUI_ORIENTATION_MASK = (wxAUI_TB_VERTICAL | wxAUI_TB_HORIZONTAL),
    wxAUI_TB_DEFAULT_STYLE = 0
};

enum wxAuiToolBarToolTextOrientation
{
    wxAUI_TBTOOL_TEXT_LEFT = 0,     // unused/unimplemented
    wxAUI_TBTOOL_TEXT_RIGHT = 1,
    wxAUI_TBTOOL_TEXT_TOP = 2,      // unused/unimplemented
    wxAUI_TBTOOL_TEXT_BOTTOM = 3
};

// FIXME: Bitfield
enum wxAuiNotebookOption
{
    wxAUI_NB_TOP                 = 1 << 0,
    wxAUI_NB_LEFT                = 1 << 1,  // not implemented yet
    wxAUI_NB_RIGHT               = 1 << 2,  // not implemented yet
    wxAUI_NB_BOTTOM              = 1 << 3,
    wxAUI_NB_TAB_SPLIT           = 1 << 4,
    wxAUI_NB_TAB_MOVE            = 1 << 5,
    wxAUI_NB_TAB_EXTERNAL_MOVE   = 1 << 6,
    wxAUI_NB_TAB_FIXED_WIDTH     = 1 << 7,
    wxAUI_NB_SCROLL_BUTTONS      = 1 << 8,
    wxAUI_NB_WINDOWLIST_BUTTON   = 1 << 9,
    wxAUI_NB_CLOSE_BUTTON        = 1 << 10,
    wxAUI_NB_CLOSE_ON_ACTIVE_TAB = 1 << 11,
    wxAUI_NB_CLOSE_ON_ALL_TABS   = 1 << 12,
    wxAUI_NB_MIDDLE_CLICK_CLOSE  = 1 << 13,

    wxAUI_NB_DEFAULT_STYLE = wxAUI_NB_TOP |
                             wxAUI_NB_TAB_SPLIT |
                             wxAUI_NB_TAB_MOVE |
                             wxAUI_NB_SCROLL_BUTTONS |
                             wxAUI_NB_CLOSE_ON_ACTIVE_TAB |
                             wxAUI_NB_MIDDLE_CLICK_CLOSE
};

wxALLOW_COMBINING_ENUMS(wxAuiNotebookOption, wxBorder)

enum wxAuiToolBarArtSetting
{
    wxAUI_TBART_SEPARATOR_SIZE = 0,
    wxAUI_TBART_GRIPPER_SIZE = 1,
    wxAUI_TBART_OVERFLOW_SIZE = 2,
    wxAUI_TBART_DROPDOWN_SIZE = 3
};

} // export
