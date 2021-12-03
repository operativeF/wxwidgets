/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/dataview.h
// Purpose:     wxDataViewCtrl generic implementation header
// Author:      Robert Roebling
// Modified By: Bo Yang
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GENERICDATAVIEWCTRLH__
#define __GENERICDATAVIEWCTRLH__

#include "wx/object.h"
#include "wx/control.h"
#include "wx/scrolwin.h"
#include "wx/icon.h"
#if wxUSE_ACCESSIBILITY
    #include "wx/access.h"
#endif // wxUSE_ACCESSIBILITY

import WX.Cfg.Flags;

import <string>;
import <vector>;

class wxDataViewMainWindow;
class wxDataViewHeaderWindow;
#if wxUSE_ACCESSIBILITY
class wxDataViewCtrlAccessible;
#endif // wxUSE_ACCESSIBILITY

// ---------------------------------------------------------
// wxDataViewColumn
// ---------------------------------------------------------

class wxDataViewColumn : public wxDataViewColumnBase
{
public:
    wxDataViewColumn(const std::string& title,
                     wxDataViewRenderer *renderer,
                     unsigned int model_column,
                     int width = wxDVC_DEFAULT_WIDTH,
                     wxAlignment align = wxALIGN_CENTER,
                     unsigned int flags = wxDATAVIEW_COL_RESIZABLE)
        : wxDataViewColumnBase(renderer, model_column),
          m_title{title},
          m_width{width},
          m_manuallySetWidth{width},
          m_align{align},
          m_flags{flags}
    {
    }

    wxDataViewColumn(const wxBitmap& bitmap,
                     wxDataViewRenderer *renderer,
                     unsigned int model_column,
                     int width = wxDVC_DEFAULT_WIDTH,
                     wxAlignment align = wxALIGN_CENTER,
                     unsigned int flags = wxDATAVIEW_COL_RESIZABLE)
        : wxDataViewColumnBase(bitmap, renderer, model_column),
          m_width{width},
          m_manuallySetWidth{width},
          m_align{align},
          m_flags{flags}
    {
    }

    // implement wxHeaderColumnBase methods
    void SetTitle(const std::string& title) override
    {
        m_title = title;
        UpdateWidth();
    }
    
    std::string GetTitle() const override
    {
        return m_title;
    }

    void SetWidth(int width) override
    {
        // Call the actual update method, used for both automatic and "manual"
        // width changes.
        WXUpdateWidth(width);

        // Do remember the last explicitly set width: this is used to prevent
        // UpdateColumnSizes() from resizing the last column to be smaller than
        // this size.
        m_manuallySetWidth = width;
    }
    int GetWidth() const override;

    void SetMinWidth(int minWidth) override
    {
        m_minWidth = minWidth;
        UpdateWidth();
    }
    int GetMinWidth() const override
    {
        return m_minWidth;
    }

    void SetAlignment(wxAlignment align) override
    {
        m_align = align;
        UpdateDisplay();
    }
    wxAlignment GetAlignment() const override
    {
        return m_align;
    }

    void SetFlags(unsigned int flags) override
    {
        m_flags = flags;
        UpdateDisplay();
    }
    unsigned int GetFlags() const override
    {
        return m_flags;
    }

    bool IsSortKey() const override
    {
        return m_sort;
    }

    void UnsetAsSortKey() override;

    void SetSortOrder(bool ascending) override;

    bool IsSortOrderAscending() const override
    {
        return m_sortAscending;
    }

    void SetBitmap( const wxBitmap& bitmap ) override
    {
        wxDataViewColumnBase::SetBitmap(bitmap);
        UpdateWidth();
    }

    // This method is specific to the generic implementation and is used only
    // by wxWidgets itself.
    void WXUpdateWidth(int width)
    {
        if ( width == m_width )
            return;

        m_width = width;
        UpdateWidth();
    }

    // This method is also internal and called when the column is resized by
    // user interactively.
    void WXOnResize(int width);

    int WXGetSpecifiedWidth() const override;

private:
    // These methods forward to wxDataViewCtrl::OnColumnChange() and
    // OnColumnWidthChange() respectively, i.e. the latter is stronger than the
    // former.
    void UpdateDisplay();
    void UpdateWidth();

    // Return the effective value corresponding to the given width, handling
    // its negative values such as wxCOL_WIDTH_DEFAULT.
    int DoGetEffectiveWidth(int width) const;

    std::string m_title;

    int m_width;
    int m_manuallySetWidth;
    int m_minWidth{0};

    unsigned int m_flags;

    wxAlignment m_align;

    bool m_sort{false};
    bool m_sortAscending{true};

    friend class wxDataViewHeaderWindowBase;
    friend class wxDataViewHeaderWindow;
    friend class wxDataViewHeaderWindowMSW;
};

// ---------------------------------------------------------
// wxDataViewCtrl
// ---------------------------------------------------------

