/////////////////////////////////////////////////////////////////////////////
// Name:        wx/vscroll.h
// Purpose:     Variable scrolled windows (wx[V/H/HV]ScrolledWindow)
// Author:      Vadim Zeitlin
// Modified by: Brad Anderson, Bryan Petty
// Created:     30.05.03
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_VSCROLL_H_
#define _WX_VSCROLL_H_

#include "wx/panel.h"
#include "wx/scrolwin.h"

import Utils.Position;

import <string>;

class wxVarScrollHelperEvtHandler;

// Using the same techniques as the wxScrolledWindow class      |
// hierarchy, the wx[V/H/HV]ScrolledWindow classes are slightly |
// more complex (compare with the diagram outlined in           |
// scrolwin.h) for the purpose of reducing code duplication     |
// through the use of mix-in classes.                           |
//                                                              |
//                   wxAnyScrollHelperBase                      |
//                            |                                 |
//                            |                                 |
//                            |                                 |
//                            V                                 |
//                  wxVarScrollHelperBase                       |
//                   /                 \                        |
//                  /                   \                       |
//                 V                     V                      |
//  wxVarHScrollHelper                 wxVarVScrollHelper       |
//       |          \                   /           |           |
//       |           \                 /            |           |
//       |            V               V             |           |
//       |           wxVarHVScrollHelper            |           |
//       |                      |                   |           |
//       |                      |                   V           |
//       |         wxPanel      |    wxVarVScrollLegacyAdaptor  |
//       |         /  \  \      |                   |           |
//       |        /    \  `-----|----------.        |           |
//       |       /      \       |           \       |           |
//       |      /        \      |            \      |           |
//       V     V          \     |             V     V           |
//  wxHScrolledWindow      \    |        wxVScrolledWindow      |
//                          V   V                               |
//                    wxHVScrolledWindow                        |
//                                                              |
//                                                              |
//   Border added to suppress GCC multi-line comment warnings ->|


// ===========================================================================
// wxVarScrollHelperBase
// ===========================================================================

// Provides all base common scroll calculations needed for either orientation,
// automatic scrollbar functionality, saved scroll positions, functionality
// for changing the target window to be scrolled, as well as defining all
// required virtual functions that need to be implemented for any orientation
// specific work.

class wxVarScrollHelperBase : public wxAnyScrollHelperBase
{
public:
    // constructors and such
    // ---------------------

    wxVarScrollHelperBase(wxWindow *winToScroll);
    ~wxVarScrollHelperBase();

    // operations
    // ----------

    // with physical scrolling on, the device origin is changed properly when
    // a wxPaintDC is prepared, children are actually moved and laid out
    // properly, and the contents of the window (pixels) are actually moved
    void EnablePhysicalScrolling(bool scrolling = true)
        { m_physicalScrolling = scrolling; }

    // wxNOT_FOUND if none, i.e. if it is below the last item
    int VirtualHitTest(wxCoord coord) const;

    // recalculate all our parameters and redisplay all units
    virtual void RefreshAll();

    // get the first currently visible unit
    size_t GetVisibleBegin() const { return m_unitFirst; }

    // get the last currently visible unit
    size_t GetVisibleEnd() const
        { return m_unitFirst + m_nUnitsVisible; }

    // is this unit currently visible?
    bool IsVisible(size_t unit) const
        { return unit >= m_unitFirst && unit < GetVisibleEnd(); }

    // translate between scrolled and unscrolled coordinates
    int CalcScrolledPosition(int coord) const
        {  return DoCalcScrolledPosition(coord); }
    int CalcUnscrolledPosition(int coord) const
        {  return DoCalcUnscrolledPosition(coord); }

    virtual int DoCalcScrolledPosition(int coord) const;
    virtual int DoCalcUnscrolledPosition(int coord) const;

    // update the thumb size shown by the scrollbar
    virtual void UpdateScrollbar();
    void RemoveScrollbar();

    // Normally the wxScrolledWindow will scroll itself, but in some rare
    // occasions you might want it to scroll [part of] another window (e.g. a
    // child of it in order to scroll only a portion the area between the
    // scrollbars (spreadsheet: only cell area will move).
    virtual void SetTargetWindow(wxWindow *target);

