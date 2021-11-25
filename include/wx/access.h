/////////////////////////////////////////////////////////////////////////////
// Name:        wx/access.h
// Purpose:     Accessibility classes
// Author:      Julian Smart
// Modified by:
// Created:     2003-02-12
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACCESSBASE_H_
#define _WX_ACCESSBASE_H_

// ----------------------------------------------------------------------------
// headers we have to include here
// ----------------------------------------------------------------------------

#if wxUSE_ACCESSIBILITY

#include "wx/defs.h"

#include "wx/variant.h"

import Utils.Geometry;

import <string>;

enum class wxAccStatus
{
    Fail,
    False,
    Ok,
    NotImplemented,
    NotSupported,
    InvalidArg
};

// Child ids are integer identifiers from 1 up.
// So zero represents 'this' object.
#define wxACC_SELF 0

// Navigation constants

enum class wxNavDir
{
    Down,
    FirstChild,
    LastChild,
    Left,
    Next,
    Previous,
    Right,
    Up
};

// Role constants

enum class wxAccSystemRole {
    None,
    Alert,
    Animation,
    Application,
    Border,
    ButtonDropDown,
    ButtonDropDownGRID,
    ButtonMenu,
    Caret,
    Cell,
    Character,
    Chart,
    CheckButton,
    Client,
    Clock,
    Column,
    ColumnHeader,
    ComboBox,
    Cursor,
    Diagram,
    Dial,
    Dialog,
    Document,
    Droplist,
    Equation,
    Graphic,
    Grip,
    Grouping,
    HelpBalloon,
    HotkeyField,
    Indicator,
    Link,
    List,
    ListItem,
    Menubar,
    MenuItem,
    MenuPopup,
    Outline,
    OutlineItem,
    PageTab,
    PageTabList,
    Pane,
    ProgressBar,
    PropertyPage,
    PushButton,
    RadioButton,
    Row,
    RowHeader,
    Scrollbar,
    Separator,
    Slider,
    Sound,
    SpinButton,
    StaticText,
    StatusBar,
    Table,
    Text,
    Titlebar,
    Toolbar,
    Tooltip,
    Whitespace,
    Window
};

// Object types

enum wxAccObject {
    wxOBJID_WINDOW =    0x00000000,
    wxOBJID_SYSMENU =   0xFFFFFFFF,
    wxOBJID_TITLEBAR =  0xFFFFFFFE,
    wxOBJID_MENU =      0xFFFFFFFD,
    wxOBJID_CLIENT =    0xFFFFFFFC,
    wxOBJID_VSCROLL =   0xFFFFFFFB,
    wxOBJID_HSCROLL =   0xFFFFFFFA,
    wxOBJID_SIZEGRIP =  0xFFFFFFF9,
    wxOBJID_CARET =     0xFFFFFFF8,
    wxOBJID_CURSOR =    0xFFFFFFF7,
    wxOBJID_ALERT =     0xFFFFFFF6,
    wxOBJID_SOUND =     0xFFFFFFF5
};

// Accessible states

inline constexpr unsigned int wxACC_STATE_SYSTEM_ALERT_HIGH       = 0x00000001;
inline constexpr unsigned int wxACC_STATE_SYSTEM_ALERT_MEDIUM     = 0x00000002;
inline constexpr unsigned int wxACC_STATE_SYSTEM_ALERT_LOW        = 0x00000004;
inline constexpr unsigned int wxACC_STATE_SYSTEM_ANIMATED         = 0x00000008;
inline constexpr unsigned int wxACC_STATE_SYSTEM_BUSY             = 0x00000010;
inline constexpr unsigned int wxACC_STATE_SYSTEM_CHECKED          = 0x00000020;
inline constexpr unsigned int wxACC_STATE_SYSTEM_COLLAPSED        = 0x00000040;
inline constexpr unsigned int wxACC_STATE_SYSTEM_DEFAULT          = 0x00000080;
inline constexpr unsigned int wxACC_STATE_SYSTEM_EXPANDED         = 0x00000100;
inline constexpr unsigned int wxACC_STATE_SYSTEM_EXTSELECTABLE    = 0x00000200;
inline constexpr unsigned int wxACC_STATE_SYSTEM_FLOATING         = 0x00000400;
inline constexpr unsigned int wxACC_STATE_SYSTEM_FOCUSABLE        = 0x00000800;
inline constexpr unsigned int wxACC_STATE_SYSTEM_FOCUSED          = 0x00001000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_HOTTRACKED       = 0x00002000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_INVISIBLE        = 0x00004000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_MARQUEED         = 0x00008000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_MIXED            = 0x00010000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_MULTISELECTABLE  = 0x00020000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_OFFSCREEN        = 0x00040000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_PRESSED          = 0x00080000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_PROTECTED        = 0x00100000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_READONLY         = 0x00200000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_SELECTABLE       = 0x00400000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_SELECTED         = 0x00800000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_SELFVOICING      = 0x01000000;
inline constexpr unsigned int wxACC_STATE_SYSTEM_UNAVAILABLE      = 0x02000000;