class wxDataViewCtrl : public wxDataViewCtrlBase,
                       public wxScrollHelper
{
    friend class wxDataViewMainWindow;
    friend class wxDataViewHeaderWindowBase;
    friend class wxDataViewHeaderWindow;
    friend class wxDataViewHeaderWindowMSW;
    friend class wxDataViewColumn;
#if wxUSE_ACCESSIBILITY
    friend class wxDataViewCtrlAccessible;
#endif // wxUSE_ACCESSIBILITY

public:
    wxDataViewCtrl() : wxScrollHelper(this)
    {
    }

    wxDataViewCtrl( wxWindow *parent, wxWindowID id,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, unsigned int style = 0,
           const wxValidator& validator = wxDefaultValidator,
           const std::string& name = wxDataViewCtrlNameStr)
             : wxScrollHelper(this)
    {
        Create(parent, id, pos, size, style, validator, name);
    }
    
    wxDataViewCtrl(const wxDataViewCtrl&) = delete;
	wxDataViewCtrl& operator=(const wxDataViewCtrl&) = delete;

    ~wxDataViewCtrl();

    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxDataViewCtrlNameStr);

    bool AssociateModel( wxDataViewModel *model ) override;

    bool AppendColumn( wxDataViewColumn *col ) override;
    bool PrependColumn( wxDataViewColumn *col ) override;
    bool InsertColumn( unsigned int pos, wxDataViewColumn *col ) override;

    void DoSetExpanderColumn() override;
    void DoSetIndent() override;

    unsigned int GetColumnCount() const override;
    wxDataViewColumn* GetColumn( unsigned int pos ) const override;
    bool DeleteColumn( wxDataViewColumn *column ) override;
    bool ClearColumns() override;
    int GetColumnPosition( const wxDataViewColumn *column ) const override;

    wxDataViewColumn *GetSortingColumn() const override;
    std::vector<wxDataViewColumn *> GetSortingColumns() const override;

    wxDataViewItem GetTopItem() const override;
    int GetCountPerPage() const override;

    int GetSelectedItemsCount() const override;
    int GetSelections( wxDataViewItemArray & sel ) const override;
    void SetSelections( const wxDataViewItemArray & sel ) override;
    void Select( const wxDataViewItem & item ) override;
    void Unselect( const wxDataViewItem & item ) override;
    bool IsSelected( const wxDataViewItem & item ) const override;

    void SelectAll() override;
    void UnselectAll() override;

    void EnsureVisible( const wxDataViewItem & item,
                                const wxDataViewColumn *column = nullptr ) override;
    void HitTest( const wxPoint & point, wxDataViewItem & item,
                          wxDataViewColumn* &column ) const override;
    wxRect GetItemRect( const wxDataViewItem & item,
                                const wxDataViewColumn *column = nullptr ) const override;

    bool SetRowHeight( int rowHeight ) override;

    void Collapse( const wxDataViewItem & item ) override;
    bool IsExpanded( const wxDataViewItem & item ) const override;

    void SetFocus() override;

    bool SetFont(const wxFont & font) override;

#if wxUSE_ACCESSIBILITY
    bool Show(bool show = true) override;
    void SetName(const std::string& name) override;
    bool Reparent(wxWindowBase *newParent) override;
#endif // wxUSE_ACCESSIBILITY
    bool Enable(bool enable = true) override;

    bool AllowMultiColumnSort(bool allow) override;
    bool IsMultiColumnSortAllowed() const override { return m_allowMultiColumnSort; }
    void ToggleSortByColumn(int column) override;

#if wxUSE_DRAG_AND_DROP
    bool EnableDragSource( const wxDataFormat &format ) override;
    bool EnableDropTarget( const wxDataFormat &format ) override;
#endif // wxUSE_DRAG_AND_DROP

    wxBorder GetDefaultBorder() const override;

    void EditItem(const wxDataViewItem& item, const wxDataViewColumn *column) override;

    bool SetHeaderAttr(const wxItemAttr& attr) override;

    bool SetAlternateRowColour(const wxColour& colour) override;

    // This method is specific to generic wxDataViewCtrl implementation and
    // should not be used in portable code.
    wxColour GetAlternateRowColour() const { return m_alternateRowColour; }

    // The returned pointer is null if the control has wxDV_NO_HEADER style.
    //
    // This method is only available in the generic versions.
    wxHeaderCtrl* GenericGetHeader() const;

protected:
    void EnsureVisibleRowCol( int row, int column );

    // Notice that row here may be invalid (i.e. >= GetRowCount()), this is not
    // an error and this function simply returns an invalid item in this case.
    wxDataViewItem GetItemByRow( unsigned int row ) const;
    int GetRowByItem( const wxDataViewItem & item ) const;

    // Mark the column as being used or not for sorting.
    void UseColumnForSorting(int idx);
    void DontUseColumnForSorting(int idx);

    // Return true if the given column is sorted
    bool IsColumnSorted(int idx) const;

    // Reset all columns currently used for sorting.
    void ResetAllSortColumns();

    void DoEnableSystemTheme(bool enable, wxWindow* window) override;

    void OnDPIChanged(wxDPIChangedEvent& event);

