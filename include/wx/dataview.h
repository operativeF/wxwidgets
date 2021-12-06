/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dataview.h
// Purpose:     wxDataViewCtrl base classes
// Author:      Robert Roebling
// Modified by: Bo Yang
// Created:     08.01.06
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DATAVIEW_H_BASE_
#define _WX_DATAVIEW_H_BASE_

#if wxUSE_DATAVIEWCTRL

#include "wx/headercol.h"
#include "wx/variant.h"
#include "wx/dnd.h"             // For wxDragResult declaration only.
#include "wx/icon.h"
#include "wx/itemid.h"
#include "wx/weakref.h"
#include "wx/dataobj.h"
#include "wx/withimages.h"
#include "wx/systhemectrl.h"

import Utils.Geometry;

import <string>;
import <vector>;

import WX.Utils.Cast;

class wxImageList;
class wxItemAttr;
class wxHeaderCtrl;

#if wxUSE_NATIVE_DATAVIEWCTRL && !defined(__WXUNIVERSAL__)
    #if defined(__WXGTK20__) || defined(__WXOSX__)
        #define wxHAS_NATIVE_DATAVIEWCTRL
    #endif
#endif

#ifndef wxHAS_NATIVE_DATAVIEWCTRL
    #define wxHAS_GENERIC_DATAVIEWCTRL
#endif

// ----------------------------------------------------------------------------
// wxDataViewCtrl globals
// ----------------------------------------------------------------------------

class wxDataViewModel;
class wxDataViewCtrl;
class wxDataViewColumn;
class wxDataViewRenderer;
class wxDataViewModelNotifier;
#if wxUSE_ACCESSIBILITY
class wxDataViewCtrlAccessible;
#endif // wxUSE_ACCESSIBILITY

inline constexpr char wxDataViewCtrlNameStr[] = "dataviewCtrl";

// ----------------------------------------------------------------------------
// wxDataViewCtrl flags
// ----------------------------------------------------------------------------

// size of a wxDataViewRenderer without contents:
inline constexpr wxSize wxDVC_DEFAULT_RENDERER_SIZE = {20, 20};

// the default width of new (text) columns:
inline constexpr unsigned int wxDVC_DEFAULT_WIDTH = 80;

// the default width of new toggle columns:
inline constexpr unsigned int wxDVC_TOGGLE_DEFAULT_WIDTH = 30;

// the default minimal width of the columns:
inline constexpr unsigned int wxDVC_DEFAULT_MINWIDTH = 30;

// The default alignment of wxDataViewRenderers is to take
// the alignment from the column it owns.
inline constexpr unsigned int wxDVR_DEFAULT_ALIGNMENT = -1;


// ---------------------------------------------------------
// wxDataViewItem
// ---------------------------------------------------------

// Make it a class and not a typedef to allow forward declaring it.
class wxDataViewItem : public wxItemId<void*>
{
public:
    wxDataViewItem()  = default;
    explicit wxDataViewItem(void* pItem) : wxItemId<void*>(pItem) { }
};

using wxDataViewItemArray = std::vector<wxDataViewItem>;

// ---------------------------------------------------------
// wxDataViewModelNotifier
// ---------------------------------------------------------

class wxDataViewModelNotifier
{
public:
    wxDataViewModelNotifier() { m_owner = nullptr; }
    virtual ~wxDataViewModelNotifier() { m_owner = nullptr; }

    virtual bool ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item ) = 0;
    virtual bool ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item ) = 0;
    virtual bool ItemChanged( const wxDataViewItem &item ) = 0;
    virtual bool ItemsAdded( const wxDataViewItem &parent, const wxDataViewItemArray &items );
    virtual bool ItemsDeleted( const wxDataViewItem &parent, const wxDataViewItemArray &items );
    virtual bool ItemsChanged( const wxDataViewItemArray &items );
    virtual bool ValueChanged( const wxDataViewItem &item, unsigned int col ) = 0;
    virtual bool Cleared() = 0;

    // some platforms, such as GTK+, may need a two step procedure for ::Reset()
    virtual bool BeforeReset() { return true; }
    virtual bool AfterReset() { return Cleared(); }

    virtual void Resort() = 0;

    void SetOwner( wxDataViewModel *owner ) { m_owner = owner; }
    wxDataViewModel *GetOwner() const       { return m_owner; }

private:
    wxDataViewModel *m_owner;
};



// ----------------------------------------------------------------------------
// wxDataViewItemAttr: a structure containing the visual attributes of an item
// ----------------------------------------------------------------------------

// TODO: Merge with wxItemAttr somehow.

class wxDataViewItemAttr
{
public:
    void SetColour(const wxColour& colour) { m_colour = colour; }
    void SetBold( bool set ) { m_bold = set; }
    void SetItalic( bool set ) { m_italic = set; }
    void SetStrikethrough( bool set ) { m_strikethrough = set; }
    void SetBackgroundColour(const wxColour& colour)  { m_bgColour = colour; }

    bool HasColour() const { return m_colour.IsOk(); }
    const wxColour& GetColour() const { return m_colour; }

    bool HasFont() const { return m_bold || m_italic || m_strikethrough; }
    bool GetBold() const { return m_bold; }
    bool GetItalic() const { return m_italic; }
    bool GetStrikethrough() const { return m_strikethrough; }

    bool HasBackgroundColour() const { return m_bgColour.IsOk(); }
    const wxColour& GetBackgroundColour() const { return m_bgColour; }

    bool IsDefault() const { return !(HasColour() || HasFont() || HasBackgroundColour()); }

    // Return the font based on the given one with this attribute applied to it.
    wxFont GetEffectiveFont(const wxFont& font) const;

private:
    wxColour m_colour;
    bool     m_bold{false};
    bool     m_italic{false};
    bool     m_strikethrough{false};
    wxColour m_bgColour;
};


// ---------------------------------------------------------
// wxDataViewModel
// ---------------------------------------------------------

using wxDataViewModelNotifiers = std::vector<wxDataViewModelNotifier *>;

class wxDataViewModel: public wxRefCounter
{
public:
    virtual unsigned int GetColumnCount() const = 0;

    // return type as reported by wxVariant
    virtual std::string GetColumnType( unsigned int col ) const = 0;

    // get value into a wxVariant
    virtual void GetValue( wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) const = 0;