// Selection flag

enum wxAccSelectionFlags
{
    wxACC_SEL_NONE            = 0,
    wxACC_SEL_TAKEFOCUS       = 1,
    wxACC_SEL_TAKESELECTION   = 2,
    wxACC_SEL_EXTENDSELECTION = 4,
    wxACC_SEL_ADDSELECTION    = 8,
    wxACC_SEL_REMOVESELECTION = 16
};

// Accessibility event identifiers

constexpr unsigned int wxACC_EVENT_SYSTEM_SOUND              = 0x0001;
constexpr unsigned int wxACC_EVENT_SYSTEM_ALERT              = 0x0002;
constexpr unsigned int wxACC_EVENT_SYSTEM_FOREGROUND         = 0x0003;
constexpr unsigned int wxACC_EVENT_SYSTEM_MENUSTART          = 0x0004;
constexpr unsigned int wxACC_EVENT_SYSTEM_MENUEND            = 0x0005;
constexpr unsigned int wxACC_EVENT_SYSTEM_MENUPOPUPSTART     = 0x0006;
constexpr unsigned int wxACC_EVENT_SYSTEM_MENUPOPUPEND       = 0x0007;
constexpr unsigned int wxACC_EVENT_SYSTEM_CAPTURESTART       = 0x0008;
constexpr unsigned int wxACC_EVENT_SYSTEM_CAPTUREEND         = 0x0009;
constexpr unsigned int wxACC_EVENT_SYSTEM_MOVESIZESTART      = 0x000A;
constexpr unsigned int wxACC_EVENT_SYSTEM_MOVESIZEEND        = 0x000B;
constexpr unsigned int wxACC_EVENT_SYSTEM_CONTEXTHELPSTART   = 0x000C;
constexpr unsigned int wxACC_EVENT_SYSTEM_CONTEXTHELPEND     = 0x000D;
constexpr unsigned int wxACC_EVENT_SYSTEM_DRAGDROPSTART      = 0x000E;
constexpr unsigned int wxACC_EVENT_SYSTEM_DRAGDROPEND        = 0x000F;
constexpr unsigned int wxACC_EVENT_SYSTEM_DIALOGSTART        = 0x0010;
constexpr unsigned int wxACC_EVENT_SYSTEM_DIALOGEND          = 0x0011;
constexpr unsigned int wxACC_EVENT_SYSTEM_SCROLLINGSTART     = 0x0012;
constexpr unsigned int wxACC_EVENT_SYSTEM_SCROLLINGEND       = 0x0013;
constexpr unsigned int wxACC_EVENT_SYSTEM_SWITCHSTART        = 0x0014;
constexpr unsigned int wxACC_EVENT_SYSTEM_SWITCHEND          = 0x0015;
constexpr unsigned int wxACC_EVENT_SYSTEM_MINIMIZESTART      = 0x0016;
constexpr unsigned int wxACC_EVENT_SYSTEM_MINIMIZEEND        = 0x0017;
constexpr unsigned int wxACC_EVENT_OBJECT_CREATE                 = 0x8000;
constexpr unsigned int wxACC_EVENT_OBJECT_DESTROY                = 0x8001;
constexpr unsigned int wxACC_EVENT_OBJECT_SHOW                   = 0x8002;
constexpr unsigned int wxACC_EVENT_OBJECT_HIDE                   = 0x8003;
constexpr unsigned int wxACC_EVENT_OBJECT_REORDER                = 0x8004;
constexpr unsigned int wxACC_EVENT_OBJECT_FOCUS                  = 0x8005;
constexpr unsigned int wxACC_EVENT_OBJECT_SELECTION              = 0x8006;
constexpr unsigned int wxACC_EVENT_OBJECT_SELECTIONADD           = 0x8007;
constexpr unsigned int wxACC_EVENT_OBJECT_SELECTIONREMOVE        = 0x8008;
constexpr unsigned int wxACC_EVENT_OBJECT_SELECTIONWITHIN        = 0x8009;
constexpr unsigned int wxACC_EVENT_OBJECT_STATECHANGE            = 0x800A;
constexpr unsigned int wxACC_EVENT_OBJECT_LOCATIONCHANGE         = 0x800B;
constexpr unsigned int wxACC_EVENT_OBJECT_NAMECHANGE             = 0x800C;
constexpr unsigned int wxACC_EVENT_OBJECT_DESCRIPTIONCHANGE      = 0x800D;
constexpr unsigned int wxACC_EVENT_OBJECT_VALUECHANGE            = 0x800E;
constexpr unsigned int wxACC_EVENT_OBJECT_PARENTCHANGE           = 0x800F;
constexpr unsigned int wxACC_EVENT_OBJECT_HELPCHANGE             = 0x8010;
constexpr unsigned int wxACC_EVENT_OBJECT_DEFACTIONCHANGE        = 0x8011;
constexpr unsigned int wxACC_EVENT_OBJECT_ACCELERATORCHANGE      = 0x8012;

