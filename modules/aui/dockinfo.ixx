///////////////////////////////////////////////////////////////////////////////
// Name:        modules/aui/dockinfo.ixx
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by: Thomas Figueroa
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

export module WX.AUI.DockInfo;

import WX.AUI.Flags;
import WX.AUI.PaneInfo;

import Utils.Geometry;

import <vector>;

export
{

class wxAuiDockInfo
{
public:
    bool IsOk() const { return dock_direction != 0; }
    bool IsHorizontal() const { return dock_direction == wxAUI_DOCK_TOP ||
                             dock_direction == wxAUI_DOCK_BOTTOM; }
    bool IsVertical() const { return dock_direction == wxAUI_DOCK_LEFT ||
                             dock_direction == wxAUI_DOCK_RIGHT ||
                             dock_direction == wxAUI_DOCK_CENTER; }
public:
    wxAuiPaneInfoPtrArray panes; // array of panes
    
    wxRect rect;              // current rectangle

    int dock_direction{0};       // dock direction (top, bottom, left, right, center)
    int dock_layer{0};           // layer number (0 = innermost layer)
    int dock_row{0};             // row number on the docking bar (0 = first row)
    int size{0};                 // size of the dock
    int min_size{0};             // minimum size of a dock (0 if there is no min)
    
    bool resizable{true};           // flag indicating whether the dock is resizable
    bool toolbar{false};             // flag indicating dock contains only toolbars
    bool fixed{false};               // flag indicating that the dock operates on
                              // absolute coordinates as opposed to proportional
    bool reserved1{false};
};

using wxAuiDockInfoArray    = std::vector<wxAuiDockInfo>;
using wxAuiDockInfoPtrArray = std::vector<wxAuiDockInfo*>;

inline const wxAuiDockInfo wxAuiNullDockInfo;

} // export