    // return true if the given item has a value to display in the given
    // column: this is always true except for container items which by default
    // only show their label in the first column (but see HasContainerColumns())
    virtual bool HasValue(const wxDataViewItem& item, unsigned col) const
    {
        return col == 0 || !IsContainer(item) || HasContainerColumns(item);
    }

    // usually ValueChanged() should be called after changing the value in the
    // model to update the control, ChangeValue() does it on its own while
    // SetValue() does not -- so while you will override SetValue(), you should
    // be usually calling ChangeValue()
    virtual bool SetValue(const wxVariant &variant,
                          const wxDataViewItem &item,
                          unsigned int col) = 0;

    bool ChangeValue(const wxVariant& variant,
                     const wxDataViewItem& item,
                     unsigned int col)
    {
        return SetValue(variant, item, col) && ValueChanged(item, col);
    }

    // Get text attribute, return false of default attributes should be used
    virtual bool GetAttr([[maybe_unused]] const wxDataViewItem &item,
                         [[maybe_unused]] unsigned int col,
                         [[maybe_unused]] wxDataViewItemAttr &attr) const
    {
        return false;
    }

    // Override this if you want to disable specific items
    virtual bool IsEnabled([[maybe_unused]] const wxDataViewItem &item,
                           [[maybe_unused]] unsigned int col) const
    {
        return true;
    }

    // define hierarchy
    virtual wxDataViewItem GetParent( const wxDataViewItem &item ) const = 0;
    virtual bool IsContainer( const wxDataViewItem &item ) const = 0;
    // Is the container just a header or an item with all columns
    virtual bool HasContainerColumns([[maybe_unused]] const wxDataViewItem& item) const
        { return false; }
    virtual unsigned int GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const = 0;

    // delegated notifiers
    bool ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item );
    bool ItemsAdded( const wxDataViewItem &parent, const wxDataViewItemArray &items );
    bool ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item );
    bool ItemsDeleted( const wxDataViewItem &parent, const wxDataViewItemArray &items );
    bool ItemChanged( const wxDataViewItem &item );
    bool ItemsChanged( const wxDataViewItemArray &items );
    bool ValueChanged( const wxDataViewItem &item, unsigned int col );
    bool Cleared();

    // some platforms, such as GTK+, may need a two step procedure for ::Reset()
    bool BeforeReset();
    bool AfterReset();


    // delegated action
    virtual void Resort();

    void AddNotifier( wxDataViewModelNotifier *notifier );
    void RemoveNotifier( wxDataViewModelNotifier *notifier );

    // default compare function
    virtual int Compare( const wxDataViewItem &item1, const wxDataViewItem &item2,
                         unsigned int column, bool ascending ) const;
    virtual bool HasDefaultCompare() const { return false; }

    // internal
    virtual bool IsListModel() const { return false; }
    virtual bool IsVirtualListModel() const { return false; }

protected:
    // Dtor is protected because the objects of this class must not be deleted,
    // DecRef() must be used instead.
    ~wxDataViewModel();

    // Helper function used by the default Compare() implementation to compare
    // values of types it is not aware about. Can be overridden in the derived
    // classes that use columns of custom types.
    virtual int DoCompareValues([[maybe_unused]] const wxVariant& value1,
                                [[maybe_unused]] const wxVariant& value2) const
    {
        return 0;
    }

private:
    wxDataViewModelNotifiers  m_notifiers;
};

// ----------------------------------------------------------------------------
// wxDataViewListModel: a model of a list, i.e. flat data structure without any
//      branches/containers, used as base class by wxDataViewIndexListModel and
//      wxDataViewVirtualListModel
// ----------------------------------------------------------------------------

class wxDataViewListModel : public wxDataViewModel
{
public:
    // derived classes should override these methods instead of
    // {Get,Set}Value() and GetAttr() inherited from the base class

    virtual void GetValueByRow(wxVariant &variant,
                               unsigned row, unsigned col) const = 0;

    virtual bool SetValueByRow(const wxVariant &variant,
                               unsigned row, unsigned col) = 0;

    virtual bool
    GetAttrByRow([[maybe_unused]] unsigned row, [[maybe_unused]] unsigned col,
                 [[maybe_unused]] wxDataViewItemAttr &attr) const
    {
        return false;
    }

    virtual bool IsEnabledByRow([[maybe_unused]] unsigned int row,
                                [[maybe_unused]] unsigned int col) const
    {
        return true;
    }


    // helper methods provided by list models only
    virtual unsigned GetRow( const wxDataViewItem &item ) const = 0;

    // returns the number of rows
    virtual unsigned int GetCount() const = 0;

    // implement some base class pure virtual directly
    wxDataViewItem
    GetParent( const [[maybe_unused]] wxDataViewItem & item ) const override
    {
        // items never have valid parent in this model
        return wxDataViewItem();
    }

    bool IsContainer( const wxDataViewItem &item ) const override
    {
        // only the invisible (and invalid) root item has children
        return !item.IsOk();
    }

    // and implement some others by forwarding them to our own ones
    void GetValue( wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) const override
    {
        GetValueByRow(variant, GetRow(item), col);
    }

    bool SetValue( const wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) override
    {
        return SetValueByRow( variant, GetRow(item), col );
    }

    bool GetAttr(const wxDataViewItem &item, unsigned int col,
                         wxDataViewItemAttr &attr) const override
    {
        return GetAttrByRow( GetRow(item), col, attr );
    }

    bool IsEnabled(const wxDataViewItem &item, unsigned int col) const override
    {
        return IsEnabledByRow( GetRow(item), col );
    }


    bool IsListModel() const override { return true; }
};

// ---------------------------------------------------------
// wxDataViewIndexListModel
// ---------------------------------------------------------

class wxDataViewIndexListModel: public wxDataViewListModel
{
public:
    wxDataViewIndexListModel( unsigned int initial_size = 0 );

    void RowPrepended();
    void RowInserted( unsigned int before );
    void RowAppended();
    void RowDeleted( unsigned int row );
    void RowsDeleted( const std::vector<int> &rows );
    void RowChanged( unsigned int row );
    void RowValueChanged( unsigned int row, unsigned int col );
    void Reset( unsigned int new_size );

    // convert to/from row/wxDataViewItem

