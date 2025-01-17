/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/listctrl.h
// Purpose:     wxListCtrl class
// Author:      Julian Smart
// Modified by: Agron Selimaj
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LISTCTRL_H_
#define _WX_LISTCTRL_H_

#include "wx/textctrl.h"

import Utils.Geometry;
import WX.WinDef;

import <string>;
import <vector>;

struct wxMSWListItemData;
class wxMSWListHeaderCustomDraw;


// define this symbol to indicate the availability of SetColumnsOrder() and
// related functions
#define wxHAS_LISTCTRL_COLUMN_ORDER

/*
    The wxListCtrl can show lists of items in four different modes:
    wxLC_LIST:   multicolumn list view, with optional small icons (icons could be
                 optional for some platforms). Columns are computed automatically,
                 i.e. you don't set columns as in wxLC_REPORT. In other words,
                 the list wraps, unlike a wxListBox.
    wxLC_REPORT: single or multicolumn report view (with optional header)
    wxLC_ICON:   large icon view, with optional labels
    wxLC_SMALL_ICON: small icon view, with optional labels

    You can change the style dynamically, either with SetSingleStyle or
    SetWindowStyleFlag.

    Further window styles:

    wxLC_ALIGN_TOP          icons align to the top (default)
    wxLC_ALIGN_LEFT         icons align to the left
    wxLC_AUTOARRANGE        icons arrange themselves
    wxLC_USER_TEXT          the app provides label text on demand, except for column headers
    wxLC_EDIT_LABELS        labels are editable: app will be notified.
    wxLC_NO_HEADER          no header in report mode
    wxLC_NO_SORT_HEADER     can't click on header
    wxLC_SINGLE_SEL         single selection
    wxLC_SORT_ASCENDING     sort ascending (must still supply a comparison callback in SortItems)
    wxLC_SORT_DESCENDING    sort descending (ditto)

    Items are referred to by their index (position in the list starting from zero).

    Label text is supplied via insertion/setting functions and is stored by the
    control, unless the wxLC_USER_TEXT style has been specified, in which case
    the app will be notified when text is required (see sample).

    Images are dealt with by (optionally) associating 3 image lists with the control.
    Zero-based indexes into these image lists indicate which image is to be used for
    which item. Each image in an image list can contain a mask, and can be made out
    of either a bitmap, two bitmaps or an icon. See ImagList.h for more details.

    Notifications are passed via the event system.

    See the sample wxListCtrl app for API usage.

    TODO:
     - addition of further convenience functions
       to avoid use of wxListItem in some functions
     - state/overlay images: probably not needed.
     - testing of whole API, extending current sample.


 */

class wxListCtrl: public wxListCtrlBase
{
public:
    wxListCtrl() = default;

    wxListCtrl(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = wxLC_ICON,
               const wxValidator& validator = {},
               std::string_view name = wxListCtrlNameStr)
    {
        Create(parent, id, pos, size, style, validator, name);
    }

    ~wxListCtrl();

    wxListCtrl& operator=(wxListCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxLC_ICON,
                const wxValidator& validator = {},
                std::string_view name = wxListCtrlNameStr);

    // Set the control colours
    bool SetForegroundColour(const wxColour& col) override;
    bool SetBackgroundColour(const wxColour& col) override;

    // Header attributes
    bool SetHeaderAttr(const wxItemAttr& attr) override;

    // Gets information about this column
    bool GetColumn(int col, wxListItem& item) const override;

    // Sets information about this column
    bool SetColumn(int col, const wxListItem& item) override;

    // Gets the column width
    int GetColumnWidth(int col) const override;

    // Sets the column width
    bool SetColumnWidth(int col, int width) override;


    // Gets the column order from its index or index from its order
    int GetColumnOrder(int col) const override;
    int GetColumnIndexFromOrder(int order) const override;

    // Gets the column order for all columns
    std::vector<int> GetColumnsOrder() const override;

    // Sets the column order for all columns
    bool SetColumnsOrder(const std::vector<int>& orders) override;


    // Gets the number of items that can fit vertically in the
    // visible area of the list control (list or report view)
    // or the total number of items in the list control (icon
    // or small icon view)
    int GetCountPerPage() const;

    // return the total area occupied by all the items (icon/small icon only)
    wxRect GetViewRect() const;

    // Gets the edit control for editing labels.
    wxTextCtrl* GetEditControl() const;

    // Gets information about the item
    bool GetItem(wxListItem& info) const;

    // Check if the item is visible
    bool IsVisible(long item) const override;

    // Sets information about the item
    bool SetItem(wxListItem& info);

    // Sets a string field at a particular column
    bool SetItem(long index, int col, const std::string& label, int imageId = -1);

    // Gets the item state
    ListStateFlags GetItemState(long item, ListStateFlags stateMask) const;