    // change the DC origin according to the scroll position. To properly
    // forward calls to wxWindow::Layout use WX_FORWARD_TO_SCROLL_HELPER()
    // derived class
    void DoPrepareDC(wxDC& dc) override;

    // the methods to be called from the window event handlers
    void HandleOnScroll(wxScrollWinEvent& event);
    void HandleOnSize(wxSizeEvent& event);
#if wxUSE_MOUSEWHEEL
    void HandleOnMouseWheel(wxMouseEvent& event);
#endif // wxUSE_MOUSEWHEEL

    // these functions must be overridden in the derived class to return
    // orientation specific data (e.g. the width for vertically scrolling
    // derivatives in the case of GetOrientationTargetSize())
    virtual int GetOrientationTargetSize() const = 0;
    virtual int GetNonOrientationTargetSize() const = 0;
    virtual wxOrientation GetOrientation() const = 0;

protected:
    // all *Unit* functions are protected to be exposed by
    // wxVarScrollHelperBase implementations (with appropriate names)

    // get the number of units this window contains (previously set by
    // SetUnitCount())
    size_t GetUnitCount() const { return m_unitMax; }

    // set the number of units the helper contains: the derived class must
    // provide the sizes for all units with indices up to the one given here
    // in its OnGetUnitSize()
    void SetUnitCount(size_t count);

    // redraw the specified unit
    virtual void RefreshUnit(size_t unit);

    // redraw all units in the specified range (inclusive)
    virtual void RefreshUnits(size_t from, size_t to);

    // scroll to the specified unit: it will become the first visible unit in
    // the window
    //
    // return true if we scrolled the window, false if nothing was done
    bool DoScrollToUnit(size_t unit);

    // scroll by the specified number of units/pages
    virtual bool DoScrollUnits(int units);
    virtual bool DoScrollPages(int pages);

    // this function must be overridden in the derived class and it should
    // return the size of the given unit in pixels
    virtual wxCoord OnGetUnitSize(size_t n) const = 0;

    // this function doesn't have to be overridden but it may be useful to do
    // it if calculating the units' sizes is a relatively expensive operation
    // as it gives the user code a possibility to calculate several of them at
    // once
    //
    // OnGetUnitsSizeHint() is normally called just before OnGetUnitSize() but
    // you shouldn't rely on the latter being called for all units in the
    // interval specified here. It is also possible that OnGetUnitHeight() will
    // be called for the units outside of this interval, so this is really just
    // a hint, not a promise.
    //
    // finally note that unitMin is inclusive, while unitMax is exclusive, as
    // usual
    virtual void OnGetUnitsSizeHint([[maybe_unused]] size_t unitMin,
                                    [[maybe_unused]] size_t unitMax) const
        { }

    // when the number of units changes, we try to estimate the total size
    // of all units which is a rather expensive operation in terms of unit
    // access, so if the user code may estimate the average size
    // better/faster than we do, it should override this function to implement
    // its own logic
    //
    // this function should return the best guess for the total size it may
    // make
    virtual wxCoord EstimateTotalSize() const { return DoEstimateTotalSize(); }

    wxCoord DoEstimateTotalSize() const;

    // find the index of the unit we need to show to fit the specified unit on
    // the opposite side either fully or partially (depending on fullyVisible)
    size_t FindFirstVisibleFromLast(size_t last,
                                    bool fullyVisible = false) const;

    // get the total size of the units between unitMin (inclusive) and
    // unitMax (exclusive)
    wxCoord GetUnitsSize(size_t unitMin, size_t unitMax) const;

    // get the offset of the first visible unit
    wxCoord GetScrollOffset() const
        { return GetUnitsSize(0, GetVisibleBegin()); }

    // get the size of the target window
    wxSize GetTargetSize() const { return m_targetWindow->GetClientSize(); }

    void GetTargetSize(int *w, int *h)
    {
        wxSize size = GetTargetSize();
        if ( w )
            *w = size.x;
        if ( h )
            *h = size.y;
    }

    // calculate the new scroll position based on scroll event type
    size_t GetNewScrollPosition(wxScrollWinEvent& event) const;

