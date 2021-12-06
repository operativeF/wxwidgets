///////////////////////////////////////////////////////////////////////////////
// Name:        wx/headerctrl.h
// Purpose:     wxHeaderCtrlBase class: interface of wxHeaderCtrl
// Author:      Vadim Zeitlin
// Created:     2008-12-01
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HEADERCTRL_H_
#define _WX_HEADERCTRL_H_

#include "wx/control.h"

#if wxUSE_HEADERCTRL

#include "wx/headercol.h"

import WX.Cfg.Flags;

import Utils.Geometry;

import <string>;
import <vector>;

import WX.Utils.Cast;


// notice that the classes in this header are defined in the core library even
// although currently they're only used by wxGrid which is in wxAdv because we
// plan to use it in wxListCtrl which is in core too in the future
class wxHeaderCtrlEvent;


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
    // allow column drag and drop
    wxHD_ALLOW_REORDER = 0x0001,

    // allow hiding (and showing back) the columns using the menu shown by
    // right clicking the header
    wxHD_ALLOW_HIDE = 0x0002,

    // force putting column images on right
    wxHD_BITMAP_ON_RIGHT = 0x0004,

    // style used by default when creating the control
    wxHD_DEFAULT_STYLE = wxHD_ALLOW_REORDER
};

inline constexpr std::string_view wxHeaderCtrlNameStr = "wxHeaderCtrl";

// ----------------------------------------------------------------------------
// wxHeaderCtrlBase defines the interface of a header control
// ----------------------------------------------------------------------------

class wxHeaderCtrlBase : public wxControl
{
public:
    /*
        Derived classes must provide default ctor as well as a ctor and
        Create() function with the following signatures:

    wxHeaderCtrl(wxWindow *parent,
                 wxWindowID winid = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxHD_DEFAULT_STYLE,
                 std::string_view name = wxHeaderCtrlNameStr);

    bool Create(wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxHD_DEFAULT_STYLE,
                std::string_view name = wxHeaderCtrlNameStr);
     */

    // column-related methods
    // ----------------------

    // set the number of columns in the control
    //
    // this also calls UpdateColumn() for all columns
    void SetColumnCount(unsigned int count);

    // return the number of columns in the control as set by SetColumnCount()
    unsigned int GetColumnCount() const { return DoGetCount(); }

    // return whether the control has any columns
    bool IsEmpty() const { return DoGetCount() == 0; }

    // update the column with the given index
    void UpdateColumn(unsigned int idx)
    {
        wxCHECK_RET( idx < GetColumnCount(), "invalid column index" );

        DoUpdate(idx);
    }


    // columns order
    // -------------

    // set the columns order: the array defines the column index which appears
    // the given position, it must have GetColumnCount() elements and contain
    // all indices exactly once
    void SetColumnsOrder(const std::vector<int>& order);
    std::vector<int> GetColumnsOrder() const;

    // get the index of the column at the given display position
    unsigned int GetColumnAt(unsigned int pos) const;

    // get the position at which this column is currently displayed
    unsigned int GetColumnPos(unsigned int idx) const;

    // reset the columns order to the natural one
    void ResetColumnsOrder();

    // helper function used by the generic version of this control and also
    // wxGrid: reshuffles the array of column indices indexed by positions
    // (i.e. using the same convention as for SetColumnsOrder()) so that the
    // column with the given index is found at the specified position
    static void MoveColumnInOrderArray(std::vector<int>& order,
                                       unsigned int idx,
                                       unsigned int pos);


    // UI helpers
    // ----------

#if wxUSE_MENUS
    // show the popup menu containing all columns with check marks for the ones
    // which are currently shown and return true if something was done using it
    // (in this case UpdateColumnVisibility() will have been called) or false
    // if the menu was cancelled
    //
    // this is called from the default right click handler for the controls
    // with wxHD_ALLOW_HIDE style
    bool ShowColumnsMenu(const wxPoint& pt, const std::string& title = {});

    // append the entries for all our columns to the given menu, with the
    // currently visible columns being checked
    //
    // this is used by ShowColumnsMenu() but can also be used if you use your
    // own custom columns menu but nevertheless want to show all the columns in
    // it
    //
    // the ids of the items corresponding to the columns are consecutive and
    // start from idColumnsBase
    void AddColumnsItems(wxMenu& menu, int idColumnsBase = 0);
#endif // wxUSE_MENUS

