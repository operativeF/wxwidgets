///////////////////////////////////////////////////////////////////////////////
// Name:        wx/listbase.h
// Purpose:     wxListCtrl class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.12.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LISTBASE_H_BASE_
#define _WX_LISTBASE_H_BASE_

#include "wx/colour.h"
#include "wx/font.h"
#include "wx/geometry/point.h"
#include "wx/event.h"
#include "wx/control.h"
#include "wx/itemattr.h"
#include "wx/systhemectrl.h"
#include "wx/bitflags.h"

#include <vector>

class wxImageList;

// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------

// type of compare function for wxListCtrl sort operation
typedef
int (wxCALLBACK *wxListCtrlCompare)(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData);

// ----------------------------------------------------------------------------
// wxListCtrl constants
// ----------------------------------------------------------------------------

// style flags
// FIXME: Bitfield
constexpr unsigned int wxLC_VRULES          = 0x0001;
constexpr unsigned int wxLC_HRULES          = 0x0002;

constexpr unsigned int wxLC_ICON            = 0x0004;
constexpr unsigned int wxLC_SMALL_ICON      = 0x0008;
constexpr unsigned int wxLC_LIST            = 0x0010;
constexpr unsigned int wxLC_REPORT          = 0x0020;

constexpr unsigned int wxLC_ALIGN_TOP       = 0x0040;
constexpr unsigned int wxLC_ALIGN_LEFT      = 0x0080;
constexpr unsigned int wxLC_AUTOARRANGE     = 0x0100;
constexpr unsigned int wxLC_VIRTUAL         = 0x0200;
constexpr unsigned int wxLC_EDIT_LABELS     = 0x0400;
constexpr unsigned int wxLC_NO_HEADER       = 0x0800;
constexpr unsigned int wxLC_NO_SORT_HEADER  = 0x1000;
constexpr unsigned int wxLC_SINGLE_SEL      = 0x2000;
constexpr unsigned int wxLC_SORT_ASCENDING  = 0x4000;
constexpr unsigned int wxLC_SORT_DESCENDING = 0x8000;
constexpr unsigned int wxLC_MASK_TYPE       = wxLC_ICON | wxLC_SMALL_ICON | wxLC_LIST | wxLC_REPORT;
constexpr unsigned int wxLC_MASK_ALIGN      = wxLC_ALIGN_TOP | wxLC_ALIGN_LEFT;
constexpr unsigned int wxLC_MASK_SORT       = wxLC_SORT_ASCENDING | wxLC_SORT_DESCENDING;

// for compatibility only
constexpr unsigned int wxLC_USER_TEXT       = wxLC_VIRTUAL;

// FIXME: Bitfields
// Omitted because
//  (a) too much detail
//  (b) not enough style flags
//  (c) not implemented anyhow in the generic version
//
// #define wxLC_NO_SCROLL
// #define wxLC_NO_LABEL_WRAP
// #define wxLC_OWNERDRAW_FIXED
// #define wxLC_SHOW_SEL_ALWAYS

enum class ListMasks
{
    State,
    Text,
    Image,
    Data,
    SetItem,
    Width,
    Format,
    _max_size
};

using ListMaskFlags = InclBitfield<ListMasks>;

enum class ListStates
{
    Nil,
#ifdef __WXMSW__
    DropHiLited,
#endif
    Focused,
    Selected,
#ifdef __WXMSW__
    Cut,
#endif
    Disabled,   // Unused
    Filtered,   // Unused
    InUse,      // Unused
    Picked,     // Unused
    Source,     // Unused
    _max_size
};

using ListStateFlags = InclBitfield<ListStates>;

// Hit test flags, used in HitTest
enum class ListHitTest
{
    Above,              // Above the control's client area.
    Below,              // Below the control's client area.
    Nowhere,            // Inside the control's client area but not over an item.
    OnItemIcon,         // Over an item's icon.
    OnItemLabel,        // Over an item's text.
    OnItemStateIcon,    // Over the checkbox of an item.
    ToLeft,             // To the left of the control's client area.
    ToRight,            // To the right of the control's client area.
    _max_size
};

using ListHitTestFlags = InclBitfield<ListHitTest>;

// GetSubItemRect constants
constexpr long wxLIST_GETSUBITEMRECT_WHOLEITEM = -1L;

// Flags for GetNextItem (MSW only except wxListGetNextItem::All)
enum class wxListGetNextItem
{
    Above,          // Searches for an item above the specified item
    All,            // Searches for subsequent item by index
    Below,          // Searches for an item below the specified item
    Left,           // Searches for an item to the left of the specified item
    Right           // Searches for an item to the right of the specified item
};