    unsigned GetRow( const wxDataViewItem &item ) const override;
    wxDataViewItem GetItem( unsigned int row ) const;

    // implement base methods
    unsigned int GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const override;

    unsigned int GetCount() const override { return (unsigned int)m_hash.size(); }

private:
    wxDataViewItemArray m_hash;
    unsigned int m_nextFreeID;

    // IDs are ordered until an item gets deleted or inserted
    bool m_ordered{true};
};

// ---------------------------------------------------------
// wxDataViewVirtualListModel
// ---------------------------------------------------------

#ifdef __WXMAC__
// better than nothing
using wxDataViewVirtualListModel = wxDataViewIndexListModel;
#else

class wxDataViewVirtualListModel: public wxDataViewListModel
{
public:
    wxDataViewVirtualListModel( unsigned int initial_size = 0 );

    void RowPrepended();
    void RowInserted( unsigned int before );
    void RowAppended();
    void RowDeleted( unsigned int row );
    void RowsDeleted( const std::vector<int> &rows );
    void RowChanged( unsigned int row );
    void RowValueChanged( unsigned int row, unsigned int col );
    void Reset( unsigned int new_size );

    // convert to/from row/wxDataViewItem

    unsigned GetRow( const wxDataViewItem &item ) const override;
    wxDataViewItem GetItem( unsigned int row ) const;

    // compare based on index

    int Compare( const wxDataViewItem &item1, const wxDataViewItem &item2,
                         unsigned int column, bool ascending ) const override;
    bool HasDefaultCompare() const override;

    // implement base methods
    unsigned int GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const override;

    unsigned int GetCount() const override { return m_size; }

    // internal
    bool IsVirtualListModel() const override { return true; }

private:
    unsigned int m_size;
};
#endif

// ----------------------------------------------------------------------------
// wxDataViewRenderer and related classes
// ----------------------------------------------------------------------------

#include "wx/dvrenderers.h"

// ---------------------------------------------------------
// wxDataViewColumnBase
// ---------------------------------------------------------

// for compatibility only, do not use
enum wxDataViewColumnFlags
{
    wxDATAVIEW_COL_RESIZABLE     = wxCOL_RESIZABLE,
    wxDATAVIEW_COL_SORTABLE      = wxCOL_SORTABLE,
    wxDATAVIEW_COL_REORDERABLE   = wxCOL_REORDERABLE,
    wxDATAVIEW_COL_HIDDEN        = wxCOL_HIDDEN
};

class wxDataViewColumnBase : public wxSettableHeaderColumn
{
public:
    // ctor for the text columns: takes ownership of renderer
    wxDataViewColumnBase(wxDataViewRenderer *renderer,
                         int model_column)
        : m_model_column{model_column}
    {
        m_renderer = renderer;
        // FIXME: This cast is BS.
        m_renderer->SetOwner((wxDataViewColumn*) this);
    }

    // ctor for the bitmap columns
    wxDataViewColumnBase(const wxBitmap& bitmap,
                         wxDataViewRenderer *renderer,
                         int model_column)
        : wxDataViewColumnBase{renderer, model_column}
    {
        m_bitmap = bitmap;
    }

    ~wxDataViewColumnBase();

    // setters:
    virtual void SetOwner( wxDataViewCtrl *owner )
        { m_owner = owner; }

    // getters:
    unsigned int GetModelColumn() const { return wx::narrow_cast<unsigned int>(m_model_column); }
    wxDataViewCtrl *GetOwner() const        { return m_owner; }
    wxDataViewRenderer* GetRenderer() const { return m_renderer; }

    // implement some of base class pure virtuals (the rest is port-dependent
    // and done differently in generic and native versions)
    void SetBitmap( const wxBitmap& bitmap ) override { m_bitmap = bitmap; }
    wxBitmap GetBitmap() const override { return m_bitmap; }

    // Special accessor for use by wxWidgets only returning the width that was
    // explicitly set, either by the application, using SetWidth(), or by the
    // user, resizing the column interactively. It is usually the same as
    // GetWidth(), but can be different for the last column.
    virtual int WXGetSpecifiedWidth() const { return GetWidth(); }

protected:
    wxBitmap                 m_bitmap;

    wxDataViewRenderer      *m_renderer;
    wxDataViewCtrl          *m_owner{nullptr};

    int                      m_model_column;
};

// ---------------------------------------------------------
// wxDataViewCtrlBase
// ---------------------------------------------------------

inline constexpr unsigned int wxDV_SINGLE                  = 0x0000;     // for convenience
inline constexpr unsigned int wxDV_MULTIPLE                = 0x0001;     // can select multiple items
inline constexpr unsigned int wxDV_NO_HEADER               = 0x0002;     // column titles not visible
inline constexpr unsigned int wxDV_HORIZ_RULES             = 0x0004;     // light horizontal rules between rows
inline constexpr unsigned int wxDV_VERT_RULES              = 0x0008;     // light vertical rules between columns
inline constexpr unsigned int wxDV_ROW_LINES               = 0x0010;     // alternating colour in rows
inline constexpr unsigned int wxDV_VARIABLE_LINE_HEIGHT    = 0x0020;     // variable line height

class wxDataViewCtrlBase: public wxSystemThemedControl<wxControl>
{
public:
    ~wxDataViewCtrlBase();

    wxDataViewCtrlBase& operator=(wxDataViewCtrlBase&&) = delete;

    // model
    // -----

    virtual bool AssociateModel( wxDataViewModel *model );
    wxDataViewModel* GetModel();
    const wxDataViewModel* GetModel() const;


    // column management
    // -----------------