    // show the columns customization dialog and return true if something was
    // changed using it (in which case UpdateColumnVisibility() and/or
    // UpdateColumnsOrder() will have been called)
    //
    // this is called by the control itself from ShowColumnsMenu() (which in
    // turn is only called by the control if wxHD_ALLOW_HIDE style was
    // specified) and if the control has wxHD_ALLOW_REORDER style as well
    bool ShowCustomizeDialog();

    // compute column title width
    int GetColumnTitleWidth(const wxHeaderColumn& col);

    // compute column title width for the column with the given index
    int GetColumnTitleWidth(unsigned int idx)
    {
        return GetColumnTitleWidth(GetColumn(idx));
    }

    // implementation only from now on
    // -------------------------------

    // the user doesn't need to TAB to this control
    bool AcceptsFocusFromKeyboard() const override { return false; }

    // this method is only overridden in order to synchronize the control with
    // the main window when it is scrolled, the derived class must implement
    // DoScrollHorz()
    void ScrollWindow(int dx, int dy, const wxRect *rect = nullptr) override;

protected:
    // this method must be implemented by the derived classes to return the
    // information for the given column
    virtual const wxHeaderColumn& GetColumn(unsigned int idx) const = 0;

    // this method is called from the default EVT_HEADER_SEPARATOR_DCLICK
    // handler to update the fitting column width of the given column, it
    // should return true if the width was really updated
    virtual bool UpdateColumnWidthToFit([[maybe_unused]] unsigned int idx,
                                        [[maybe_unused]] int widthTitle)
    {
        return false;
    }

    // this method is called from ShowColumnsMenu() and must be overridden to
    // update the internal column visibility (there is no need to call
    // UpdateColumn() from here, this will be done internally)
    virtual void UpdateColumnVisibility([[maybe_unused]] unsigned int idx,
                                        [[maybe_unused]] bool show)
    {
        wxFAIL_MSG( "must be overridden if called" );
    }

    // this method is called from ShowCustomizeDialog() to reorder all columns
    // at once and should be implemented for controls using wxHD_ALLOW_REORDER
    // style (there is no need to call SetColumnsOrder() from here, this is
    // done by the control itself)
    virtual void UpdateColumnsOrder([[maybe_unused]] const std::vector<int>& order)
    {
        wxFAIL_MSG( "must be overridden if called" );
    }

    // this method can be overridden in the derived classes to do something
    // (e.g. update/resize some internal data structures) before the number of
    // columns in the control changes
    virtual void OnColumnCountChanging([[maybe_unused]] unsigned int count) { }


    // helper function for the derived classes: update the array of column
    // indices after the number of columns changed
    void DoResizeColumnIndices(std::vector<int>& colIndices, size_t count);

protected:
    // this window doesn't look nice with the border so don't use it by default
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

private:
    // methods implementing our public API and defined in platform-specific
    // implementations
    virtual void DoSetCount(unsigned int count) = 0;
    virtual unsigned int DoGetCount() const = 0;
    virtual void DoUpdate(unsigned int idx) = 0;

    virtual void DoScrollHorz(int dx) = 0;

    virtual void DoSetColumnsOrder(const std::vector<int>& order) = 0;
    virtual std::vector<int> DoGetColumnsOrder() const = 0;


    // event handlers
    void OnSeparatorDClick(wxHeaderCtrlEvent& event);
#if wxUSE_MENUS
    void OnRClick(wxHeaderCtrlEvent& event);
#endif // wxUSE_MENUS

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// wxHeaderCtrl: port-specific header control implementation, notice that this
//               is still an ABC which is meant to be used as part of another
//               control, see wxHeaderCtrlSimple for a standalone version
// ----------------------------------------------------------------------------

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/headerctrl.h"
#else
    #define wxHAS_GENERIC_HEADERCTRL
    #include "wx/generic/headerctrlg.h"
#endif // platform

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// FIXME: use max?
inline constexpr unsigned int wxNO_COLUMN = wx::narrow_cast<unsigned>(-1);
inline constexpr unsigned int wxID_COLUMNS_BASE = 1;

// ----------------------------------------------------------------------------
// wxHeaderCtrlSimple: concrete header control which can be used standalone
// ----------------------------------------------------------------------------

class wxHeaderCtrlSimple : public wxHeaderCtrl
{
public:
    // control creation
    // ----------------

