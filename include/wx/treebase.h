/////////////////////////////////////////////////////////////////////////////
// Name:        wx/treebase.h
// Purpose:     wxTreeCtrl base classes and types
// Author:      Julian Smart et al
// Modified by:
// Created:     01/02/97
// Copyright:   (c) 1997,1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TREEBASE_H_
#define _WX_TREEBASE_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if wxUSE_TREECTRL

#include "wx/event.h"
#include "wx/itemid.h"

import WX.Cmn.ClntData;

// ----------------------------------------------------------------------------
// wxTreeItemId identifies an element of the tree. It's opaque for the
// application and the only method which can be used by user code is IsOk().
// ----------------------------------------------------------------------------

// This is a class and not a typedef because existing code may forward declare
// wxTreeItemId as a class and we don't want to break it without good reason.
class wxTreeItemId : public wxItemId<void*>
{
public:
    wxTreeItemId()  = default;
    wxTreeItemId(void* pItem) : wxItemId<void*>(pItem) { }
};

// ----------------------------------------------------------------------------
// wxTreeItemData is some (arbitrary) user class associated with some item. The
// main advantage of having this class (compared to old untyped interface) is
// that wxTreeItemData's are destroyed automatically by the tree and, as this
// class has virtual dtor, it means that the memory will be automatically
// freed. OTOH, we don't just use wxObject instead of wxTreeItemData because
// the size of this class is critical: in any real application, each tree leaf
// will have wxTreeItemData associated with it and number of leaves may be
// quite big.
//
// Because the objects of this class are deleted by the tree, they should
// always be allocated on the heap!
// ----------------------------------------------------------------------------

class wxTreeItemData: public wxClientData
{
friend class wxTreeCtrl;
friend class wxGenericTreeCtrl;
public:
    // creation/destruction
    // --------------------
        // default ctor
    wxTreeItemData() = default;

        // default copy ctor/assignment operator are ok

    // accessor: get the item associated with us
    const wxTreeItemId& GetId() const { return m_pItem; }
    void SetId(const wxTreeItemId& id) { m_pItem = id; }

protected:
    wxTreeItemId m_pItem;
};

using wxTreeItemIdValue = void*;

using wxArrayTreeItemIds = std::vector<wxTreeItemIdValue>;

// this is a wrapper around the array class defined above which allow to wok
// with values of natural wxTreeItemId type instead of using wxTreeItemIdValue
// and does it without any loss of efficiency

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// enum for different images associated with a treectrl item
enum wxTreeItemIcon
{
    wxTreeItemIcon_Normal,              // not selected, not expanded
    wxTreeItemIcon_Selected,            //     selected, not expanded
    wxTreeItemIcon_Expanded,            // not selected,     expanded
    wxTreeItemIcon_SelectedExpanded,    //     selected,     expanded
    wxTreeItemIcon_Max
};

// special values for the 'state' parameter of wxTreeCtrl::SetItemState()
inline constexpr int wxTREE_ITEMSTATE_NONE  = -1;   // not state (no display state image)
inline constexpr int wxTREE_ITEMSTATE_NEXT  = -2;   // cycle to the next state
inline constexpr int wxTREE_ITEMSTATE_PREV  = -3;   // cycle to the previous state

// ----------------------------------------------------------------------------
// wxTreeCtrl flags
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxTR_NO_BUTTONS              = 0x0000;     // for convenience
inline constexpr unsigned int wxTR_HAS_BUTTONS             = 0x0001;     // draw collapsed/expanded btns
inline constexpr unsigned int wxTR_NO_LINES                = 0x0004;     // don't draw lines at all
inline constexpr unsigned int wxTR_LINES_AT_ROOT           = 0x0008;     // connect top-level nodes
inline constexpr unsigned int wxTR_TWIST_BUTTONS           = 0x0010;     // still used by wxTreeListCtrl

inline constexpr unsigned int wxTR_SINGLE                  = 0x0000;     // for convenience
inline constexpr unsigned int wxTR_MULTIPLE                = 0x0020;     // can select multiple items

inline constexpr unsigned int wxTR_HAS_VARIABLE_ROW_HEIGHT = 0x0080;     // what it says

inline constexpr unsigned int wxTR_EDIT_LABELS             = 0x0200;     // can edit item labels
inline constexpr unsigned int wxTR_ROW_LINES               = 0x0400;     // put border around items
inline constexpr unsigned int wxTR_HIDE_ROOT               = 0x0800;     // don't display root node

inline constexpr unsigned int wxTR_FULL_ROW_HIGHLIGHT      = 0x2000;     // highlight full horz space

// make the default control appearance look more native-like depending on the
// platform
#if defined(__WXGTK20__)
    #define wxTR_DEFAULT_STYLE       (wxTR_HAS_BUTTONS | wxTR_NO_LINES)
#elif defined(__WXMAC__)
    #define wxTR_DEFAULT_STYLE \
        (wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_FULL_ROW_HIGHLIGHT)
#else
    #define wxTR_DEFAULT_STYLE       (wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT)
#endif