    wxDataViewColumn *PrependTextColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependIconTextColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependToggleColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_TOGGLE_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependProgressColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependDateColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Activatable, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependBitmapColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependTextColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependIconTextColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependToggleColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_TOGGLE_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependProgressColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependDateColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Activatable, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *PrependBitmapColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );

    wxDataViewColumn *AppendTextColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendIconTextColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendToggleColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_TOGGLE_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendProgressColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendDateColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Activatable, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendBitmapColumn( const std::string &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendTextColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendIconTextColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendToggleColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_TOGGLE_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendProgressColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = wxDVC_DEFAULT_WIDTH,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendDateColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Activatable, int width = -1,
                    wxAlignment align = wxALIGN_NOT,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendBitmapColumn( const wxBitmap &label, unsigned int model_column,
                    wxDataViewCellMode mode = wxDataViewCellMode::Inert, int width = -1,
                    wxAlignment align = wxALIGN_CENTER,
                    unsigned int flags = wxDATAVIEW_COL_RESIZABLE );

    virtual bool PrependColumn( wxDataViewColumn *col );
    virtual bool InsertColumn( unsigned int pos, wxDataViewColumn *col );
    virtual bool AppendColumn( wxDataViewColumn *col );

    virtual unsigned int GetColumnCount() const = 0;
    virtual wxDataViewColumn* GetColumn( unsigned int pos ) const = 0;
    virtual int GetColumnPosition( const wxDataViewColumn *column ) const = 0;

    virtual bool DeleteColumn( wxDataViewColumn *column ) = 0;
    virtual bool ClearColumns() = 0;

    void SetExpanderColumn( wxDataViewColumn *col )
        { m_expander_column = col ; DoSetExpanderColumn(); }
    wxDataViewColumn *GetExpanderColumn() const
        { return m_expander_column; }

    virtual wxDataViewColumn *GetSortingColumn() const = 0;
    virtual std::vector<wxDataViewColumn *> GetSortingColumns() const
    {
        std::vector<wxDataViewColumn *> columns;
        if ( wxDataViewColumn* col = GetSortingColumn() )
            columns.push_back(col);
        return columns;
    }

    // This must be overridden to return true if the control does allow sorting
    // by more than one column, which is not the case by default.
    virtual bool AllowMultiColumnSort(bool allow)
    {
        // We can still return true when disabling multi-column sort.
        return !allow;
    }

    // Return true if multi column sort is currently allowed.
    virtual bool IsMultiColumnSortAllowed() const { return false; }

    // This should also be overridden to actually use the specified column for
    // sorting if using multiple columns is supported.
    virtual void ToggleSortByColumn([[maybe_unused]] int column) { }


    // items management
    // ----------------

    void SetIndent( int indent )
        { m_indent = indent ; DoSetIndent(); }
    int GetIndent() const
        { return m_indent; }

    // Current item is the one used by the keyboard navigation, it is the same
    // as the (unique) selected item in single selection mode so these
    // functions are mostly useful for controls with wxDV_MULTIPLE style.
    wxDataViewItem GetCurrentItem() const;
    void SetCurrentItem(const wxDataViewItem& item);

    virtual wxDataViewItem GetTopItem() const { return wxDataViewItem(nullptr); }
    virtual int GetCountPerPage() const { return wxNOT_FOUND; }

    // Currently focused column of the current item or NULL if no column has focus
    virtual wxDataViewColumn *GetCurrentColumn() const = 0;

    // Selection: both GetSelection() and GetSelections() can be used for the
    // controls both with and without wxDV_MULTIPLE style. For single selection
    // controls GetSelections() is not very useful however. And for multi
    // selection controls GetSelection() returns an invalid item if more than
    // one item is selected. Use GetSelectedItemsCount() or HasSelection() to
    // check if any items are selected at all.
    virtual int GetSelectedItemsCount() const = 0;
    bool HasSelection() const { return GetSelectedItemsCount() != 0; }
    wxDataViewItem GetSelection() const;
    virtual int GetSelections( wxDataViewItemArray & sel ) const = 0;
    virtual void SetSelections( const wxDataViewItemArray & sel ) = 0;
    virtual void Select( const wxDataViewItem & item ) = 0;
    virtual void Unselect( const wxDataViewItem & item ) = 0;
    virtual bool IsSelected( const wxDataViewItem & item ) const = 0;

    virtual void SelectAll() = 0;
    virtual void UnselectAll() = 0;

    void Expand( const wxDataViewItem & item );
    void ExpandChildren( const wxDataViewItem & item );
    void ExpandAncestors( const wxDataViewItem & item );
    virtual void Collapse( const wxDataViewItem & item ) = 0;
    virtual bool IsExpanded( const wxDataViewItem & item ) const = 0;

    virtual void EnsureVisible( const wxDataViewItem & item,
                                const wxDataViewColumn *column = nullptr ) = 0;
    virtual void HitTest( const wxPoint & point, wxDataViewItem &item, wxDataViewColumn* &column ) const = 0;
    virtual wxRect GetItemRect( const wxDataViewItem & item, const wxDataViewColumn *column = nullptr ) const = 0;

    virtual bool SetRowHeight( [[maybe_unused]] int rowHeight ) { return false; }

    virtual void EditItem(const wxDataViewItem& item, const wxDataViewColumn *column) = 0;

#if wxUSE_DRAG_AND_DROP
    virtual bool EnableDragSource([[maybe_unused]] const wxDataFormat& format)
        { return false; }
    virtual bool EnableDropTarget([[maybe_unused]] const wxDataFormat& format)
        { return false; }
#endif // wxUSE_DRAG_AND_DROP

    // define control visual attributes
    // --------------------------------

    // Header attributes: only implemented in the generic version currently.
    virtual bool SetHeaderAttr([[maybe_unused]] const wxItemAttr& attr)
        { return false; }

    // Set the colour used for the "alternate" rows when wxDV_ROW_LINES is on.
    // Also only supported in the generic version, which returns true to
    // indicate it.
    virtual bool SetAlternateRowColour([[maybe_unused]] const wxColour& colour)
        { return false; }

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal)
    {
        return wxControl::GetCompositeControlsDefaultAttributes(variant);
    }

protected:
    virtual void DoSetExpanderColumn() = 0 ;
    virtual void DoSetIndent() = 0;

    // Just expand this item assuming it is already shown, i.e. its parent has
    // been already expanded using ExpandAncestors().
    //
    // If expandChildren is true, also expand all its children recursively.
    virtual void DoExpand(const wxDataViewItem & item, bool expandChildren) = 0;

private:
    // Implementation of the public Set/GetCurrentItem() methods which are only
    // called in multi selection case (for single selection controls their
    // implementation is trivial and is done in the base class itself).
    virtual wxDataViewItem DoGetCurrentItem() const = 0;
    virtual void DoSetCurrentItem(const wxDataViewItem& item) = 0;

    wxDataViewModel        *m_model{nullptr};
    wxDataViewColumn       *m_expander_column{nullptr};
    int m_indent{8};
};