    wxHeaderCtrlSimple()
    { 
        m_sortKey = wxNO_COLUMN;
    }

    wxHeaderCtrlSimple(wxWindow *parent,
                       wxWindowID winid = wxID_ANY,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxHD_DEFAULT_STYLE,
                       std::string_view name = wxHeaderCtrlNameStr)
    {
        
        m_sortKey = wxNO_COLUMN;
        Create(parent, winid, pos, size, style, name);
    }

    wxHeaderCtrlSimple& operator=(wxHeaderCtrlSimple&&) = delete;

    // managing the columns
    // --------------------

    // insert the column at the given position, using GetColumnCount() as
    // position appends it at the end
    void InsertColumn(const wxHeaderColumnSimple& col, unsigned int idx)
    {
        wxCHECK_RET( idx <= GetColumnCount(), "invalid column index" );

        DoInsert(col, idx);
    }

    // append the column to the end of the control
    void AppendColumn(const wxHeaderColumnSimple& col)
    {
        DoInsert(col, GetColumnCount());
    }

    // delete the column at the given index
    void DeleteColumn(unsigned int idx)
    {
        wxCHECK_RET( idx < GetColumnCount(), "invalid column index" );

        DoDelete(idx);
    }

    // delete all the existing columns
    void DeleteAllColumns();


    // modifying columns
    // -----------------

    // show or hide the column, notice that even when a column is hidden we
    // still account for it when using indices
    void ShowColumn(unsigned int idx, bool show = true)
    {
        wxCHECK_RET( idx < GetColumnCount(), "invalid column index" );

        DoShowColumn(idx, show);
    }

    void HideColumn(unsigned int idx)
    {
        ShowColumn(idx, false);
    }

    // indicate that the column is used for sorting
    void ShowSortIndicator(unsigned int idx, bool ascending = true)
    {
        wxCHECK_RET( idx < GetColumnCount(), "invalid column index" );

        DoShowSortIndicator(idx, ascending);
    }

    // remove the sort indicator completely
    void RemoveSortIndicator();

protected:
    // implement/override base class methods
    const wxHeaderColumn& GetColumn(unsigned int idx) const override;
    bool UpdateColumnWidthToFit(unsigned int idx, int widthTitle) override;

    // and define another one to be overridden in the derived classes: it
    // should return the best width for the given column contents or -1 if not
    // implemented, we use it to implement UpdateColumnWidthToFit()
    virtual int GetBestFittingWidth([[maybe_unused]] unsigned int idx) const
    {
        return -1;
    }

    void OnHeaderResizing(wxHeaderCtrlEvent& evt);

private:
    // functions implementing our public API
    void DoInsert(const wxHeaderColumnSimple& col, unsigned int idx);
    void DoDelete(unsigned int idx);
    void DoShowColumn(unsigned int idx, bool show);
    void DoShowSortIndicator(unsigned int idx, bool ascending);

    // common part of all ctors
    

    // bring the column count in sync with the number of columns we store
    void UpdateColumnCount()
    {
        SetColumnCount(wx::narrow_cast<int>(m_cols.size()));
    }


    // all our current columns
    using Columns = std::vector<wxHeaderColumnSimple>;
    Columns m_cols;

    // the column currently used for sorting or -1 if none
    unsigned int m_sortKey;

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// wxHeaderCtrl events
// ----------------------------------------------------------------------------

class wxHeaderCtrlEvent : public wxNotifyEvent
{
public:
    wxHeaderCtrlEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxNotifyEvent(commandType, winid),
          // FIXME: use max?
          m_order(wx::narrow_cast<unsigned int>(-1))
    {
    }

    wxHeaderCtrlEvent(const wxHeaderCtrlEvent& event) = default;
	wxHeaderCtrlEvent& operator=(const wxHeaderCtrlEvent&) = delete;