    // Sets the item state
    bool SetItemState(long item, ListStateFlags state, ListStateFlags stateMask);

    // Sets the item image
    bool SetItemImage(long item, int image, int selImage = -1);
    bool SetItemColumnImage(long item, long column, int image);

    // Gets the item text
    std::string GetItemText(long item, int col = 0) const;

    // Sets the item text
    void SetItemText(long item, const std::string& str);

    // Gets the item data
    wxUIntPtr GetItemData(long item) const;

    // Sets the item data
    bool SetItemPtrData(long item, wxUIntPtr data);
    bool SetItemData(long item, long data) { return SetItemPtrData(item, data); }

    // Gets the item rectangle
    bool GetItemRect(long item, wxRect& rect, wxListRectFlags code = wxListRectFlags::Bounds) const;

    // Gets the subitem rectangle in report mode
    bool GetSubItemRect(long item, long subItem, wxRect& rect, wxListRectFlags code = wxListRectFlags::Bounds) const;

    // Gets the item position
    bool GetItemPosition(long item, wxPoint& pos) const;

    // Sets the item position
    bool SetItemPosition(long item, const wxPoint& pos);

    // Gets the number of items in the list control
    int GetItemCount() const override;

    // Gets the number of columns in the list control
    int GetColumnCount() const override { return m_colCount; }

    // get the horizontal and vertical components of the item spacing
    wxSize GetItemSpacing() const;

    // Foreground colour of an item.
    void SetItemTextColour( long item, const wxColour& col);
    wxColour GetItemTextColour( long item ) const;

    // Background colour of an item.
    void SetItemBackgroundColour( long item, const wxColour &col);
    wxColour GetItemBackgroundColour( long item ) const;

    // Font of an item.
    void SetItemFont( long item, const wxFont &f);
    wxFont GetItemFont( long item ) const;

    // Checkbox state of an item
    bool HasCheckBoxes() const override;
    bool EnableCheckBoxes(bool enable = true) override;
    bool IsItemChecked(long item) const override;
    void CheckItem(long item, bool check) override;

    // Gets the number of selected items in the list control
    int GetSelectedItemCount() const;

    // Gets the text colour of the listview
    wxColour GetTextColour() const;

    // Sets the text colour of the listview
    void SetTextColour(const wxColour& col);

    // Gets the index of the topmost visible item when in
    // list or report view
    long GetTopItem() const;

    // Add or remove a single window style
    void SetSingleStyle(unsigned int style, bool add = true);

    // Set the whole window style
    void SetWindowStyleFlag(unsigned int style) override;

    // Searches for an item, starting from 'item'.
    // item can be -1 to find the first item that matches the
    // specified flags.
    // Returns the item or -1 if unsuccessful.
    long GetNextItem(long item, wxListGetNextItem geometry = wxListGetNextItem::All, ListStateFlags state = ListStates::Nil) const;

    // Gets one of the three image lists
    wxImageList *GetImageList(int which) const override;

    // Sets the image list
    void SetImageList(wxImageList *imageList, int which) override;
    void AssignImageList(wxImageList *imageList, int which) override;

    // refresh items selectively (only useful for virtual list controls)
    void RefreshItem(long item);
    void RefreshItems(long itemFrom, long itemTo);

    // Arranges the items
    bool Arrange(wxListAlignment flag = wxListAlignment::Default);

    // Deletes an item
    bool DeleteItem(long item);

    // Deletes all items
    bool DeleteAllItems();

    // Deletes a column
    bool DeleteColumn(int col) override;

    // Deletes all columns
    bool DeleteAllColumns() override;

    // Clears items, and columns if there are any.
    void ClearAll();

    // Edit the label
    wxTextCtrl* EditLabel(long item, wxClassInfo* textControlClass = wxCLASSINFO(wxTextCtrl));

    // End label editing, optionally cancelling the edit
    bool EndEditLabel(bool cancel);

    // Ensures this item is visible
    bool EnsureVisible(long item);

    // Find an item whose label matches this string, starting from the item after 'start'
    // or the beginning if 'start' is -1.
    long FindItem(long start, const std::string& str, bool partial = false);

    // Find an item whose data matches this data, starting from the item after 'start'
    // or the beginning if 'start' is -1.
    long FindItem(long start, wxUIntPtr data);

    // Find an item nearest this position in the specified direction, starting from
    // the item after 'start' or the beginning if 'start' is -1.
    long FindItem(long start, const wxPoint& pt, wxListFind direction);

    // Determines which item (if any) is at the specified point,
    // giving details in 'flags' (see ListHitTestFlags above)
    // Request the subitem number as well at the given coordinate.
    long HitTest(const wxPoint& point, ListHitTestFlags& flags, long* ptrSubItem = nullptr) const;

    // Inserts an item, returning the index of the new item if successful,
    // -1 otherwise.
    long InsertItem(const wxListItem& info);