// values for the `flags' parameter of wxTreeCtrl::HitTest() which determine
// where exactly the specified point is situated:

inline constexpr unsigned int wxTREE_HITTEST_ABOVE            = 0x0001;
inline constexpr unsigned int wxTREE_HITTEST_BELOW            = 0x0002;
inline constexpr unsigned int wxTREE_HITTEST_NOWHERE          = 0x0004;
    // on the button associated with an item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMBUTTON     = 0x0008;
    // on the bitmap associated with an item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMICON       = 0x0010;
    // on the indent associated with an item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMINDENT     = 0x0020;
    // on the label (string) associated with an item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMLABEL      = 0x0040;
    // on the right of the label associated with an item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMRIGHT      = 0x0080;
    // on the label (string) associated with an item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMSTATEICON  = 0x0100;
    // on the left of the wxTreeCtrl.
inline constexpr unsigned int wxTREE_HITTEST_TOLEFT           = 0x0200;
    // on the right of the wxTreeCtrl.
inline constexpr unsigned int wxTREE_HITTEST_TORIGHT          = 0x0400;
    // on the upper part (first half) of the item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMUPPERPART  = 0x0800;
    // on the lower part (second half) of the item.
inline constexpr unsigned int wxTREE_HITTEST_ONITEMLOWERPART  = 0x1000;

    // anywhere on the item
inline constexpr unsigned int wxTREE_HITTEST_ONITEM  = wxTREE_HITTEST_ONITEMICON |
                                                wxTREE_HITTEST_ONITEMLABEL;

// tree ctrl default name
inline constexpr std::string_view wxTreeCtrlNameStr = "treeCtrl";

// ----------------------------------------------------------------------------
// wxTreeEvent is a special class for all events associated with tree controls
//
// NB: note that not all accessors make sense for all events, see the event
//     descriptions below
// ----------------------------------------------------------------------------

class wxTreeCtrlBase;

class wxTreeEvent : public wxNotifyEvent
{
public:
    wxTreeEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
    wxTreeEvent(wxEventType commandType,
                wxTreeCtrlBase *tree,
                const wxTreeItemId &item = wxTreeItemId());
    wxTreeEvent(const wxTreeEvent& event) = default;

    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxTreeEvent>(*this); }

        // get the item on which the operation was performed or the newly
        // selected item for wxEVT_TREE_SEL_CHANGED/ING events
    wxTreeItemId GetItem() const { return m_item; }
    void SetItem(const wxTreeItemId& item) { m_item = item; }

        // for wxEVT_TREE_SEL_CHANGED/ING events, get the previously
        // selected item
    wxTreeItemId GetOldItem() const { return m_itemOld; }
    void SetOldItem(const wxTreeItemId& item) { m_itemOld = item; }

        // the point where the mouse was when the drag operation started (for
        // wxEVT_TREE_BEGIN_(R)DRAG events only) or click position
    wxPoint GetPoint() const { return m_pointDrag; }
    void SetPoint(const wxPoint& pt) { m_pointDrag = pt; }

        // keyboard data (for wxEVT_TREE_KEY_DOWN only)
    const wxKeyEvent& GetKeyEvent() const { return m_evtKey; }
    int GetKeyCode() const { return m_evtKey.GetKeyCode(); }
    void SetKeyEvent(const wxKeyEvent& evt) { m_evtKey = evt; }

        // label (for EVT_TREE_{BEGIN|END}_LABEL_EDIT only)
    const std::string& GetLabel() const { return m_label; }
    void SetLabel(const std::string& label) { m_label = label; }

        // edit cancel flag (for EVT_TREE_{BEGIN|END}_LABEL_EDIT only)
    bool IsEditCancelled() const { return m_editCancelled; }
    void SetEditCanceled(bool editCancelled) { m_editCancelled = editCancelled; }

        // Set the tooltip for the item (for EVT_TREE_ITEM_GETTOOLTIP events)
    void SetToolTip(const std::string& toolTip) { m_label = toolTip; }
    std::string GetToolTip() const { return m_label; }

private:
    std::string      m_label;

    // not all of the members are used (or initialized) for all events
    wxKeyEvent    m_evtKey;
    wxTreeItemId  m_item,
                  m_itemOld;
    wxPoint       m_pointDrag;
    bool          m_editCancelled{false};

    friend class wxTreeCtrl;
    friend class wxGenericTreeCtrl;

    wxDECLARE_DYNAMIC_CLASS(wxTreeEvent);
};

typedef void (wxEvtHandler::*wxTreeEventFunction)(wxTreeEvent&);

// ----------------------------------------------------------------------------
// tree control events and macros for handling them
// ----------------------------------------------------------------------------

wxDECLARE_EVENT( wxEVT_TREE_BEGIN_DRAG, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_BEGIN_RDRAG, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_BEGIN_LABEL_EDIT, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_END_LABEL_EDIT, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_DELETE_ITEM, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_GET_INFO, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_SET_INFO, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_EXPANDED, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_EXPANDING, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_COLLAPSED, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_COLLAPSING, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_SEL_CHANGED, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_SEL_CHANGING, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_KEY_DOWN, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_ACTIVATED, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_RIGHT_CLICK, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_MIDDLE_CLICK, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_END_DRAG, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_STATE_IMAGE_CLICK, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_GETTOOLTIP, wxTreeEvent );
wxDECLARE_EVENT( wxEVT_TREE_ITEM_MENU, wxTreeEvent );