    // the column which this event pertains to: valid for all header events
    int GetColumn() const { return m_col; }
    void SetColumn(int col) { m_col = col; }

    // the width of the column: valid for column resizing/dragging events only
    int GetWidth() const { return m_width; }
    void SetWidth(int width) { m_width = width; }

    // the new position of the column: for end reorder events only
    unsigned int GetNewOrder() const { return m_order; }
    void SetNewOrder(unsigned int order) { m_order = order; }

    wxEvent *Clone() const override { return new wxHeaderCtrlEvent(*this); }

private:
    // the column affected by the event
    int m_col{-1};

    // the current width for the dragging events
    int m_width{0};

    // the new column position for end reorder event
    unsigned int m_order;
};


wxDECLARE_EVENT( wxEVT_HEADER_CLICK, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_RIGHT_CLICK, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_MIDDLE_CLICK, wxHeaderCtrlEvent );

wxDECLARE_EVENT( wxEVT_HEADER_DCLICK, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_RIGHT_DCLICK, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_MIDDLE_DCLICK, wxHeaderCtrlEvent );

wxDECLARE_EVENT( wxEVT_HEADER_SEPARATOR_DCLICK, wxHeaderCtrlEvent );

wxDECLARE_EVENT( wxEVT_HEADER_BEGIN_RESIZE, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_RESIZING, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_END_RESIZE, wxHeaderCtrlEvent );

wxDECLARE_EVENT( wxEVT_HEADER_BEGIN_REORDER, wxHeaderCtrlEvent );
wxDECLARE_EVENT( wxEVT_HEADER_END_REORDER, wxHeaderCtrlEvent );

wxDECLARE_EVENT( wxEVT_HEADER_DRAGGING_CANCELLED, wxHeaderCtrlEvent );

typedef void (wxEvtHandler::*wxHeaderCtrlEventFunction)(wxHeaderCtrlEvent&);

#define wxHeaderCtrlEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxHeaderCtrlEventFunction, func)

#define wx__DECLARE_HEADER_EVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_HEADER_ ## evt, id, wxHeaderCtrlEventHandler(fn))

#define EVT_HEADER_CLICK(id, fn) wx__DECLARE_HEADER_EVT(CLICK, id, fn)
#define EVT_HEADER_RIGHT_CLICK(id, fn) wx__DECLARE_HEADER_EVT(RIGHT_CLICK, id, fn)
#define EVT_HEADER_MIDDLE_CLICK(id, fn) wx__DECLARE_HEADER_EVT(MIDDLE_CLICK, id, fn)

#define EVT_HEADER_DCLICK(id, fn) wx__DECLARE_HEADER_EVT(DCLICK, id, fn)
#define EVT_HEADER_RIGHT_DCLICK(id, fn) wx__DECLARE_HEADER_EVT(RIGHT_DCLICK, id, fn)
#define EVT_HEADER_MIDDLE_DCLICK(id, fn) wx__DECLARE_HEADER_EVT(MIDDLE_DCLICK, id, fn)

#define EVT_HEADER_SEPARATOR_DCLICK(id, fn) wx__DECLARE_HEADER_EVT(SEPARATOR_DCLICK, id, fn)

#define EVT_HEADER_BEGIN_RESIZE(id, fn) wx__DECLARE_HEADER_EVT(BEGIN_RESIZE, id, fn)
#define EVT_HEADER_RESIZING(id, fn) wx__DECLARE_HEADER_EVT(RESIZING, id, fn)
#define EVT_HEADER_END_RESIZE(id, fn) wx__DECLARE_HEADER_EVT(END_RESIZE, id, fn)

#define EVT_HEADER_BEGIN_REORDER(id, fn) wx__DECLARE_HEADER_EVT(BEGIN_REORDER, id, fn)
#define EVT_HEADER_END_REORDER(id, fn) wx__DECLARE_HEADER_EVT(END_REORDER, id, fn)

#define EVT_HEADER_DRAGGING_CANCELLED(id, fn) wx__DECLARE_HEADER_EVT(DRAGGING_CANCELLED, id, fn)

#endif // wxUSE_HEADERCTRL

#endif // _WX_HEADERCTRL_H_