// ----------------------------------------------------------------------------
// wxDataViewEvent - the event class for the wxDataViewCtrl notifications
// ----------------------------------------------------------------------------

class wxDataViewEvent : public wxNotifyEvent
{
public:
    // Default ctor, normally shouldn't be used and mostly exists only for
    // backwards compatibility.
    wxDataViewEvent()
         
    {
        Init(nullptr, nullptr, wxDataViewItem());
    }

    // Constructor for the events affecting columns (and possibly also items).
    wxDataViewEvent(wxEventType evtType,
                    wxDataViewCtrlBase* dvc,
                    wxDataViewColumn* column,
                    const wxDataViewItem& item = wxDataViewItem())
        : wxNotifyEvent(evtType, dvc->GetId())
    {
        Init(dvc, column, item);
    }

    // Constructor for the events affecting only the items.
    wxDataViewEvent(wxEventType evtType,
                    wxDataViewCtrlBase* dvc,
                    const wxDataViewItem& item)
        : wxNotifyEvent(evtType, dvc->GetId())
    {
        Init(dvc, nullptr, item);
    }

    wxDataViewEvent(const wxDataViewEvent& event) = default;
	wxDataViewEvent& operator=(const wxDataViewEvent&) = delete;

    wxDataViewItem GetItem() const { return m_item; }
    int GetColumn() const { return m_col; }
    wxDataViewModel* GetModel() const { return m_model; }

    const wxVariant &GetValue() const { return m_value; }
    void SetValue( const wxVariant &value ) { m_value = value; }

    // for wxEVT_DATAVIEW_ITEM_EDITING_DONE only
    bool IsEditCancelled() const { return m_editCancelled; }

    // for wxEVT_DATAVIEW_COLUMN_HEADER_CLICKED only
    wxDataViewColumn *GetDataViewColumn() const { return m_column; }

    // for wxEVT_DATAVIEW_CONTEXT_MENU only
    wxPoint GetPosition() const { return m_pos; }
    void SetPosition( wxPoint pt ) { m_pos = pt; }

    // For wxEVT_DATAVIEW_CACHE_HINT
    int GetCacheFrom() const { return m_cacheFrom; }
    int GetCacheTo() const { return m_cacheTo; }
    void SetCache(int from, int to) { m_cacheFrom = from; m_cacheTo = to; }


#if wxUSE_DRAG_AND_DROP
    // For drag operations
    void SetDataObject( wxDataObject *obj ) { m_dataObject = obj; }
    wxDataObject *GetDataObject() const { return m_dataObject; }

    // For drop operations
    void SetDataFormat( const wxDataFormat &format ) { m_dataFormat = format; }
    wxDataFormat GetDataFormat() const { return m_dataFormat; }
    void SetDataSize( size_t size ) { m_dataSize = size; }
    size_t GetDataSize() const { return m_dataSize; }
    void SetDataBuffer( void* buf ) { m_dataBuffer = buf;}
    void *GetDataBuffer() const { return m_dataBuffer; }
    void SetDragFlags( unsigned int flags ) { m_dragFlags = flags; }
    int GetDragFlags() const { return m_dragFlags; }
    void SetDropEffect( wxDragResult effect ) { m_dropEffect = effect; }
    wxDragResult GetDropEffect() const { return m_dropEffect; }
    // For platforms (currently generic and OSX) that support Drag/Drop
    // insertion of items, this is the proposed child index for the insertion.
    void SetProposedDropIndex(int index) { m_proposedDropIndex = index; }
    int GetProposedDropIndex() const { return m_proposedDropIndex;}
#endif // wxUSE_DRAG_AND_DROP

    wxEvent *Clone() const override { return new wxDataViewEvent(*this); }

    void SetColumn( int col ) { m_col = col; }
    void SetEditCancelled() { m_editCancelled = true; }

protected:
    wxVariant           m_value;

    wxDataViewModel    *m_model;
    wxDataViewColumn   *m_column;
    
#if wxUSE_DRAG_AND_DROP
    wxDataObject       *m_dataObject;

    wxDataFormat        m_dataFormat;
    
    void*               m_dataBuffer;

    size_t              m_dataSize;

    int                 m_dragFlags;
    int                 m_proposedDropIndex;
    
    wxDragResult        m_dropEffect;
#endif // wxUSE_DRAG_AND_DROP

    wxPoint             m_pos;

    wxDataViewItem      m_item;
    
    int                 m_col;
    int                 m_cacheFrom;
    int                 m_cacheTo;

    bool                m_editCancelled;

private:
    // Common part of non-copy ctors.
    void Init(wxDataViewCtrlBase* dvc,
              wxDataViewColumn* column,
              const wxDataViewItem& item);
};

wxDECLARE_EVENT( wxEVT_DATAVIEW_SELECTION_CHANGED, wxDataViewEvent );

wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_ACTIVATED, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_COLLAPSED, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_EXPANDED, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_COLLAPSING, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_EXPANDING, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_START_EDITING, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_EDITING_STARTED, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_EDITING_DONE, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEvent );

wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEvent );

wxDECLARE_EVENT( wxEVT_DATAVIEW_COLUMN_HEADER_CLICK, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_COLUMN_SORTED, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_COLUMN_REORDERED, wxDataViewEvent );

wxDECLARE_EVENT( wxEVT_DATAVIEW_CACHE_HINT, wxDataViewEvent );

wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, wxDataViewEvent );
wxDECLARE_EVENT( wxEVT_DATAVIEW_ITEM_DROP, wxDataViewEvent );

typedef void (wxEvtHandler::*wxDataViewEventFunction)(wxDataViewEvent&);

#define wxDataViewEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxDataViewEventFunction, func)

#define wx__DECLARE_DATAVIEWEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_DATAVIEW_ ## evt, id, wxDataViewEventHandler(fn))

#define EVT_DATAVIEW_SELECTION_CHANGED(id, fn) wx__DECLARE_DATAVIEWEVT(SELECTION_CHANGED, id, fn)