public:     // utility functions not part of the API

    // returns the "best" width for the idx-th column
    unsigned int GetBestColumnWidth(int idx) const;

    // called by header window after reorder
    void ColumnMoved( wxDataViewColumn* col, unsigned int new_pos );

    // update the display after a change to an individual column
    void OnColumnChange(unsigned int idx);

    // update after the column width changes due to interactive resizing
    void OnColumnResized();

    // update after the column width changes because of e.g. title or bitmap
    // change, invalidates the column best width and calls OnColumnChange()
    void OnColumnWidthChange(unsigned int idx);

    // update after a change to the number of columns
    void OnColumnsCountChanged();

    wxWindow *GetMainWindow() { return (wxWindow*) m_clientArea; }

    // return the index of the given column in m_cols
    int GetColumnIndex(const wxDataViewColumn *column) const;

    // Return the index of the column having the given model index.
    int GetModelColumnIndex(unsigned int model_column) const;

    // return the column displayed at the given position in the control
    wxDataViewColumn *GetColumnAt(unsigned int pos) const;

    wxDataViewColumn *GetCurrentColumn() const override;

    void OnInternalIdle() override;

#if wxUSE_ACCESSIBILITY
    wxAccessible* CreateAccessible() override;
#endif // wxUSE_ACCESSIBILITY

private:
    wxDataViewItem DoGetCurrentItem() const override;
    void DoSetCurrentItem(const wxDataViewItem& item) override;

    void DoExpand(const wxDataViewItem& item, bool expandChildren) override;

    void InvalidateColBestWidths();
    void InvalidateColBestWidth(int idx);
    void UpdateColWidths();

    void DoClearColumns();

    // cached column best widths information, values are for
    // respective columns from m_cols and the arrays have same size
    struct CachedColWidthInfo
    {
        int width{0};  // cached width or 0 if not computed
        bool dirty{true}; // column was invalidated, header needs updating
    };

    // user defined color to draw row lines, may be invalid
    wxColour m_alternateRowColour;

    // columns indices used for sorting, empty if nothing is sorted
    std::vector<int> m_sortingColumnIdxs;
    std::vector<CachedColWidthInfo> m_colsBestWidths;
    std::vector<wxDataViewColumn*> m_cols;

    wxDataViewModelNotifier  *m_notifier{nullptr};
    wxDataViewMainWindow     *m_clientArea{nullptr};
    wxDataViewHeaderWindow   *m_headerArea{nullptr};

    // if true, allow sorting by more than one column
    bool m_allowMultiColumnSort{false};

    // This indicates that at least one entry in m_colsBestWidths has 'dirty'
    // flag set. It's cheaper to check one flag in OnInternalIdle() than to
    // iterate over m_colsBestWidths to check if anything needs to be done.
    bool                      m_colsDirty{false};

private:
    void OnSize( wxSizeEvent &event );
    wxSize GetSizeAvailableForScrollTarget(const wxSize& size) override;

    // we need to return a special WM_GETDLGCODE value to process just the
    // arrows but let the other navigation characters through
#ifdef WX_WINDOWS
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
#endif // WX_WINDOWS

    WX_FORWARD_TO_SCROLL_HELPER()

private:
    wxDECLARE_EVENT_TABLE();
};

#if wxUSE_ACCESSIBILITY
//-----------------------------------------------------------------------------
// wxDataViewCtrlAccessible
//-----------------------------------------------------------------------------

class wxDataViewCtrlAccessible: public wxWindowAccessible
{
public:
    wxDataViewCtrlAccessible(wxDataViewCtrl* win);

    wxAccStatus HitTest(const wxPoint& pt, int* childId,
                                wxAccessible** childObject) override;

    wxAccStatus GetLocation(wxRect& rect, int elementId) override;

    wxAccStatus Navigate(wxNavDir navDir, int fromId,
                                 int* toId, wxAccessible** toObject) override;

    wxAccStatus GetName(int childId, std::string* name) override;

    wxAccStatus GetChildCount(int* childCount) override;

    wxAccStatus GetChild(int childId, wxAccessible** child) override;

    // wxWindowAccessible::GetParent() implementation is enough.
    // wxAccStatus GetParent(wxAccessible** parent) override;

    wxAccStatus DoDefaultAction(int childId) override;

    wxAccStatus GetDefaultAction(int childId, std::string* actionName) override;

    wxAccStatus GetDescription(int childId, std::string* description) override;

    wxAccStatus GetHelpText(int childId, std::string* helpText) override;

    wxAccStatus GetKeyboardShortcut(int childId, std::string* shortcut) override;

    wxAccStatus GetRole(int childId, wxAccSystemRole* role) override;

    wxAccStatus GetState(int childId, unsigned int* state) override;

    wxAccStatus GetValue(int childId, std::string* strValue) override;

    wxAccStatus Select(int childId, wxAccSelectionFlags selectFlags) override;

    wxAccStatus GetFocus(int* childId, wxAccessible** child) override;

    wxAccStatus GetSelections(wxVariant* selections) override;
};
#endif // wxUSE_ACCESSIBILITY

#endif // __GENERICDATAVIEWCTRLH__