// ----------------------------------------------------------------------------
// wxAccessible
// All functions return an indication of success, failure, or not implemented.
// ----------------------------------------------------------------------------

class wxAccessible;
class wxWindow;

class wxAccessibleBase
{
public:
    wxAccessibleBase(wxWindow* win): m_window(win) {}
    virtual ~wxAccessibleBase() = default;

    wxAccessibleBase& operator=(wxAccessibleBase&&) = delete;

        // Can return either a child object, or an integer
        // representing the child element, starting from 1.
        // pt is in screen coordinates.
    virtual wxAccStatus HitTest([[maybe_unused]] const wxPoint& pt, [[maybe_unused]] int* childId, [[maybe_unused]] wxAccessible** childObject)
         { return wxAccStatus::NotImplemented; }

        // Returns the rectangle for this object (id = 0) or a child element (id > 0).
        // rect is in screen coordinates.
    virtual wxAccStatus GetLocation([[maybe_unused]] wxRect& rect, [[maybe_unused]] int elementId)
         { return wxAccStatus::NotImplemented; }

        // Navigates from fromId to toId/toObject.
    virtual wxAccStatus Navigate([[maybe_unused]] wxNavDir navDir, [[maybe_unused]] int fromId,
                [[maybe_unused]] int* toId, [[maybe_unused]] wxAccessible** toObject)
         { return wxAccStatus::NotImplemented; }

        // Gets the name of the specified object.
    virtual wxAccStatus GetName([[maybe_unused]] int childId, [[maybe_unused]] std::string* name)
         { return wxAccStatus::NotImplemented; }

        // Gets the number of children.
    virtual wxAccStatus GetChildCount([[maybe_unused]] int* childCount)
         { return wxAccStatus::NotImplemented; }

        // Gets the specified child (starting from 1).
        // If *child is NULL and return value is wxAccStatus::Ok,
        // this means that the child is a simple element and
        // not an accessible object.
    virtual wxAccStatus GetChild([[maybe_unused]] int childId, [[maybe_unused]] wxAccessible** child)
         { return wxAccStatus::NotImplemented; }

        // Gets the parent, or NULL.
    virtual wxAccStatus GetParent([[maybe_unused]] wxAccessible** parent)
         { return wxAccStatus::NotImplemented; }

