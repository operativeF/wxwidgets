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

#include "wx/defs.h"

#if wxUSE_ACCESSIBILITY

#include "wx/variant.h"

#include <string>

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

constexpr unsigned int wxACC_STATE_SYSTEM_ALERT_HIGH       = 0x00000001;
constexpr unsigned int wxACC_STATE_SYSTEM_ALERT_MEDIUM     = 0x00000002;
constexpr unsigned int wxACC_STATE_SYSTEM_ALERT_LOW        = 0x00000004;
constexpr unsigned int wxACC_STATE_SYSTEM_ANIMATED         = 0x00000008;
constexpr unsigned int wxACC_STATE_SYSTEM_BUSY             = 0x00000010;
constexpr unsigned int wxACC_STATE_SYSTEM_CHECKED          = 0x00000020;
constexpr unsigned int wxACC_STATE_SYSTEM_COLLAPSED        = 0x00000040;
constexpr unsigned int wxACC_STATE_SYSTEM_DEFAULT          = 0x00000080;
constexpr unsigned int wxACC_STATE_SYSTEM_EXPANDED         = 0x00000100;
constexpr unsigned int wxACC_STATE_SYSTEM_EXTSELECTABLE    = 0x00000200;
constexpr unsigned int wxACC_STATE_SYSTEM_FLOATING         = 0x00000400;
constexpr unsigned int wxACC_STATE_SYSTEM_FOCUSABLE        = 0x00000800;
constexpr unsigned int wxACC_STATE_SYSTEM_FOCUSED          = 0x00001000;
constexpr unsigned int wxACC_STATE_SYSTEM_HOTTRACKED       = 0x00002000;
constexpr unsigned int wxACC_STATE_SYSTEM_INVISIBLE        = 0x00004000;
constexpr unsigned int wxACC_STATE_SYSTEM_MARQUEED         = 0x00008000;
constexpr unsigned int wxACC_STATE_SYSTEM_MIXED            = 0x00010000;
constexpr unsigned int wxACC_STATE_SYSTEM_MULTISELECTABLE  = 0x00020000;
constexpr unsigned int wxACC_STATE_SYSTEM_OFFSCREEN        = 0x00040000;
constexpr unsigned int wxACC_STATE_SYSTEM_PRESSED          = 0x00080000;
constexpr unsigned int wxACC_STATE_SYSTEM_PROTECTED        = 0x00100000;
constexpr unsigned int wxACC_STATE_SYSTEM_READONLY         = 0x00200000;
constexpr unsigned int wxACC_STATE_SYSTEM_SELECTABLE       = 0x00400000;
constexpr unsigned int wxACC_STATE_SYSTEM_SELECTED         = 0x00800000;
constexpr unsigned int wxACC_STATE_SYSTEM_SELFVOICING      = 0x01000000;
constexpr unsigned int wxACC_STATE_SYSTEM_UNAVAILABLE      = 0x02000000;

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

class  WXDLLIMPEXP_FWD_CORE wxAccessible;
class  WXDLLIMPEXP_FWD_CORE wxWindow;

class WXDLLIMPEXP_CORE wxAccessibleBase
{
public:
    wxAccessibleBase(wxWindow* win): m_window(win) {}
    virtual ~wxAccessibleBase() = default;

    wxAccessibleBase& operator=(wxAccessibleBase&&) = delete;

        // Can return either a child object, or an integer
        // representing the child element, starting from 1.
        // pt is in screen coordinates.
    virtual wxAccStatus HitTest(const wxPoint& WXUNUSED(pt), int* WXUNUSED(childId), wxAccessible** WXUNUSED(childObject))
         { return wxAccStatus::NotImplemented; }

        // Returns the rectangle for this object (id = 0) or a child element (id > 0).
        // rect is in screen coordinates.
    virtual wxAccStatus GetLocation(wxRect& WXUNUSED(rect), int WXUNUSED(elementId))
         { return wxAccStatus::NotImplemented; }

        // Navigates from fromId to toId/toObject.
    virtual wxAccStatus Navigate(wxNavDir WXUNUSED(navDir), int WXUNUSED(fromId),
                int* WXUNUSED(toId), wxAccessible** WXUNUSED(toObject))
         { return wxAccStatus::NotImplemented; }