#define EVT_DATAVIEW_ITEM_ACTIVATED(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_ACTIVATED, id, fn)
#define EVT_DATAVIEW_ITEM_COLLAPSING(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_COLLAPSING, id, fn)
#define EVT_DATAVIEW_ITEM_COLLAPSED(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_COLLAPSED, id, fn)
#define EVT_DATAVIEW_ITEM_EXPANDING(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_EXPANDING, id, fn)
#define EVT_DATAVIEW_ITEM_EXPANDED(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_EXPANDED, id, fn)
#define EVT_DATAVIEW_ITEM_START_EDITING(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_START_EDITING, id, fn)
#define EVT_DATAVIEW_ITEM_EDITING_STARTED(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_EDITING_STARTED, id, fn)
#define EVT_DATAVIEW_ITEM_EDITING_DONE(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_EDITING_DONE, id, fn)
#define EVT_DATAVIEW_ITEM_VALUE_CHANGED(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_VALUE_CHANGED, id, fn)

#define EVT_DATAVIEW_ITEM_CONTEXT_MENU(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_CONTEXT_MENU, id, fn)

#define EVT_DATAVIEW_COLUMN_HEADER_CLICK(id, fn) wx__DECLARE_DATAVIEWEVT(COLUMN_HEADER_CLICK, id, fn)
#define EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK(id, fn) wx__DECLARE_DATAVIEWEVT(COLUMN_HEADER_RIGHT_CLICK, id, fn)
#define EVT_DATAVIEW_COLUMN_SORTED(id, fn) wx__DECLARE_DATAVIEWEVT(COLUMN_SORTED, id, fn)
#define EVT_DATAVIEW_COLUMN_REORDERED(id, fn) wx__DECLARE_DATAVIEWEVT(COLUMN_REORDERED, id, fn)
#define EVT_DATAVIEW_CACHE_HINT(id, fn) wx__DECLARE_DATAVIEWEVT(CACHE_HINT, id, fn)

#define EVT_DATAVIEW_ITEM_BEGIN_DRAG(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_BEGIN_DRAG, id, fn)
#define EVT_DATAVIEW_ITEM_DROP_POSSIBLE(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_DROP_POSSIBLE, id, fn)
#define EVT_DATAVIEW_ITEM_DROP(id, fn) wx__DECLARE_DATAVIEWEVT(ITEM_DROP, id, fn)

// Old and not documented synonym, don't use.
#define EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICKED(id, fn) EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK(id, fn)

#ifdef wxHAS_GENERIC_DATAVIEWCTRL
    #include "wx/generic/dataview.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/dataview.h"
#elif defined(__WXMAC__)
    #include "wx/osx/dataview.h"
#elif defined(__WXQT__)
    #include "wx/qt/dataview.h"
#else
    #error "unknown native wxDataViewCtrl implementation"
#endif

//-----------------------------------------------------------------------------
// wxDataViewListStore
//-----------------------------------------------------------------------------

class wxDataViewListStoreLine
{
public:
    wxDataViewListStoreLine( wxUIntPtr data = 0 )
    {
        m_data = data;
    }

    void SetData( wxUIntPtr data )
        { m_data = data; }
    wxUIntPtr GetData() const
        { return m_data; }

    std::vector<wxVariant>  m_values;

private:
    wxUIntPtr m_data;
};


class wxDataViewListStore: public wxDataViewIndexListModel
{
public:
    ~wxDataViewListStore();

    void PrependColumn( const std::string &varianttype );
    void InsertColumn( unsigned int pos, const std::string &varianttype );
    void AppendColumn( const std::string &varianttype );

    void AppendItem( const std::vector<wxVariant> &values, wxUIntPtr data = 0 );
    void PrependItem( const std::vector<wxVariant> &values, wxUIntPtr data = 0 );
    void InsertItem(  unsigned int row, const std::vector<wxVariant> &values, wxUIntPtr data = 0 );
    void DeleteItem( unsigned int pos );
    void DeleteAllItems();
    void ClearColumns();

    unsigned int GetItemCount() const;

    void SetItemData( const wxDataViewItem& item, wxUIntPtr data );
    wxUIntPtr GetItemData( const wxDataViewItem& item ) const;

    // override base virtuals

    unsigned int GetColumnCount() const override;

    std::string GetColumnType( unsigned int col ) const override;

    void GetValueByRow( wxVariant &value,
                           unsigned int row, unsigned int col ) const override;

    bool SetValueByRow( const wxVariant &value,
                           unsigned int row, unsigned int col ) override;


public:
    std::vector<wxDataViewListStoreLine*> m_data;
    std::vector<std::string>                 m_cols;
};

//-----------------------------------------------------------------------------

class wxDataViewListCtrl: public wxDataViewCtrl
{
public:
    wxDataViewListCtrl() = default;
    wxDataViewListCtrl( wxWindow *parent, wxWindowID id,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, unsigned int style = wxDV_ROW_LINES,
           const wxValidator& validator = wxDefaultValidator );

	wxDataViewListCtrl& operator=(const wxDataViewListCtrl&) = delete;

    [[maybe_unused]] bool Create( wxWindow *parent, wxWindowID id,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, unsigned int style = wxDV_ROW_LINES,
           const wxValidator& validator = wxDefaultValidator );

    wxDataViewListStore *GetStore()
        { return (wxDataViewListStore*) GetModel(); }
    const wxDataViewListStore *GetStore() const
        { return (const wxDataViewListStore*) GetModel(); }

    int ItemToRow(const wxDataViewItem &item) const
        { return item.IsOk() ? (int)GetStore()->GetRow(item) : wxNOT_FOUND; }
    wxDataViewItem RowToItem(int row) const
        { return row == wxNOT_FOUND ? wxDataViewItem() : GetStore()->GetItem(row); }

    int GetSelectedRow() const
        { return ItemToRow(GetSelection()); }
    void SelectRow(unsigned row)
        { Select(RowToItem(row)); }
    void UnselectRow(unsigned row)
        { Unselect(RowToItem(row)); }
    bool IsRowSelected(unsigned row) const
        { return IsSelected(RowToItem(row)); }

    bool AppendColumn( wxDataViewColumn *column, const std::string &varianttype );
    bool PrependColumn( wxDataViewColumn *column, const std::string &varianttype );
    bool InsertColumn( unsigned int pos, wxDataViewColumn *column, const std::string &varianttype );

    // overridden from base class
    bool PrependColumn( wxDataViewColumn *col ) override;
    bool InsertColumn( unsigned int pos, wxDataViewColumn *col ) override;
    bool AppendColumn( wxDataViewColumn *col ) override;
    bool ClearColumns() override;