        // Performs the default action. childId is 0 (the action for this object)
        // or > 0 (the action for a child).
        // Return wxAccStatus::NotSupported if there is no default action for this
        // window (e.g. an edit control).
    virtual wxAccStatus DoDefaultAction([[maybe_unused]] int childId)
         { return wxAccStatus::NotImplemented; }

        // Gets the default action for this object (0) or > 0 (the action for a child).
        // Return wxAccStatus::Ok even if there is no action. actionName is the action, or the empty
        // string if there is no action.
        // The retrieved string describes the action that is performed on an object,
        // not what the object does as a result. For example, a toolbar button that prints
        // a document has a default action of "Press" rather than "Prints the current document."
    virtual wxAccStatus GetDefaultAction([[maybe_unused]] int childId, [[maybe_unused]] std::string* actionName)
         { return wxAccStatus::NotImplemented; }

        // Returns the description for this object or a child.
    virtual wxAccStatus GetDescription([[maybe_unused]] int childId, [[maybe_unused]] std::string* description)
         { return wxAccStatus::NotImplemented; }

        // Returns help text for this object or a child, similar to tooltip text.
    virtual wxAccStatus GetHelpText([[maybe_unused]] int childId, [[maybe_unused]] std::string* helpText)
         { return wxAccStatus::NotImplemented; }

        // Returns the keyboard shortcut for this object or child.
        // Return e.g. ALT+K
    virtual wxAccStatus GetKeyboardShortcut([[maybe_unused]] int childId, [[maybe_unused]] std::string* shortcut)
         { return wxAccStatus::NotImplemented; }

        // Returns a role constant.
    virtual wxAccStatus GetRole([[maybe_unused]] int childId, [[maybe_unused]] wxAccSystemRole* role)
         { return wxAccStatus::NotImplemented; }

        // Returns a state constant.
    virtual wxAccStatus GetState([[maybe_unused]] int childId, unsigned [[maybe_unused]] int* state)
         { return wxAccStatus::NotImplemented; }

        // Returns a localized string representing the value for the object
        // or child.
    virtual wxAccStatus GetValue([[maybe_unused]] int childId, [[maybe_unused]] std::string* strValue)
         { return wxAccStatus::NotImplemented; }

        // Selects the object or child.
    virtual wxAccStatus Select([[maybe_unused]] int childId, [[maybe_unused]] wxAccSelectionFlags selectFlags)
         { return wxAccStatus::NotImplemented; }

        // Gets the window with the keyboard focus.
        // If childId is 0 and child is NULL, no object in
        // this subhierarchy has the focus.
        // If this object has the focus, child should be 'this'.
    virtual wxAccStatus GetFocus([[maybe_unused]] int* childId, [[maybe_unused]] wxAccessible** child)
         { return wxAccStatus::NotImplemented; }

#if wxUSE_VARIANT
        // Gets a variant representing the selected children
        // of this object.
        // Acceptable values:
        // - a null variant (IsNull() returns TRUE)
        // - a list variant (GetType() == "list")
        // - an integer representing the selected child element,
        //   or 0 if this object is selected (GetType() == "long")
        // - a "void*" pointer to a wxAccessible child object
    virtual wxAccStatus GetSelections([[maybe_unused]] wxVariant* selections)
         { return wxAccStatus::NotImplemented; }
#endif // wxUSE_VARIANT

// Accessors

        // Returns the window associated with this object.

    wxWindow* GetWindow() { return m_window; }

        // Sets the window associated with this object.

    void SetWindow(wxWindow* window) { m_window = window; }

// Operations

        // Each platform's implementation must define this
    // static void NotifyEvent(int eventType, wxWindow* window, wxAccObject objectType,
    //                         int objectId);

private:

// Data members

    wxWindow*   m_window;
};


// ----------------------------------------------------------------------------
// now include the declaration of the real class
// ----------------------------------------------------------------------------

#if defined(__WXMSW__)
    #include "wx/msw/ole/access.h"
#endif

#endif // wxUSE_ACCESSIBILITY

#endif // _WX_ACCESSBASE_H_