    // replacement implementation of wxWindow::Layout virtual method.  To
    // properly forward calls to wxWindow::Layout use
    // WX_FORWARD_TO_SCROLL_HELPER() derived class
    bool ScrollLayout();

#ifdef __WXMAC__
    // queue mac window update after handling scroll event
    virtual void UpdateMacScrollWindow() { }
#endif // __WXMAC__

    // change the target window
    void DoSetTargetWindow(wxWindow *target);

    // delete the event handler we installed
    void DeleteEvtHandler();

    // helper function abstracting the orientation test: with vertical
    // orientation, it assigns the first value to x and the second one to y,
    // with horizontal orientation it reverses them, i.e. the first value is
    // assigned to y and the second one to x
    void AssignOrient(wxCoord& x, wxCoord& y, wxCoord first, wxCoord second);

    // similar to "oriented assignment" above but does "oriented increment":
    // for vertical orientation, y is incremented by the given value and x if
    // left unchanged, for horizontal orientation x is incremented
    void IncOrient(wxCoord& x, wxCoord& y, wxCoord inc);

private:

    // handler injected into target window to forward some useful events to us
    wxVarScrollHelperEvtHandler *m_handler{nullptr};
    
    // the total number of (logical) units
    size_t m_unitMax{0};

    // the first currently visible unit
    size_t m_unitFirst{0};

    // the number of currently visible units (including the last, possibly only
    // partly, visible one)
    size_t m_nUnitsVisible{0};

    // the total (estimated) size
    wxCoord m_sizeTotal{0};

    // accumulated mouse wheel rotation
#if wxUSE_MOUSEWHEEL
    int m_sumWheelRotation{0};
#endif

    // do child scrolling (used in DoPrepareDC())
    bool m_physicalScrolling{true};
};



// ===========================================================================
// wxVarVScrollHelper
// ===========================================================================

// Provides public API functions targeted for vertical-specific scrolling,
// wrapping the functionality of wxVarScrollHelperBase.

class wxVarVScrollHelper : public wxVarScrollHelperBase
{
public:
    // constructors and such
    // ---------------------

    // ctor must be given the associated window
    wxVarVScrollHelper(wxWindow *winToScroll)
        : wxVarScrollHelperBase(winToScroll)
    {
    }

    // operators

    void SetRowCount(size_t rowCount) { SetUnitCount(rowCount); }
    bool ScrollToRow(size_t row) { return DoScrollToUnit(row); }

    virtual bool ScrollRows(int rows)
        { return DoScrollUnits(rows); }
    virtual bool ScrollRowPages(int pages)
        { return DoScrollPages(pages); }

    virtual void RefreshRow(size_t row)
        { RefreshUnit(row); }
    virtual void RefreshRows(size_t from, size_t to)
        { RefreshUnits(from, to); }

    size_t GetRowCount() const                  { return GetUnitCount(); }
    size_t GetVisibleRowsBegin() const          { return GetVisibleBegin(); }
    size_t GetVisibleRowsEnd() const            { return GetVisibleEnd(); }
    bool IsRowVisible(size_t row) const         { return IsVisible(row); }

    int GetOrientationTargetSize() const override
        { return GetTargetWindow()->GetClientSize().y; }
    int GetNonOrientationTargetSize() const override
        { return GetTargetWindow()->GetClientSize().x; }
    wxOrientation GetOrientation() const override { return wxVERTICAL; }

protected:
    // this function must be overridden in the derived class and it should
    // return the size of the given row in pixels
    virtual wxCoord OnGetRowHeight(size_t n) const = 0;
    wxCoord OnGetUnitSize(size_t n) const override       { return OnGetRowHeight(n); }

    virtual void OnGetRowsHeightHint([[maybe_unused]] size_t rowMin,
                                     [[maybe_unused]] size_t rowMax) const { }

    // forward calls to OnGetRowsHeightHint()
    void OnGetUnitsSizeHint(size_t unitMin, size_t unitMax) const override
        { OnGetRowsHeightHint(unitMin, unitMax); }

    // again, if not overridden, it will fall back on default method
    virtual wxCoord EstimateTotalHeight() const
        { return DoEstimateTotalSize(); }

    // forward calls to EstimateTotalHeight()
    wxCoord EstimateTotalSize() const override { return EstimateTotalHeight(); }

    wxCoord GetRowsHeight(size_t rowMin, size_t rowMax) const
        { return GetUnitsSize(rowMin, rowMax); }
};