// Alignment flags for Arrange (MSW only except wxListAlignment::Left)
enum class wxListAlignment
{
    Default,
    Left,
    Top,
    SnapToGrid
};

// Column format (MSW only except wxListColumnFormat::Left)
enum class wxListColumnFormat
{
    Left,
    Right,
    Center
};

// Autosize values for SetColumnWidth
enum
{
    wxLIST_AUTOSIZE = -1,
    wxLIST_AUTOSIZE_USEHEADER = -2      // partly supported by generic version
};

// Flag values for GetItemRect
enum class wxListRectFlags
{
    Bounds,
    Icon,
    Label
};

// Flag values for FindItem (MSW only)
enum class wxListFind
{
    Up,
    Down,
    Left,
    Right
};

// For compatibility, define the old name for this class. There is no need to
// deprecate it as it doesn't cost us anything to keep this typedef, but the
// new code should prefer to use the new wxItemAttr name.
using wxListItemAttr = wxItemAttr;

// ----------------------------------------------------------------------------
// wxListItem: the item or column info, used to exchange data with wxListCtrl
// ----------------------------------------------------------------------------

class wxListItem
{
public:
    wxListItem() = default;
    wxListItem(const wxListItem& item)
        : 
          m_mask(item.m_mask),
          m_itemId(item.m_itemId),
          m_col(item.m_col),
          m_state(item.m_state),
          m_stateMask(item.m_stateMask),
          m_text(item.m_text),
          m_image(item.m_image),
          m_data(item.m_data),
          m_format(item.m_format),
          m_width(item.m_width),
          m_attr(nullptr)
    {
        // copy list item attributes
        if ( item.HasAttributes() )
            m_attr = new wxItemAttr(*item.GetAttributes());
    }

    wxListItem& operator=(const wxListItem& item)
    {
        if ( &item != this )
        {
            m_mask = item.m_mask;
            m_itemId = item.m_itemId;
            m_col = item.m_col;
            m_state = item.m_state;
            m_stateMask = item.m_stateMask;
            m_text = item.m_text;
            m_image = item.m_image;
            m_data = item.m_data;
            m_format = item.m_format;
            m_width = item.m_width;
            m_attr = item.m_attr ? new wxItemAttr(*item.m_attr) : nullptr;
        }

        return *this;
    }

    ~wxListItem() { delete m_attr; }

    // resetting
    void Clear()
    {
        m_mask.clear();
        m_itemId = -1;
        m_col = 0;
        m_state.clear();
        m_stateMask.clear();
        m_image = -1;
        m_data = 0;

        m_format = wxListColumnFormat::Center;
        m_width = 0;
        m_text.clear();
        ClearAttributes();
    }
    void ClearAttributes() { if ( m_attr ) { delete m_attr; m_attr = nullptr; } }

    // setters
    void SetMask(ListMaskFlags mask)
        { m_mask = mask; }
    void SetId(long id)
        { m_itemId = id; }
    void SetColumn(int col)
        { m_col = col; }
    void SetState(ListStateFlags state)
        { m_mask |= ListMasks::State; m_state = state; m_stateMask |= state; }
    void SetStateMask(ListStateFlags stateMask)
        { m_stateMask = stateMask; }
    void SetText(const wxString& text)
        { m_mask |= ListMasks::Text; m_text = text; }
    void SetImage(int image)
        { m_mask |= ListMasks::Image; m_image = image; }
    void SetData(long data)
        { m_mask |= ListMasks::Data; m_data = data; }
    void SetData(void *data)
        { m_mask |= ListMasks::Data; m_data = wxPtrToUInt(data); }

    void SetWidth(int width)
        { m_mask |= ListMasks::Width; m_width = width; }
    void SetAlign(wxListColumnFormat align)
        { m_mask |= ListMasks::Format; m_format = align; }

    void SetTextColour(const wxColour& colText)
        { Attributes().SetTextColour(colText); }
    void SetBackgroundColour(const wxColour& colBack)
        { Attributes().SetBackgroundColour(colBack); }
    void SetFont(const wxFont& font)
        { Attributes().SetFont(font); }

    ListMaskFlags GetMask() const { return m_mask; }
    long GetId() const { return m_itemId; }
    int GetColumn() const { return m_col; }
    ListStateFlags GetState() const { return m_state & m_stateMask; }
    const wxString& GetText() const { return m_text; }
    int GetImage() const { return m_image; }
    wxUIntPtr GetData() const { return m_data; }

    int GetWidth() const { return m_width; }
    wxListColumnFormat GetAlign() const { return (wxListColumnFormat)m_format; }