#define wxTreeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxTreeEventFunction, func)

#define wx__DECLARE_TREEEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_TREE_ ## evt, id, wxTreeEventHandler(fn))

// GetItem() returns the item being dragged, GetPoint() the mouse coords
//
// if you call event.Allow(), the drag operation will start and a
// EVT_TREE_END_DRAG event will be sent when the drag is over.
#define EVT_TREE_BEGIN_DRAG(id, fn) wx__DECLARE_TREEEVT(BEGIN_DRAG, id, fn)
#define EVT_TREE_BEGIN_RDRAG(id, fn) wx__DECLARE_TREEEVT(BEGIN_RDRAG, id, fn)

// GetItem() is the item on which the drop occurred (if any) and GetPoint() the
// current mouse coords
#define EVT_TREE_END_DRAG(id, fn) wx__DECLARE_TREEEVT(END_DRAG, id, fn)

// GetItem() returns the itme whose label is being edited, GetLabel() returns
// the current item label for BEGIN and the would be new one for END.
//
// Vetoing BEGIN event means that label editing won't happen at all,
// vetoing END means that the new value is discarded and the old one kept
#define EVT_TREE_BEGIN_LABEL_EDIT(id, fn) wx__DECLARE_TREEEVT(BEGIN_LABEL_EDIT, id, fn)
#define EVT_TREE_END_LABEL_EDIT(id, fn) wx__DECLARE_TREEEVT(END_LABEL_EDIT, id, fn)

// provide/update information about GetItem() item
#define EVT_TREE_GET_INFO(id, fn) wx__DECLARE_TREEEVT(GET_INFO, id, fn)
#define EVT_TREE_SET_INFO(id, fn) wx__DECLARE_TREEEVT(SET_INFO, id, fn)

// GetItem() is the item being expanded/collapsed, the "ING" versions can use
#define EVT_TREE_ITEM_EXPANDED(id, fn) wx__DECLARE_TREEEVT(ITEM_EXPANDED, id, fn)
#define EVT_TREE_ITEM_EXPANDING(id, fn) wx__DECLARE_TREEEVT(ITEM_EXPANDING, id, fn)
#define EVT_TREE_ITEM_COLLAPSED(id, fn) wx__DECLARE_TREEEVT(ITEM_COLLAPSED, id, fn)
#define EVT_TREE_ITEM_COLLAPSING(id, fn) wx__DECLARE_TREEEVT(ITEM_COLLAPSING, id, fn)

// GetOldItem() is the item which had the selection previously, GetItem() is
// the item which acquires selection
#define EVT_TREE_SEL_CHANGED(id, fn) wx__DECLARE_TREEEVT(SEL_CHANGED, id, fn)
#define EVT_TREE_SEL_CHANGING(id, fn) wx__DECLARE_TREEEVT(SEL_CHANGING, id, fn)

// GetKeyCode() returns the key code
// NB: this is the only message for which GetItem() is invalid (you may get the
//     item from GetSelection())
#define EVT_TREE_KEY_DOWN(id, fn) wx__DECLARE_TREEEVT(KEY_DOWN, id, fn)

// GetItem() returns the item being deleted, the associated data (if any) will
// be deleted just after the return of this event handler (if any)
#define EVT_TREE_DELETE_ITEM(id, fn) wx__DECLARE_TREEEVT(DELETE_ITEM, id, fn)

// GetItem() returns the item that was activated (double click, enter, space)
#define EVT_TREE_ITEM_ACTIVATED(id, fn) wx__DECLARE_TREEEVT(ITEM_ACTIVATED, id, fn)

// GetItem() returns the item for which the context menu shall be shown
#define EVT_TREE_ITEM_MENU(id, fn) wx__DECLARE_TREEEVT(ITEM_MENU, id, fn)

// GetItem() returns the item that was clicked on
#define EVT_TREE_ITEM_RIGHT_CLICK(id, fn) wx__DECLARE_TREEEVT(ITEM_RIGHT_CLICK, id, fn)
#define EVT_TREE_ITEM_MIDDLE_CLICK(id, fn) wx__DECLARE_TREEEVT(ITEM_MIDDLE_CLICK, id, fn)

// GetItem() returns the item whose state image was clicked on
#define EVT_TREE_STATE_IMAGE_CLICK(id, fn) wx__DECLARE_TREEEVT(STATE_IMAGE_CLICK, id, fn)

// GetItem() is the item for which the tooltip is being requested
#define EVT_TREE_ITEM_GETTOOLTIP(id, fn) wx__DECLARE_TREEEVT(ITEM_GETTOOLTIP, id, fn)

#endif // wxUSE_TREECTRL

#endif // _WX_TREEBASE_H_