    wxDataViewColumn *AppendTextColumn( const std::string &label,
          wxDataViewCellMode mode = wxDataViewCellMode::Inert,
          int width = -1, wxAlignment align = wxALIGN_LEFT, unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendToggleColumn( const std::string &label,
          wxDataViewCellMode mode = wxDataViewCellMode::Activatable,
          int width = -1, wxAlignment align = wxALIGN_LEFT, unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendProgressColumn( const std::string &label,
          wxDataViewCellMode mode = wxDataViewCellMode::Inert,
          int width = -1, wxAlignment align = wxALIGN_LEFT, unsigned int flags = wxDATAVIEW_COL_RESIZABLE );
    wxDataViewColumn *AppendIconTextColumn( const std::string &label,
          wxDataViewCellMode mode = wxDataViewCellMode::Inert,
          int width = -1, wxAlignment align = wxALIGN_LEFT, unsigned int flags = wxDATAVIEW_COL_RESIZABLE );

    void AppendItem( const std::vector<wxVariant> &values, wxUIntPtr data = 0 )
        { GetStore()->AppendItem( values, data ); }
    void PrependItem( const std::vector<wxVariant> &values, wxUIntPtr data = 0 )
        { GetStore()->PrependItem( values, data ); }
    void InsertItem(  unsigned int row, const std::vector<wxVariant> &values, wxUIntPtr data = 0 )
        { GetStore()->InsertItem( row, values, data ); }
    void DeleteItem( unsigned row )
        { GetStore()->DeleteItem( row ); }
    void DeleteAllItems()
        { GetStore()->DeleteAllItems(); }

    void SetValue( const wxVariant &value, unsigned int row, unsigned int col )
        { GetStore()->SetValueByRow( value, row, col );
          GetStore()->RowValueChanged( row, col); }
    void GetValue( wxVariant &value, unsigned int row, unsigned int col )
        { GetStore()->GetValueByRow( value, row, col ); }

    void SetTextValue( const std::string &value, unsigned int row, unsigned int col )
        { GetStore()->SetValueByRow( value, row, col );
          GetStore()->RowValueChanged( row, col); }
    std::string GetTextValue( unsigned int row, unsigned int col ) const
        { wxVariant value; GetStore()->GetValueByRow( value, row, col ); return value.GetString(); }

    void SetToggleValue( bool value, unsigned int row, unsigned int col )
        { GetStore()->SetValueByRow( value, row, col );
          GetStore()->RowValueChanged( row, col); }
    bool GetToggleValue( unsigned int row, unsigned int col ) const
        { wxVariant value; GetStore()->GetValueByRow( value, row, col ); return value.GetBool(); }

    void SetItemData( const wxDataViewItem& item, wxUIntPtr data )
        { GetStore()->SetItemData( item, data ); }
    wxUIntPtr GetItemData( const wxDataViewItem& item ) const
        { return GetStore()->GetItemData( item ); }

    int GetItemCount() const
        { return GetStore()->GetItemCount(); }

    void OnSize( wxSizeEvent &event );

private:
    wxDECLARE_EVENT_TABLE();
};

//-----------------------------------------------------------------------------
// wxDataViewTreeStore
//-----------------------------------------------------------------------------

class wxDataViewTreeStoreNode
{
public:
    wxDataViewTreeStoreNode( wxDataViewTreeStoreNode *parent,
        const std::string &text, const wxIcon &icon = wxNullIcon, wxClientData *data = nullptr );
    virtual ~wxDataViewTreeStoreNode();

    void SetText( const std::string &text )
        { m_text = text; }
    std::string GetText() const
        { return m_text; }
    void SetIcon( const wxIcon &icon )
        { m_icon = icon; }
    const wxIcon &GetIcon() const
        { return m_icon; }
    void SetData( wxClientData *data )
        { delete m_data; m_data = data; }
    wxClientData *GetData() const
        { return m_data; }

    wxDataViewItem GetItem() const
        { return wxDataViewItem(const_cast<void*>(static_cast<const void*>(this))); }

    virtual bool IsContainer()
        { return false; }

    wxDataViewTreeStoreNode *GetParent()
        { return m_parent; }

private:
    std::string               m_text;

    wxIcon                    m_icon;

    wxDataViewTreeStoreNode  *m_parent;
    wxClientData             *m_data;
};

using wxDataViewTreeStoreNodes = std::vector<wxDataViewTreeStoreNode *>;

class wxDataViewTreeStoreContainerNode: public wxDataViewTreeStoreNode
{
public:
    wxDataViewTreeStoreContainerNode( wxDataViewTreeStoreNode *parent,
        const std::string &text, const wxIcon &icon = wxNullIcon, const wxIcon &expanded = wxNullIcon,
        wxClientData *data = nullptr );
    ~wxDataViewTreeStoreContainerNode();

    const wxDataViewTreeStoreNodes &GetChildren() const
        { return m_children; }
    wxDataViewTreeStoreNodes &GetChildren()
        { return m_children; }

    wxDataViewTreeStoreNodes::iterator FindChild(wxDataViewTreeStoreNode* node);

    void SetExpandedIcon( const wxIcon &icon )
        { m_iconExpanded = icon; }
    const wxIcon &GetExpandedIcon() const
        { return m_iconExpanded; }

    void SetExpanded( bool expanded = true )
        { m_isExpanded = expanded; }
    bool IsExpanded() const
        { return m_isExpanded; }

    bool IsContainer() override
        { return true; }

    void DestroyChildren();

private:
    wxDataViewTreeStoreNodes     m_children;
    wxIcon                       m_iconExpanded;
    bool                         m_isExpanded{false};
};

//-----------------------------------------------------------------------------

class wxDataViewTreeStore: public wxDataViewModel
{
public:
    wxDataViewTreeStore();
    ~wxDataViewTreeStore();

    wxDataViewItem AppendItem( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon = wxNullIcon, wxClientData *data = nullptr );
    wxDataViewItem PrependItem( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon = wxNullIcon, wxClientData *data = nullptr );
    wxDataViewItem InsertItem( const wxDataViewItem& parent, const wxDataViewItem& previous,
        const std::string &text, const wxIcon &icon = wxNullIcon, wxClientData *data = nullptr );

    wxDataViewItem PrependContainer( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon = wxNullIcon, const wxIcon &expanded = wxNullIcon,
        wxClientData *data = nullptr );
    wxDataViewItem AppendContainer( const wxDataViewItem& parent,
        const std::string &text, const wxIcon &icon = wxNullIcon, const wxIcon &expanded = wxNullIcon,
        wxClientData *data = nullptr );
    wxDataViewItem InsertContainer( const wxDataViewItem& parent, const wxDataViewItem& previous,
        const std::string &text, const wxIcon &icon = wxNullIcon, const wxIcon &expanded = wxNullIcon,
        wxClientData *data = nullptr );

    wxDataViewItem GetNthChild( const wxDataViewItem& parent, unsigned int pos ) const;
    int GetChildCount( const wxDataViewItem& parent ) const;

    void SetItemText( const wxDataViewItem& item, const std::string &text );
    std::string GetItemText( const wxDataViewItem& item ) const;
    void SetItemIcon( const wxDataViewItem& item, const wxIcon &icon );
    const wxIcon &GetItemIcon( const wxDataViewItem& item ) const;
    void SetItemExpandedIcon( const wxDataViewItem& item, const wxIcon &icon );
    const wxIcon &GetItemExpandedIcon( const wxDataViewItem& item ) const;
    void SetItemData( const wxDataViewItem& item, wxClientData *data );
    wxClientData *GetItemData( const wxDataViewItem& item ) const;

    void DeleteItem( const wxDataViewItem& item );
    void DeleteChildren( const wxDataViewItem& item );
    void DeleteAllItems();

    // implement base methods

    void GetValue( wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) const override;
    bool SetValue( const wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) override;
    wxDataViewItem GetParent( const wxDataViewItem &item ) const override;
    bool IsContainer( const wxDataViewItem &item ) const override;
    unsigned int GetChildren( const wxDataViewItem &item, wxDataViewItemArray &children ) const override;

    int Compare( const wxDataViewItem &item1, const wxDataViewItem &item2,
                         unsigned int column, bool ascending ) const override;

    bool HasDefaultCompare() const override
        { return true; }
    unsigned int GetColumnCount() const override
        { return 1; }
    std::string GetColumnType( [[maybe_unused]] unsigned int col ) const override
        { return "wxDataViewIconText"; }

    wxDataViewTreeStoreNode *FindNode( const wxDataViewItem &item ) const;
    wxDataViewTreeStoreContainerNode *FindContainerNode( const wxDataViewItem &item ) const;
    wxDataViewTreeStoreNode *GetRoot() const { return m_root; }

public:
    wxDataViewTreeStoreNode *m_root;
};

//-----------------------------------------------------------------------------

class wxDataViewTreeCtrl: public wxDataViewCtrl,
                                          public wxWithImages
{
public:
    wxDataViewTreeCtrl() = default;
    wxDataViewTreeCtrl(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxDV_NO_HEADER | wxDV_ROW_LINES,
                       const wxValidator& validator = wxDefaultValidator)
    {
        Create(parent, id, pos, size, style, validator);
    }

	wxDataViewTreeCtrl& operator=(const wxDataViewTreeCtrl&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDV_NO_HEADER | wxDV_ROW_LINES,
                const wxValidator& validator = wxDefaultValidator);

    wxDataViewTreeStore *GetStore()
        { return (wxDataViewTreeStore*) GetModel(); }
    const wxDataViewTreeStore *GetStore() const
        { return (const wxDataViewTreeStore*) GetModel(); }

    bool IsContainer( const wxDataViewItem& item ) const
        { return GetStore()->IsContainer(item); }

    wxDataViewItem AppendItem( const wxDataViewItem& parent,
        const std::string &text, int icon = NO_IMAGE, wxClientData *data = nullptr );
    wxDataViewItem PrependItem( const wxDataViewItem& parent,
        const std::string &text, int icon = NO_IMAGE, wxClientData *data = nullptr );
    wxDataViewItem InsertItem( const wxDataViewItem& parent, const wxDataViewItem& previous,
        const std::string &text, int icon = NO_IMAGE, wxClientData *data = nullptr );

    wxDataViewItem PrependContainer( const wxDataViewItem& parent,
        const std::string &text, int icon = NO_IMAGE, int expanded = NO_IMAGE,
        wxClientData *data = nullptr );
    wxDataViewItem AppendContainer( const wxDataViewItem& parent,
        const std::string &text, int icon = NO_IMAGE, int expanded = NO_IMAGE,
        wxClientData *data = nullptr );
    wxDataViewItem InsertContainer( const wxDataViewItem& parent, const wxDataViewItem& previous,
        const std::string &text, int icon = NO_IMAGE, int expanded = NO_IMAGE,
        wxClientData *data = nullptr );

    wxDataViewItem GetNthChild( const wxDataViewItem& parent, unsigned int pos ) const
        { return GetStore()->GetNthChild(parent, pos); }
    int GetChildCount( const wxDataViewItem& parent ) const
        { return GetStore()->GetChildCount(parent); }

    void SetItemText( const wxDataViewItem& item, const std::string &text );
    std::string GetItemText( const wxDataViewItem& item ) const
        { return GetStore()->GetItemText(item); }
    void SetItemIcon( const wxDataViewItem& item, const wxIcon &icon );
    const wxIcon &GetItemIcon( const wxDataViewItem& item ) const
        { return GetStore()->GetItemIcon(item); }
    void SetItemExpandedIcon( const wxDataViewItem& item, const wxIcon &icon );
    const wxIcon &GetItemExpandedIcon( const wxDataViewItem& item ) const
        { return GetStore()->GetItemExpandedIcon(item); }
    void SetItemData( const wxDataViewItem& item, wxClientData *data )
        { GetStore()->SetItemData(item,data); }
    wxClientData *GetItemData( const wxDataViewItem& item ) const
        { return GetStore()->GetItemData(item); }

    void DeleteItem( const wxDataViewItem& item );
    void DeleteChildren( const wxDataViewItem& item );
    void DeleteAllItems();

    void OnExpanded( wxDataViewEvent &event );
    void OnCollapsed( wxDataViewEvent &event );
    void OnSize( wxSizeEvent &event );

private:
    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_DATAVIEWCTRL

#endif
    // _WX_DATAVIEW_H_BASE_