    wxItemAttr *GetAttributes() const { return m_attr; }
    bool HasAttributes() const { return m_attr != nullptr; }

    wxColour GetTextColour() const
        { return HasAttributes() ? m_attr->GetTextColour() : wxNullColour; }
    wxColour GetBackgroundColour() const
        { return HasAttributes() ? m_attr->GetBackgroundColour()
                                 : wxNullColour; }
    wxFont GetFont() const
        { return HasAttributes() ? m_attr->GetFont() : wxNullFont; }

    // this conversion is necessary to make old code using GetItem() to
    // compile
    operator long() const { return m_itemId; }

    // these members are public for compatibility
    wxString        m_text;     // The label/header text

    wxUIntPtr       m_data{0};     // App-defined data

    ListMaskFlags   m_mask{};     // Indicates what fields are valid
    long            m_itemId{-1};   // The zero-based item position
    ListStateFlags  m_state{};    // The state of the item
    ListStateFlags  m_stateMask{};// Which flags of m_state are valid (uses same flags)
    int             m_image{-1};    // The zero-based index into an image list
    int             m_col{0};      // Zero-based column, if in report mode

    // For columns only
    wxListColumnFormat m_format{wxListColumnFormat::Center};   // left, right, centre
    int             m_width{0};    // width of column

protected:
    // creates m_attr if we don't have it yet
    wxItemAttr& Attributes()
    {
        if ( !m_attr )
            m_attr = new wxItemAttr;

        return *m_attr;
    }

    wxItemAttr *m_attr{nullptr};     // optional pointer to the items style
};

// ----------------------------------------------------------------------------
// wxListCtrlBase: the base class for the main control itself.
// ----------------------------------------------------------------------------

// Unlike other base classes, this class doesn't currently define the API of
// the real control class but is just used for implementation convenience. We
// should define the public class functions as pure virtual here in the future
// however.
class wxListCtrlBase : public wxSystemThemedControl<wxControl>
{
public:
    // Image list methods.
    // -------------------

    // Associate the given (possibly NULL to indicate that no images will be
    // used) image list with the control. The ownership of the image list
    // passes to the control, i.e. it will be deleted when the control itself
    // is destroyed.
    //
    // The value of "which" must be one of wxIMAGE_LIST_{NORMAL,SMALL,STATE}.
    virtual void AssignImageList(wxImageList* imageList, int which) = 0;

    // Same as AssignImageList() but the control does not delete the image list
    // so it can be shared among several controls.
    virtual void SetImageList(wxImageList* imageList, int which) = 0;

    // Return the currently used image list, may be NULL.
    virtual wxImageList* GetImageList(int which) const = 0;


    // Column-related methods.
    // -----------------------

    // All these methods can only be used in report view mode.

    // Appends a new column.
    //
    // Returns the index of the newly inserted column or -1 on error.
    long AppendColumn(const wxString& heading,
                      wxListColumnFormat format = wxListColumnFormat::Left,
                      int width = -1);

    // Add a new column to the control at the position "col".
    //
    // Returns the index of the newly inserted column or -1 on error.
    long InsertColumn(long col, const wxListItem& info);
    long InsertColumn(long col,
                      const wxString& heading,
                      wxListColumnFormat format = wxListColumnFormat::Left,
                      int width = wxLIST_AUTOSIZE);

    // Delete the given or all columns.
    virtual bool DeleteColumn(int col) = 0;
    virtual bool DeleteAllColumns() = 0;

    // Return the current number of items.
    virtual int GetItemCount() const = 0;

    // Check if the control is empty, i.e. doesn't contain any items.
    bool IsEmpty() const { return GetItemCount() == 0; }

    // Return the current number of columns.
    virtual int GetColumnCount() const = 0;

    // Get or update information about the given column. Set item mask to
    // indicate the fields to retrieve or change.
    //
    // Returns false on error, e.g. if the column index is invalid.
    virtual bool GetColumn(int col, wxListItem& item) const = 0;
    virtual bool SetColumn(int col, const wxListItem& item) = 0;

    // Convenient wrappers for the above methods which get or update just the
    // column width.
    virtual int GetColumnWidth(int col) const = 0;
    virtual bool SetColumnWidth(int col, int width) = 0;

    // Column ordering functions
    virtual int GetColumnOrder(int col) const = 0;
    virtual int GetColumnIndexFromOrder(int order) const = 0;

    virtual std::vector<int> GetColumnsOrder() const = 0;
    virtual bool SetColumnsOrder(const std::vector<int>& orders) = 0;


    // Other miscellaneous accessors.
    // ------------------------------