// ===========================================================================
// wxVarHScrollHelper
// ===========================================================================

// Provides public API functions targeted for horizontal-specific scrolling,
// wrapping the functionality of wxVarScrollHelperBase.

class wxVarHScrollHelper : public wxVarScrollHelperBase
{
public:
    // constructors and such
    // ---------------------

    // ctor must be given the associated window
    wxVarHScrollHelper(wxWindow *winToScroll)
        : wxVarScrollHelperBase(winToScroll)
    {
    }

    // operators

    void SetColumnCount(size_t columnCount)
        { SetUnitCount(columnCount); }

    bool ScrollToColumn(size_t column)
        { return DoScrollToUnit(column); }
    virtual bool ScrollColumns(int columns)
        { return DoScrollUnits(columns); }
    virtual bool ScrollColumnPages(int pages)
        { return DoScrollPages(pages); }

    virtual void RefreshColumn(size_t column)
        { RefreshUnit(column); }
    virtual void RefreshColumns(size_t from, size_t to)
        { RefreshUnits(from, to); }

    size_t GetColumnCount() const
        { return GetUnitCount(); }
    size_t GetVisibleColumnsBegin() const
        { return GetVisibleBegin(); }
    size_t GetVisibleColumnsEnd() const
        { return GetVisibleEnd(); }
    bool IsColumnVisible(size_t column) const
        { return IsVisible(column); }


    int GetOrientationTargetSize() const override
        { return GetTargetWindow()->GetClientSize().x; }
    int GetNonOrientationTargetSize() const override
        { return GetTargetWindow()->GetClientSize().y; }
    wxOrientation GetOrientation() const override   { return wxHORIZONTAL; }

protected:
    // this function must be overridden in the derived class and it should
    // return the size of the given column in pixels
    virtual wxCoord OnGetColumnWidth(size_t n) const = 0;
    wxCoord OnGetUnitSize(size_t n) const override { return OnGetColumnWidth(n); }

    virtual void OnGetColumnsWidthHint([[maybe_unused]] size_t columnMin,
                                        [[maybe_unused]] size_t columnMax) const
        { }

    // forward calls to OnGetColumnsWidthHint()
    void OnGetUnitsSizeHint(size_t unitMin, size_t unitMax) const override
        { OnGetColumnsWidthHint(unitMin, unitMax); }

    // again, if not overridden, it will fall back on default method
    virtual wxCoord EstimateTotalWidth() const { return DoEstimateTotalSize(); }

    // forward calls to EstimateTotalWidth()
    wxCoord EstimateTotalSize() const override { return EstimateTotalWidth(); }

    wxCoord GetColumnsWidth(size_t columnMin, size_t columnMax) const
        { return GetUnitsSize(columnMin, columnMax); }
};



// ===========================================================================
// wxVarHVScrollHelper
// ===========================================================================

// Provides public API functions targeted at functions with similar names in
// both wxVScrollHelper and wxHScrollHelper so class scope doesn't need to be
// specified (since we are using multiple inheritance). It also provides
// functions to make changing values for both orientations at the same time
// easier.