    // Insert a string item
    long InsertItem(long index, const std::string& label);

    // Insert an image item
    long InsertItem(long index, int imageIndex);

    // Insert an image/string item
    long InsertItem(long index, const std::string& label, int imageIndex);

    // set the number of items in a virtual list control
    void SetItemCount(long count);

    // Scrolls the list control. If in icon, small icon or report view mode,
    // x specifies the number of pixels to scroll. If in list view mode, x
    // specifies the number of columns to scroll.
    // If in icon, small icon or list view mode, y specifies the number of pixels
    // to scroll. If in report view mode, y specifies the number of lines to scroll.
    bool ScrollList(int dx, int dy);

    // Sort items.

    // fn is a function which takes 3 long arguments: item1, item2, data.
    // item1 is the long data associated with a first item (NOT the index).
    // item2 is the long data associated with a second item (NOT the index).
    // data is the same value as passed to SortItems.
    // The return value is a negative number if the first item should precede the second
    // item, a positive number of the second item should precede the first,
    // or zero if the two items are equivalent.

    // data is arbitrary data to be passed to the sort function.
    bool SortItems(wxListCtrlCompare fn, wxIntPtr data);

    // IMPLEMENTATION
    bool MSWCommand(WXUINT param, WXWORD id) override;
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;
    bool MSWShouldPreProcessMessage(WXMSG* msg) override;

    // Necessary for drawing hrules and vrules, if specified
    void OnPaint(wxPaintEvent& event);

    // Override SetDoubleBuffered() to do nothing, its implementation in the
    // base class is incompatible with the double buffering done by this native
    // control.
    bool IsDoubleBuffered() const override;
    void SetDoubleBuffered(bool on) override;

    bool ShouldInheritColours() const override { return false; }

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal);

    // convert our styles to Windows
    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

    // special Windows message handling
    WXLRESULT MSWWindowProc(WXUINT nMsg,
                                    WXWPARAM wParam,
                                    WXLPARAM lParam) override;

protected:
    bool MSWShouldSetDefaultFont() const override { return false; }

    // Implement constrained best size calculation.
    int DoGetBestClientHeight(int width) const override
        { return MSWGetBestViewRect(width, -1).y; }
    int DoGetBestClientWidth(int height) const override
        { return MSWGetBestViewRect(-1, height).x; }
#if wxUSE_TOOLTIPS
    void DoSetToolTip(wxToolTip *tip) override;
#endif // wxUSE_TOOLTIPS

    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    void OnDPIChanged(wxDPIChangedEvent& event);

    wxSize MSWGetBestViewRect(int x, int y) const;

    // Implement base class pure virtual methods.
    long DoInsertColumn(long col, const wxListItem& info) override;

    // free memory taken by all internal data
    void FreeAllInternalData();

    // all wxMSWListItemData objects we use
    std::vector<wxMSWListItemData *> m_internalData;

private:
    // Object using for header custom drawing if necessary, may be NULL.
    wxMSWListHeaderCustomDraw* m_headerCustomDraw{nullptr};

// FIXME: Make all variables private
protected:
    // get the internal data object for this item (may return NULL)
    wxMSWListItemData *MSWGetItemData(long item) const;

    // get the item attribute, either by querying it for virtual control, or by
    // returning the one previously set using setter methods for a normal one
    wxItemAttr *DoGetItemColumnAttr(long item, long column) const;


    wxTextCtrl*       m_textCtrl{nullptr};        // The control used for editing a label
    wxImageList *     m_imageListNormal{nullptr}; // The image list for normal icons
    wxImageList *     m_imageListSmall{nullptr};  // The image list for small icons
    wxImageList *     m_imageListState{nullptr};  // The image list state icons (not implemented yet)

    int               m_colCount{0};   // Windows doesn't have GetColumnCount so must
                                    // keep track of inserted/deleted columns

    bool              m_ownsImageListNormal{false};
    bool              m_ownsImageListSmall{false};
    bool              m_ownsImageListState{false};

    // true if we have any items with custom attributes
    bool m_hasAnyAttr{false};

private:
    // process NM_CUSTOMDRAW notification message
    WXLPARAM OnCustomDraw(WXLPARAM lParam);

    // set the extended styles for the control (used by Create() and
    // UpdateStyle()), only should be called if InReportView()
    void MSWSetExListStyles();

    // initialize the (already created) m_textCtrl with the associated WXHWND
    void InitEditControl(WXHWND hWnd);

    // destroy m_textCtrl if it's currently valid and reset it to NULL
    void DeleteEditControl();

    // Intercept Escape and Enter keys to avoid them being stolen from our
    // in-place editor control.
    void OnCharHook(wxKeyEvent& event);

    wxDECLARE_DYNAMIC_CLASS(wxListCtrl);
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_LISTCTRL_H_
