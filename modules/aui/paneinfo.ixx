///////////////////////////////////////////////////////////////////////////////
// Name:        modules/aui/paneinfo.ixx
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by: Thomas Figueroa
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/bitmap.h"
#include "wx/frame.h"
#include "wx/window.h"

export module WX.AUI.PaneInfo;

import Utils.Geometry;

import WX.AUI.Flags;

import <string>;
import <vector>;

export
{

class wxAuiPaneInfo
{
public:
    wxAuiPaneInfo()
    {
        DefaultPane();
    }

    // Write the safe parts of a newly loaded PaneInfo structure "source" into "this"
    // used on loading perspectives etc.
    void SafeSet(wxAuiPaneInfo source)
    {
        // note source is not passed by reference so we can overwrite, to keep the
        // unsafe bits of "dest"
        source.window = window;
        source.frame = frame;
        //wxCHECK_RET(source.IsValid(),
        //            "window settings a  nd pane settings are incompatible");
        // now assign
        *this = source;
    }

    bool IsOk() const { return window != nullptr; }
    bool IsFixed() const { return !HasFlag(optionResizable); }
    bool IsResizable() const { return HasFlag(optionResizable); }
    bool IsShown() const { return !HasFlag(optionHidden); }
    bool IsFloating() const { return HasFlag(optionFloating); }
    bool IsDocked() const { return !HasFlag(optionFloating); }
    bool IsToolbar() const { return HasFlag(optionToolbar); }
    bool IsTopDockable() const { return HasFlag(optionTopDockable); }
    bool IsBottomDockable() const { return HasFlag(optionBottomDockable); }
    bool IsLeftDockable() const { return HasFlag(optionLeftDockable); }
    bool IsRightDockable() const { return HasFlag(optionRightDockable); }
    bool IsDockable() const
    {
        return HasFlag(optionTopDockable | optionBottomDockable |
                        optionLeftDockable | optionRightDockable);
    }
    bool IsFloatable() const { return HasFlag(optionFloatable); }
    bool IsMovable() const { return HasFlag(optionMovable); }
    bool IsDestroyOnClose() const { return HasFlag(optionDestroyOnClose); }
    bool wxIsMaximized() const { return HasFlag(optionMaximized); }
    bool HasCaption() const { return HasFlag(optionCaption); }
    bool HasGripper() const { return HasFlag(optionGripper); }
    bool HasBorder() const { return HasFlag(optionPaneBorder); }
    bool HasCloseButton() const { return HasFlag(buttonClose); }
    bool HasMaximizeButton() const { return HasFlag(buttonMaximize); }
    bool HasMinimizeButton() const { return HasFlag(buttonMinimize); }
    bool HasPinButton() const { return HasFlag(buttonPin); }
    bool HasGripperTop() const { return HasFlag(optionGripperTop); }

#ifdef SWIG
    %typemap(out) wxAuiPaneInfo& { $result = $self; Py_INCREF($result); }
#endif
    wxAuiPaneInfo& Window(wxWindow* w)
    {
        wxAuiPaneInfo test(*this);
        test.window = w;
        //wxCHECK_MSG(test.IsValid(), *this,
        //            "window settings and pane settings are incompatible");
        *this = test;
        return *this;
    }
    wxAuiPaneInfo& Name(const std::string& n) { name = n; return *this; }
    wxAuiPaneInfo& Caption(const std::string& c) { caption = c; return *this; }
    wxAuiPaneInfo& Icon(const wxBitmap& b) { icon = b; return *this; }
    wxAuiPaneInfo& Left() { dock_direction = wxAUI_DOCK_LEFT; return *this; }
    wxAuiPaneInfo& Right() { dock_direction = wxAUI_DOCK_RIGHT; return *this; }
    wxAuiPaneInfo& Top() { dock_direction = wxAUI_DOCK_TOP; return *this; }
    wxAuiPaneInfo& Bottom() { dock_direction = wxAUI_DOCK_BOTTOM; return *this; }
    wxAuiPaneInfo& Center() { dock_direction = wxAUI_DOCK_CENTER; return *this; }
    wxAuiPaneInfo& Centre() { dock_direction = wxAUI_DOCK_CENTRE; return *this; }
    wxAuiPaneInfo& Direction(int direction) { dock_direction = direction; return *this; }
    wxAuiPaneInfo& Layer(int layer) { dock_layer = layer; return *this; }
    wxAuiPaneInfo& Row(int row) { dock_row = row; return *this; }
    wxAuiPaneInfo& Position(int pos) { dock_pos = pos; return *this; }
    wxAuiPaneInfo& BestSize(const wxSize& size) { best_size = size; return *this; }
    wxAuiPaneInfo& MinSize(const wxSize& size) { min_size = size; return *this; }
    wxAuiPaneInfo& MaxSize(const wxSize& size) { max_size = size; return *this; }
    wxAuiPaneInfo& BestSize(int x, int y) { best_size.Set(x,y); return *this; }
    wxAuiPaneInfo& MinSize(int x, int y) { min_size.Set(x,y); return *this; }
    wxAuiPaneInfo& MaxSize(int x, int y) { max_size.Set(x,y); return *this; }
    wxAuiPaneInfo& FloatingPosition(const wxPoint& pos) { floating_pos = pos; return *this; }
    wxAuiPaneInfo& FloatingPosition(int x, int y) { floating_pos.x = x; floating_pos.y = y; return *this; }
    wxAuiPaneInfo& FloatingSize(const wxSize& size) { floating_size = size; return *this; }
    wxAuiPaneInfo& FloatingSize(int x, int y) { floating_size.Set(x,y); return *this; }
    wxAuiPaneInfo& Fixed() { return SetFlag(optionResizable, false); }
    wxAuiPaneInfo& Resizable(bool resizable = true) { return SetFlag(optionResizable, resizable); }
    wxAuiPaneInfo& Dock() { return SetFlag(optionFloating, false); }
    wxAuiPaneInfo& Float() { return SetFlag(optionFloating, true); }
    wxAuiPaneInfo& Hide() { return SetFlag(optionHidden, true); }
    wxAuiPaneInfo& Show(bool show = true) { return SetFlag(optionHidden, !show); }
    wxAuiPaneInfo& CaptionVisible(bool visible = true) { return SetFlag(optionCaption, visible); }
    wxAuiPaneInfo& Maximize() { return SetFlag(optionMaximized, true); }
    wxAuiPaneInfo& Restore() { return SetFlag(optionMaximized, false); }
    wxAuiPaneInfo& PaneBorder(bool visible = true) { return SetFlag(optionPaneBorder, visible); }
    wxAuiPaneInfo& Gripper(bool visible = true) { return SetFlag(optionGripper, visible); }
    wxAuiPaneInfo& GripperTop(bool attop = true) { return SetFlag(optionGripperTop, attop); }
    wxAuiPaneInfo& CloseButton(bool visible = true) { return SetFlag(buttonClose, visible); }
    wxAuiPaneInfo& MaximizeButton(bool visible = true) { return SetFlag(buttonMaximize, visible); }
    wxAuiPaneInfo& MinimizeButton(bool visible = true) { return SetFlag(buttonMinimize, visible); }
    wxAuiPaneInfo& PinButton(bool visible = true) { return SetFlag(buttonPin, visible); }
    wxAuiPaneInfo& DestroyOnClose(bool b = true) { return SetFlag(optionDestroyOnClose, b); }
    wxAuiPaneInfo& TopDockable(bool b = true) { return SetFlag(optionTopDockable, b); }
    wxAuiPaneInfo& BottomDockable(bool b = true) { return SetFlag(optionBottomDockable, b); }
    wxAuiPaneInfo& LeftDockable(bool b = true) { return SetFlag(optionLeftDockable, b); }
    wxAuiPaneInfo& RightDockable(bool b = true) { return SetFlag(optionRightDockable, b); }
    wxAuiPaneInfo& Floatable(bool b = true) { return SetFlag(optionFloatable, b); }
    wxAuiPaneInfo& Movable(bool b = true) { return SetFlag(optionMovable, b); }
    wxAuiPaneInfo& DockFixed(bool b = true) { return SetFlag(optionDockFixed, b); }

    wxAuiPaneInfo& Dockable(bool b = true)
    {
        return TopDockable(b).BottomDockable(b).LeftDockable(b).RightDockable(b);
    }

    wxAuiPaneInfo& DefaultPane()
    {
        wxAuiPaneInfo test(*this);
        test.state |= optionTopDockable | optionBottomDockable |
                 optionLeftDockable | optionRightDockable |
                 optionFloatable | optionMovable | optionResizable |
                 optionCaption | optionPaneBorder | buttonClose;
        // wxCHECK_MSG(test.IsValid(), *this,
        //             "window settings and pane settings are incompatible");
        *this = test;
        return *this;
    }

    wxAuiPaneInfo& CentrePane() { return CenterPane(); }
    wxAuiPaneInfo& CenterPane()
    {
        state = 0;
        return Center().PaneBorder().Resizable();
    }

    wxAuiPaneInfo& ToolbarPane()
    {
        DefaultPane();
        state |= (optionToolbar | optionGripper);
        state &= ~(optionResizable | optionCaption);
        if (dock_layer == 0)
            dock_layer = 10;
        return *this;
    }

    wxAuiPaneInfo& SetFlag(int flag, bool option_state)
    {
        wxAuiPaneInfo test(*this);
        if (option_state)
            test.state |= flag;
        else
            test.state &= ~flag;
        //wxCHECK_MSG(test.IsValid(), *this,
        //            "window settings and pane settings are incompatible");
        *this = test;
        return *this;
    }

    bool HasFlag(int flag) const
    {
        return (state & flag) != 0;
    }

#ifdef SWIG
    %typemap(out) wxAuiPaneInfo& ;
#endif

public:
    // NOTE: You can add and subtract flags from this list,
    // but do not change the values of the flags, because
    // they are stored in a binary integer format in the
    // perspective string.  If you really need to change the
    // values around, you'll have to ensure backwards-compatibility
    // in the perspective loading code.
    enum wxAuiPaneState
    {
        optionFloating        = 1 << 0,
        optionHidden          = 1 << 1,
        optionLeftDockable    = 1 << 2,
        optionRightDockable   = 1 << 3,
        optionTopDockable     = 1 << 4,
        optionBottomDockable  = 1 << 5,
        optionFloatable       = 1 << 6,
        optionMovable         = 1 << 7,
        optionResizable       = 1 << 8,
        optionPaneBorder      = 1 << 9,
        optionCaption         = 1 << 10,
        optionGripper         = 1 << 11,
        optionDestroyOnClose  = 1 << 12,
        optionToolbar         = 1 << 13,
        optionActive          = 1 << 14,
        optionGripperTop      = 1 << 15,
        optionMaximized       = 1 << 16,
        optionDockFixed       = 1 << 17,

        buttonClose           = 1 << 21,
        buttonMaximize        = 1 << 22,
        buttonMinimize        = 1 << 23,
        buttonPin             = 1 << 24,

        buttonCustom1         = 1 << 26,
        buttonCustom2         = 1 << 27,
        buttonCustom3         = 1 << 28,

        savedHiddenState      = 1 << 30, // used internally
        actionPane            = 1u << 31  // used internally
    };

public:
    std::string name;        // name of the pane
    std::string caption;     // caption displayed on the window
    
    wxBitmap icon;        // icon of the pane, may be invalid

    wxRect rect;              // current rectangle (populated by wxAUI)

    wxWindow* window{nullptr};     // window that is in this pane
    wxFrame* frame{nullptr};       // floating frame window that holds the pane
    
    wxSize best_size{wxDefaultSize};     // size that the layout engine will prefer
    wxSize min_size{wxDefaultSize};      // minimum size the pane window can tolerate
    wxSize max_size{wxDefaultSize};      // maximum size the pane window can tolerate
    wxSize floating_size{wxDefaultSize}; // size while floating

    wxPoint floating_pos{wxDefaultPosition}; // position while floating

    unsigned int state{0};   // a combination of wxPaneState values

    int dock_direction{wxAUI_DOCK_LEFT};   // dock direction (top, bottom, left, right, center)
    int dock_layer{0};       // layer number (0 = innermost layer)
    int dock_row{0};         // row number on the docking bar (0 = first row)
    int dock_pos{0};         // position inside the row (0 = first position)
    int dock_proportion{0};  // proportion while docked

    // FIXME: This is all detectable at compile-time.
    //bool IsValid() const;
};

using wxAuiPaneInfoArray    = std::vector<wxAuiPaneInfo>;
using wxAuiPaneInfoPtrArray = std::vector<wxAuiPaneInfo*>;

} // export