class wxVarHVScrollHelper : public wxVarVScrollHelper,
                                        public wxVarHScrollHelper
{
public:
    // constructors and such
    // ---------------------

    // ctor must be given the associated window
    wxVarHVScrollHelper(wxWindow *winToScroll)
        : wxVarVScrollHelper(winToScroll), wxVarHScrollHelper(winToScroll) { }

    // operators
    // ---------

    // set the number of units the window contains for each axis: the derived
    // class must provide the widths and heights for all units with indices up
    // to each of the one given here in its OnGetColumnWidth() and
    // OnGetRowHeight()
    void SetRowColumnCount(size_t rowCount, size_t columnCount);


    // with physical scrolling on, the device origin is changed properly when
    // a wxPaintDC is prepared, children are actually moved and laid out
    // properly, and the contents of the window (pixels) are actually moved
    void EnablePhysicalScrolling(bool vscrolling = true, bool hscrolling = true)
    {
        wxVarVScrollHelper::EnablePhysicalScrolling(vscrolling);
        wxVarHScrollHelper::EnablePhysicalScrolling(hscrolling);
    }

    // scroll to the specified row/column: it will become the first visible
    // cell in the window
    //
    // return true if we scrolled the window, false if nothing was done
    bool ScrollToRowColumn(size_t row, size_t column);
    bool ScrollToRowColumn(const wxPosition &pos)
        { return ScrollToRowColumn(pos.GetRow(), pos.GetColumn()); }

    // redraw the specified cell
    virtual void RefreshRowColumn(size_t row, size_t column);
    virtual void RefreshRowColumn(const wxPosition &pos)
        { RefreshRowColumn(pos.GetRow(), pos.GetColumn()); }

    // redraw the specified regions (inclusive).  If the target window for
    // both orientations is the same the rectangle of cells is refreshed; if
    // the target windows differ the entire client size opposite the
    // orientation direction is refreshed between the specified limits
    virtual void RefreshRowsColumns(size_t fromRow, size_t toRow,
                                    size_t fromColumn, size_t toColumn);
    virtual void RefreshRowsColumns(const wxPosition& from,
                                    const wxPosition& to)
    {
        RefreshRowsColumns(from.GetRow(), to.GetRow(),
                          from.GetColumn(), to.GetColumn());
    }

    // locate the virtual position from the given device coordinates
    wxPosition VirtualHitTest(wxCoord x, wxCoord y) const;
    wxPosition VirtualHitTest(const wxPoint &pos) const
        { return VirtualHitTest(pos.x, pos.y); }

    // change the DC origin according to the scroll position. To properly
    // forward calls to wxWindow::Layout use WX_FORWARD_TO_SCROLL_HELPER()
    // derived class. We use this version to call both base classes'
    // DoPrepareDC()
    void DoPrepareDC(wxDC& dc) override;

    // replacement implementation of wxWindow::Layout virtual method.  To
    // properly forward calls to wxWindow::Layout use
    // WX_FORWARD_TO_SCROLL_HELPER() derived class. We use this version to
    // call both base classes' ScrollLayout()
    bool ScrollLayout();

    // get the number of units this window contains (previously set by
    // Set[Column/Row/RowColumn/Unit]Count())
    wxSize GetRowColumnCount() const;

    // get the first currently visible units
    wxPosition GetVisibleBegin() const;
    wxPosition GetVisibleEnd() const;

    // is this cell currently visible?
    bool IsVisible(size_t row, size_t column) const;
    bool IsVisible(const wxPosition &pos) const
        { return IsVisible(pos.GetRow(), pos.GetColumn()); }
};

// shortcut to avoid checking compatibility modes later
// remove this and all references to wxVarVScrollLegacyAdaptor once
// wxWidgets 2.6 and 2.8 compatibility is removed
using wxVarVScrollLegacyAdaptor = wxVarVScrollHelper;

// this macro must be used in declaration of wxVarScrollHelperBase-derived
// classes
#define WX_FORWARD_TO_VAR_SCROLL_HELPER()                                     \
public:                                                                       \
    void PrepareDC(wxDC& dc) override { DoPrepareDC(dc); }                     \
    bool Layout() override { return ScrollLayout(); }



// ===========================================================================
// wxVScrolledWindow
// ===========================================================================

// In the name of this class, "V" may stand for "variable" because it can be
// used for scrolling rows of variable heights; "virtual", because it is not
// necessary to know the heights of all rows in advance -- only those which
// are shown on the screen need to be measured; or even "vertical", because
// this class only supports scrolling vertically.

// In any case, this is a generalization of the wxScrolledWindow class which
// can be only used when all rows have the same heights. It lacks some other
// wxScrolledWindow features however, notably it can't scroll only a rectangle
// of the window and not its entire client area.

class wxVScrolledWindow : public wxPanel,
                                      public wxVarVScrollLegacyAdaptor
{
public:
    // constructors and such
    // ---------------------

    // default ctor, you must call Create() later
    wxVScrolledWindow() : wxVarVScrollLegacyAdaptor(this) { }

    // normal ctor, no need to call Create() after this one
    //
    // note that wxVSCROLL is always automatically added to our style, there is
    // no need to specify it explicitly
    wxVScrolledWindow(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      unsigned int style = 0,
                      std::string_view name = wxPanelNameStr)
    : wxVarVScrollLegacyAdaptor(this)
    {
        Create(parent, id, pos, size, style, name);
    }

    wxVScrolledWindow& operator=(wxVScrolledWindow&&) = delete;

    // same as the previous ctor but returns status code: true if ok
    //
    // just as with the ctor above, wxVSCROLL style is always used, there is no
    // need to specify it
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxPanelNameStr)
    {
        return wxPanel::Create(parent, id, pos, size, style | wxVSCROLL, name);
    }

    WX_FORWARD_TO_VAR_SCROLL_HELPER()