    // Convenient functions for testing the list control mode:
    bool InReportView() const { return HasFlag(wxLC_REPORT); }
    bool IsVirtual() const { return HasFlag(wxLC_VIRTUAL); }

    // Check if the item is visible
    virtual bool IsVisible(long WXUNUSED(item)) const { return false; }

    // Enable or disable beep when incremental match doesn't find any item.
    // Only implemented in the generic version currently.
    virtual void EnableBellOnNoMatch(bool WXUNUSED(on) = true) { }

    void EnableAlternateRowColours(bool enable = true);
    void SetAlternateRowColour(const wxColour& colour);
    wxColour GetAlternateRowColour() const { return m_alternateRowColour.GetBackgroundColour(); }

    virtual void ExtendRulesAndAlternateColour(bool WXUNUSED(extend) = true) { }

    // Header attributes support: only implemented in wxMSW currently.
    virtual bool SetHeaderAttr(const wxItemAttr& WXUNUSED(attr)) { return false; }

    // Checkboxes support.
    virtual bool HasCheckBoxes() const { return false; }
    virtual bool EnableCheckBoxes(bool WXUNUSED(enable) = true) { return false; }
    virtual bool IsItemChecked(long WXUNUSED(item)) const { return false; }
    virtual void CheckItem(long WXUNUSED(item), bool WXUNUSED(check)) { }

protected:
    // Real implementations methods to which our public forwards.
    virtual long DoInsertColumn(long col, const wxListItem& info) = 0;

    // Overridden methods of the base class.
    wxSize DoGetBestClientSize() const override;

    // these functions are only used for virtual list view controls, i.e. the
    // ones with wxLC_VIRTUAL style

    // return the attribute for the item (may return NULL if none)
    virtual wxItemAttr* OnGetItemAttr(long item) const;

    // return the text for the given column of the given item
    virtual wxString OnGetItemText(long item, long column) const;

    // return whether the given item is checked
    virtual bool OnGetItemIsChecked(long item) const;

    // return the icon for the given item. In report view, OnGetItemImage will
    // only be called for the first column. See OnGetItemColumnImage for
    // details.
    virtual int OnGetItemImage(long item) const;

    // return the icon for the given item and column.
    virtual int OnGetItemColumnImage(long item, long column) const;

    // return the attribute for the given item and column (may return NULL if none)
    virtual wxItemAttr* OnGetItemColumnAttr(long item, long column) const;

private:
    // user defined color to draw row lines, may be invalid
    wxItemAttr m_alternateRowColour;
};

// ----------------------------------------------------------------------------
// wxListEvent - the event class for the wxListCtrl notifications
// ----------------------------------------------------------------------------

class wxListEvent : public wxNotifyEvent
{
public:
    wxListEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxNotifyEvent(commandType, winid) { }

    wxListEvent(const wxListEvent& event) = default;

	wxListEvent& operator=(const wxListEvent&) = delete;

    int GetKeyCode() const { return m_code; }
    long GetIndex() const { return m_itemIndex; }
    int GetColumn() const { return m_col; }
    wxPoint GetPoint() const { return m_pointDrag; }
    const wxString& GetLabel() const { return m_item.m_text; }
    const wxString& GetText() const { return m_item.m_text; }
    int GetImage() const { return m_item.m_image; }
    wxUIntPtr GetData() const { return m_item.m_data; }
    ListMaskFlags GetMask() const { return m_item.m_mask; }
    const wxListItem& GetItem() const { return m_item; }

    void SetKeyCode(int code) { m_code = code; }
    void SetIndex(long index) { m_itemIndex = index; }
    void SetColumn(int col) { m_col = col; }
    void SetPoint(const wxPoint& point) { m_pointDrag = point; }
    void SetItem(const wxListItem& item) { m_item = item; }

    // for wxEVT_LIST_CACHE_HINT only
    long GetCacheFrom() const { return m_oldItemIndex; }
    long GetCacheTo() const { return m_itemIndex; }
    void SetCacheFrom(long cacheFrom) { m_oldItemIndex = cacheFrom; }
    void SetCacheTo(long cacheTo) { m_itemIndex = cacheTo; }

    // was label editing canceled? (for wxEVT_LIST_END_LABEL_EDIT only)
    bool IsEditCancelled() const { return m_editCancelled; }
    void SetEditCanceled(bool editCancelled) { m_editCancelled = editCancelled; }

    wxEvent *Clone() const override { return new wxListEvent(*this); }

//protected: -- not for backwards compatibility
    int           m_code{-1};
    long          m_oldItemIndex{-1}; // only for wxEVT_LIST_CACHE_HINT
    long          m_itemIndex{-1};
    int           m_col{-1};
    wxPoint       m_pointDrag;