        // Gets the name of the specified object.
    virtual wxAccStatus GetName(int WXUNUSED(childId), std::string* WXUNUSED(name))
         { return wxAccStatus::NotImplemented; }

        // Gets the number of children.
    virtual wxAccStatus GetChildCount(int* WXUNUSED(childCount))
         { return wxAccStatus::NotImplemented; }

        // Gets the specified child (starting from 1).
        // If *child is NULL and return value is wxAccStatus::Ok,
        // this means that the child is a simple element and
        // not an accessible object.
    virtual wxAccStatus GetChild(int WXUNUSED(childId), wxAccessible** WXUNUSED(child))
         { return wxAccStatus::NotImplemented; }

        // Gets the parent, or NULL.
    virtual wxAccStatus GetParent(wxAccessible** WXUNUSED(parent))
         { return wxAccStatus::NotImplemented; }

        // Performs the default action. childId is 0 (the action for this object)
        // or > 0 (the action for a child).
        // Return wxAccStatus::NotSupported if there is no default action for this
        // window (e.g. an edit control).
    virtual wxAccStatus DoDefaultAction(int WXUNUSED(childId))
         { return wxAccStatus::NotImplemented; }

        // Gets the default action for this object (0) or > 0 (the action for a child).
        // Return wxAccStatus::Ok even if there is no action. actionName is the action, or the empty
        // string if there is no action.
        // The retrieved string describes the action that is performed on an object,
        // not what the object does as a result. For example, a toolbar button that prints
        // a document has a default action of "Press" rather than "Prints the current document."
    virtual wxAccStatus GetDefaultAction(int WXUNUSED(childId), std::string* WXUNUSED(actionName))
         { return wxAccStatus::NotImplemented; }

        // Returns the description for this object or a child.
    virtual wxAccStatus GetDescription(int WXUNUSED(childId), std::string* WXUNUSED(description))
         { return wxAccStatus::NotImplemented; }

        // Returns help text for this object or a child, similar to tooltip text.
    virtual wxAccStatus GetHelpText(int WXUNUSED(childId), std::string* WXUNUSED(helpText))
         { return wxAccStatus::NotImplemented; }

        // Returns the keyboard shortcut for this object or child.
        // Return e.g. ALT+K
    virtual wxAccStatus GetKeyboardShortcut(int WXUNUSED(childId), std::string* WXUNUSED(shortcut))
         { return wxAccStatus::NotImplemented; }

        // Returns a role constant.
    virtual wxAccStatus GetRole(int WXUNUSED(childId), wxAccSystemRole* WXUNUSED(role))
         { return wxAccStatus::NotImplemented; }

        // Returns a state constant.
    virtual wxAccStatus GetState(int WXUNUSED(childId), unsigned int* WXUNUSED(state))
         { return wxAccStatus::NotImplemented; }

        // Returns a localized string representing the value for the object
        // or child.
    virtual wxAccStatus GetValue(int WXUNUSED(childId), std::string* WXUNUSED(strValue))
         { return wxAccStatus::NotImplemented; }

        // Selects the object or child.
    virtual wxAccStatus Select(int WXUNUSED(childId), wxAccSelectionFlags WXUNUSED(selectFlags))
         { return wxAccStatus::NotImplemented; }

        // Gets the window with the keyboard focus.
        // If childId is 0 and child is NULL, no object in
        // this subhierarchy has the focus.
        // If this object has the focus, child should be 'this'.
    virtual wxAccStatus GetFocus(int* WXUNUSED(childId), wxAccessible** WXUNUSED(child))
         { return wxAccStatus::NotImplemented; }

#if wxUSE_VARIANT
        // Gets a variant representing the selected children
        // of this object.
        // Acceptable values:
        // - a null variant (IsNull() returns TRUE)
        // - a list variant (GetType() == wxT("list"))
        // - an integer representing the selected child element,
        //   or 0 if this object is selected (GetType() == wxT("long"))
        // - a "void*" pointer to a wxAccessible child object
    virtual wxAccStatus GetSelections(wxVariant* WXUNUSED(selections))
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