#ifdef __WXMAC__
protected:
    void UpdateMacScrollWindow() override { Update(); }
#endif // __WXMAC__

private:
    wxDECLARE_ABSTRACT_CLASS(wxVScrolledWindow);
};



// ===========================================================================
// wxHScrolledWindow
// ===========================================================================

// In the name of this class, "H" stands for "horizontal" because it can be
// used for scrolling columns of variable widths. It is not necessary to know
// the widths of all columns in advance -- only those which are shown on the
// screen need to be measured.

// This is a generalization of the wxScrolledWindow class which can be only
// used when all columns have the same width. It lacks some other
// wxScrolledWindow features however, notably it can't scroll only a rectangle
// of the window and not its entire client area.

class wxHScrolledWindow : public wxPanel,
                                      public wxVarHScrollHelper
{
public:
    // constructors and such
    // ---------------------

    // default ctor, you must call Create() later
    wxHScrolledWindow() : wxVarHScrollHelper(this) { }

    // normal ctor, no need to call Create() after this one
    //
    // note that wxHSCROLL is always automatically added to our style, there is
    // no need to specify it explicitly
    wxHScrolledWindow(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      unsigned int style = 0,
                      std::string_view name = wxPanelNameStr)
        : wxVarHScrollHelper(this)
    {
        Create(parent, id, pos, size, style, name);
    }

    wxHScrolledWindow& operator=(wxHScrolledWindow&&) = delete;

    // same as the previous ctor but returns status code: true if ok
    //
    // just as with the ctor above, wxHSCROLL style is always used, there is no
    // need to specify it
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxPanelNameStr)
    {
        return wxPanel::Create(parent, id, pos, size, style | wxHSCROLL, name);
    }

    WX_FORWARD_TO_VAR_SCROLL_HELPER()

#ifdef __WXMAC__
protected:
    void UpdateMacScrollWindow() override { Update(); }
#endif // __WXMAC__

private:
    wxDECLARE_ABSTRACT_CLASS(wxHScrolledWindow);
};



// ===========================================================================
// wxHVScrolledWindow
// ===========================================================================

// This window inherits all functionality of both vertical and horizontal
// scrolled windows automatically handling everything needed to scroll both
// axis simultaneously.

class wxHVScrolledWindow : public wxPanel,
                                       public wxVarHVScrollHelper
{
public:
    // constructors and such
    // ---------------------

    // default ctor, you must call Create() later
    wxHVScrolledWindow()
        : 
          wxVarHVScrollHelper(this) { }

    // normal ctor, no need to call Create() after this one
    //
    // note that wxVSCROLL and wxHSCROLL are always automatically added to our
    // style, there is no need to specify them explicitly
    wxHVScrolledWindow(wxWindow *parent,
                       wxWindowID id = wxID_ANY,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = 0,
                       std::string_view name = wxPanelNameStr)
        : 
          wxVarHVScrollHelper(this)
    {
        Create(parent, id, pos, size, style, name);
    }

    wxHVScrolledWindow& operator=(wxHVScrolledWindow&&) = delete;

    // same as the previous ctor but returns status code: true if ok
    //
    // just as with the ctor above, wxVSCROLL and wxHSCROLL styles are always
    // used, there is no need to specify them
    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxPanelNameStr)
    {
        return wxPanel::Create(parent, id, pos, size,
                               style | wxVSCROLL | wxHSCROLL, name);
    }

    WX_FORWARD_TO_VAR_SCROLL_HELPER()

#ifdef __WXMAC__
protected:
    void UpdateMacScrollWindow() override { Update(); }
#endif // __WXMAC__

private:
    wxDECLARE_ABSTRACT_CLASS(wxHVScrolledWindow);
};

#endif // _WX_VSCROLL_H_