    wxListItem    m_item;

protected:
    bool          m_editCancelled{false};

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

// ----------------------------------------------------------------------------
// wxListCtrl event macros
// ----------------------------------------------------------------------------

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_BEGIN_DRAG, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_BEGIN_RDRAG, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_BEGIN_LABEL_EDIT, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_END_LABEL_EDIT, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_DELETE_ITEM, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_DELETE_ALL_ITEMS, wxListEvent );

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_SELECTED, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_DESELECTED, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_KEY_DOWN, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_INSERT_ITEM, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_COL_CLICK, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_RIGHT_CLICK, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_MIDDLE_CLICK, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_ACTIVATED, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_CACHE_HINT, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_COL_RIGHT_CLICK, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_COL_BEGIN_DRAG, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_COL_DRAGGING, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_COL_END_DRAG, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_FOCUSED, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_CHECKED, wxListEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_LIST_ITEM_UNCHECKED, wxListEvent );

typedef void (wxEvtHandler::*wxListEventFunction)(wxListEvent&);

#define wxListEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxListEventFunction, func)

#define wx__DECLARE_LISTEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_LIST_ ## evt, id, wxListEventHandler(fn))

#define EVT_LIST_BEGIN_DRAG(id, fn) wx__DECLARE_LISTEVT(BEGIN_DRAG, id, fn)
#define EVT_LIST_BEGIN_RDRAG(id, fn) wx__DECLARE_LISTEVT(BEGIN_RDRAG, id, fn)
#define EVT_LIST_BEGIN_LABEL_EDIT(id, fn) wx__DECLARE_LISTEVT(BEGIN_LABEL_EDIT, id, fn)
#define EVT_LIST_END_LABEL_EDIT(id, fn) wx__DECLARE_LISTEVT(END_LABEL_EDIT, id, fn)
#define EVT_LIST_DELETE_ITEM(id, fn) wx__DECLARE_LISTEVT(DELETE_ITEM, id, fn)
#define EVT_LIST_DELETE_ALL_ITEMS(id, fn) wx__DECLARE_LISTEVT(DELETE_ALL_ITEMS, id, fn)
#define EVT_LIST_KEY_DOWN(id, fn) wx__DECLARE_LISTEVT(KEY_DOWN, id, fn)
#define EVT_LIST_INSERT_ITEM(id, fn) wx__DECLARE_LISTEVT(INSERT_ITEM, id, fn)

#define EVT_LIST_COL_CLICK(id, fn) wx__DECLARE_LISTEVT(COL_CLICK, id, fn)
#define EVT_LIST_COL_RIGHT_CLICK(id, fn) wx__DECLARE_LISTEVT(COL_RIGHT_CLICK, id, fn)
#define EVT_LIST_COL_BEGIN_DRAG(id, fn) wx__DECLARE_LISTEVT(COL_BEGIN_DRAG, id, fn)
#define EVT_LIST_COL_DRAGGING(id, fn) wx__DECLARE_LISTEVT(COL_DRAGGING, id, fn)
#define EVT_LIST_COL_END_DRAG(id, fn) wx__DECLARE_LISTEVT(COL_END_DRAG, id, fn)

#define EVT_LIST_ITEM_SELECTED(id, fn) wx__DECLARE_LISTEVT(ITEM_SELECTED, id, fn)
#define EVT_LIST_ITEM_DESELECTED(id, fn) wx__DECLARE_LISTEVT(ITEM_DESELECTED, id, fn)
#define EVT_LIST_ITEM_RIGHT_CLICK(id, fn) wx__DECLARE_LISTEVT(ITEM_RIGHT_CLICK, id, fn)
#define EVT_LIST_ITEM_MIDDLE_CLICK(id, fn) wx__DECLARE_LISTEVT(ITEM_MIDDLE_CLICK, id, fn)
#define EVT_LIST_ITEM_ACTIVATED(id, fn) wx__DECLARE_LISTEVT(ITEM_ACTIVATED, id, fn)
#define EVT_LIST_ITEM_FOCUSED(id, fn) wx__DECLARE_LISTEVT(ITEM_FOCUSED, id, fn)
#define EVT_LIST_ITEM_CHECKED(id, fn) wx__DECLARE_LISTEVT(ITEM_CHECKED, id, fn)
#define EVT_LIST_ITEM_UNCHECKED(id, fn) wx__DECLARE_LISTEVT(ITEM_UNCHECKED, id, fn)

#define EVT_LIST_CACHE_HINT(id, fn) wx__DECLARE_LISTEVT(CACHE_HINT, id, fn)

#endif
    // _WX_LISTCTRL_H_BASE_
