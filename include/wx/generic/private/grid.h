/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/private/grid.h
// Purpose:     Private wxGrid structures
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: Santiago Palacios
// Created:     1/08/1999
// Copyright:   (c) Michael Bedward
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_GRID_PRIVATE_H_
#define _WX_GENERIC_GRID_PRIVATE_H_

#if wxUSE_GRID

#include "wx/dc.h"
#include "wx/headerctrl.h"

import WX.Grid;
import Utils.Geometry;

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// array classes
// ----------------------------------------------------------------------------

using wxArrayAttrs = std::vector<wxGridCellAttr*>;

WX_DECLARE_HASH_MAP_WITH_DECL(std::int64_t, wxGridCellAttr*,
                              wxIntegerHash, wxIntegerEqual,
                              wxGridCoordsToAttrMap, class);


// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// header column providing access to the column information stored in wxGrid
// via wxHeaderColumn interface
class wxGridHeaderColumn : public wxHeaderColumn
{
public:
    wxGridHeaderColumn(wxGrid *grid, int col)
        : m_grid(grid),
          m_col(col)
    {
    }

    std::string GetTitle() const override { return m_grid->GetColLabelValue(m_col); }
    wxBitmap GetBitmap() const override { return wxNullBitmap; }
    int GetWidth() const override { return m_grid->GetColSize(m_col); }
    int GetMinWidth() const override { return m_grid->GetColMinimalWidth(m_col); }
    wxAlignment GetAlignment() const override
    {
        int horz;
        int vert;
        m_grid->GetColLabelAlignment(&horz, &vert);

        return static_cast<wxAlignment>(horz);
    }

    unsigned int GetFlags() const override
    {
        // we can't know in advance whether we can sort by this column or not
        // with wxGrid API so suppose we can by default
        int flags = wxCOL_SORTABLE;
        if ( m_grid->CanDragColSize(m_col) )
            flags |= wxCOL_RESIZABLE;
        if ( m_grid->CanDragColMove() )
            flags |= wxCOL_REORDERABLE;
        if ( GetWidth() == 0 )
            flags |= wxCOL_HIDDEN;

        return flags;
    }

    bool IsSortKey() const override
    {
        return m_grid->IsSortingBy(m_col);
    }

    bool IsSortOrderAscending() const override
    {
        return m_grid->IsSortOrderAscending();
    }

private:
    // these really should be const but are not because the column needs to be
    // assignable to be used in a wxVector (in STL build, in non-STL build we
    // avoid the need for this)
    wxGrid *m_grid;
    int m_col;
};

// header control retrieving column information from the grid
class wxGridHeaderCtrl : public wxHeaderCtrl
{
public:
    wxGridHeaderCtrl(wxGrid *owner)
        : wxHeaderCtrl(owner,
                       wxID_ANY,
                       wxDefaultPosition,
                       wxDefaultSize,
                       (owner->CanHideColumns() ? wxHD_ALLOW_HIDE : 0) |
                       (owner->CanDragColMove() ? wxHD_ALLOW_REORDER : 0))
    {
    }

    wxGridHeaderCtrl(const wxGridHeaderCtrl&) = delete;
	wxGridHeaderCtrl& operator=(const wxGridHeaderCtrl&) = delete;

    // Special method to call from wxGrid::DoSetColSize(), see comments there.
    void UpdateIfNotResizing(unsigned int idx)
    {
        if ( !m_inResizing )
            UpdateColumn(idx);
    }

protected:
    const wxHeaderColumn& GetColumn(unsigned int idx) const override
    {
        return m_columns[idx];
    }

    wxGrid *GetOwner() const { return dynamic_cast<wxGrid *>(GetParent()); }

private:
    wxMouseEvent GetDummyMouseEvent() const
    {
        // make up a dummy event for the grid event to use -- unfortunately we
        // can't do anything else here
        wxMouseEvent e;
        e.SetState(wxGetMouseState());
        GetOwner()->ScreenToClient(&e.m_x, &e.m_y);
        return e;
    }

    // override the base class method to update our m_columns array
    void OnColumnCountChanging(unsigned int count) override
    {
        const unsigned countOld = m_columns.size();
        if ( count < countOld )
        {
            // just discard the columns which don't exist any more (notice that
            // we can't use resize() here as it would require the vector
            // value_type, i.e. wxGridHeaderColumn to be default constructible,
            // which it is not)
            m_columns.erase(m_columns.begin() + count, m_columns.end());
        }
        else // new columns added
        {
            // add columns for the new elements
            for ( unsigned n = countOld; n < count; n++ )
                m_columns.push_back(wxGridHeaderColumn(GetOwner(), n));
        }
    }

    // override to implement column auto sizing
    bool UpdateColumnWidthToFit(unsigned int idx, [[maybe_unused]] int widthTitle) override
    {
        GetOwner()->HandleColumnAutosize(idx, GetDummyMouseEvent());

        return true;
    }

    // overridden to react to the actions using the columns popup menu
    void UpdateColumnVisibility(unsigned int idx, bool show) override
    {
        GetOwner()->SetColSize(idx, show ? wxGRID_AUTOSIZE : 0);

        // as this is done by the user we should notify the main program about
        // it
        GetOwner()->SendGridSizeEvent(wxEVT_GRID_COL_SIZE, -1, idx,
                                      GetDummyMouseEvent());
    }

    // overridden to react to the columns order changes in the customization
    // dialog
    void UpdateColumnsOrder(const std::vector<int>& order) override
    {
        GetOwner()->SetColumnsOrder(order);
    }


    // event handlers forwarding wxHeaderCtrl events to wxGrid
    void OnClick(wxHeaderCtrlEvent& event)
    {
        GetOwner()->SendEvent(wxEVT_GRID_LABEL_LEFT_CLICK,
                              -1, event.GetColumn(),
                              GetDummyMouseEvent());

        GetOwner()->DoColHeaderClick(event.GetColumn());
    }

    void OnDoubleClick(wxHeaderCtrlEvent& event)
    {
        if ( !GetOwner()->SendEvent(wxEVT_GRID_LABEL_LEFT_DCLICK,
                                    -1, event.GetColumn(),
                                    GetDummyMouseEvent()) )
        {
            event.Skip();
        }
    }

    void OnRightClick(wxHeaderCtrlEvent& event)
    {
        if ( !GetOwner()->SendEvent(wxEVT_GRID_LABEL_RIGHT_CLICK,
                                    -1, event.GetColumn(),
                                    GetDummyMouseEvent()) )
        {
            event.Skip();
        }
    }

    void OnBeginResize(wxHeaderCtrlEvent& event)
    {
        GetOwner()->DoHeaderStartDragResizeCol(event.GetColumn());

        event.Skip();
    }

    void OnResizing(wxHeaderCtrlEvent& event)
    {
        // Calling wxGrid method results in a call to our own UpdateColumn()
        // because it ends up in wxGrid::SetColSize() which must indeed update
        // the column when it's called by the program -- but in the case where
        // the size change comes from the column itself, it is useless and, in
        // fact, harmful, as it results in extra flicker due to the inefficient
        // implementation of UpdateColumn() in wxMSW wxHeaderCtrl, so skip
        // calling it from our overridden version by setting this flag for the
        // duration of this function execution and checking it in our
        // UpdateIfNotResizing().
        m_inResizing++;

        GetOwner()->DoHeaderDragResizeCol(event.GetWidth());

        m_inResizing--;
    }

    void OnEndResize(wxHeaderCtrlEvent& event)
    {
        GetOwner()->DoHeaderEndDragResizeCol(event.GetWidth());

        event.Skip();
    }

    void OnBeginReorder(wxHeaderCtrlEvent& event)
    {
        GetOwner()->DoStartMoveCol(event.GetColumn());
    }

    void OnEndReorder(wxHeaderCtrlEvent& event)
    {
        GetOwner()->DoEndMoveCol(event.GetNewOrder());
    }

    std::vector<wxGridHeaderColumn> m_columns;

    // The count of OnResizing() call nesting, 0 if not inside it.
    int m_inResizing{};

    wxDECLARE_EVENT_TABLE();
};

// common base class for various grid subwindows
class wxGridSubwindow : public wxWindow
{
public:
    wxGridSubwindow(wxGrid *owner,
                    int additionalStyle = 0,
                    std::string_view name = wxPanelNameStr)
        : wxWindow(owner, wxID_ANY,
                   wxDefaultPosition, wxDefaultSize,
                   wxBORDER_NONE | additionalStyle,
                   name),
          m_owner{owner}
    {
    }

    wxGridSubwindow(const wxGridSubwindow&) = delete;
	wxGridSubwindow& operator=(const wxGridSubwindow&) = delete;

    wxWindow *GetMainWindowOfCompositeControl() override { return m_owner; }

    bool AcceptsFocus() const override { return false; }

    wxGrid *GetOwner() { return m_owner; }

protected:
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    wxGrid *m_owner;

    wxDECLARE_EVENT_TABLE();
};

class wxGridRowLabelWindow : public wxGridSubwindow
{
public:
    wxGridRowLabelWindow(wxGrid *parent)
      : wxGridSubwindow(parent)
    {
    }

    wxGridRowLabelWindow(const wxGridRowLabelWindow&) = delete;
	wxGridRowLabelWindow& operator=(const wxGridRowLabelWindow&) = delete;

    virtual bool IsFrozen() const { return false; }

private:
    void OnPaint( wxPaintEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );

    wxDECLARE_EVENT_TABLE();
};


class wxGridRowFrozenLabelWindow : public wxGridRowLabelWindow
{
public:
    wxGridRowFrozenLabelWindow(wxGrid *parent)
        : wxGridRowLabelWindow(parent)
    {
    }

    bool IsFrozen() const override { return true; }
};


class wxGridColLabelWindow : public wxGridSubwindow
{
public:
    wxGridColLabelWindow(wxGrid *parent)
        : wxGridSubwindow(parent)
    {
    }

    wxGridColLabelWindow(const wxGridColLabelWindow&) = delete;
	wxGridColLabelWindow& operator=(const wxGridColLabelWindow&) = delete;

    virtual bool IsFrozen() const { return false; }

private:
    void OnPaint( wxPaintEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );

    wxDECLARE_EVENT_TABLE();
};


class wxGridColFrozenLabelWindow : public wxGridColLabelWindow
{
public:
    wxGridColFrozenLabelWindow(wxGrid *parent)
        : wxGridColLabelWindow(parent)
    {
    }

    bool IsFrozen() const override { return true; }
};


class wxGridCornerLabelWindow : public wxGridSubwindow
{
public:
    wxGridCornerLabelWindow(wxGrid *parent)
        : wxGridSubwindow(parent)
    {
    }

    wxGridCornerLabelWindow(const wxGridCornerLabelWindow&) = delete;
	wxGridCornerLabelWindow& operator=(const wxGridCornerLabelWindow&) = delete;

private:
    void OnMouseEvent( wxMouseEvent& event );
    void OnMouseWheel( wxMouseEvent& event );
    void OnPaint( wxPaintEvent& event );

    wxDECLARE_EVENT_TABLE();
};

class wxGridWindow : public wxGridSubwindow
{
public:
    // grid window variants for scrolling possibilities
    enum wxGridWindowType
    {
        wxGridWindowNormal          = 0,
        wxGridWindowFrozenCol       = 1,
        wxGridWindowFrozenRow       = 2,
        wxGridWindowFrozenCorner    = wxGridWindowFrozenCol |
                                      wxGridWindowFrozenRow
    };

    wxGridWindow(wxGrid *parent, wxGridWindowType type)
        : wxGridSubwindow(parent,
                          wxWANTS_CHARS | wxCLIP_CHILDREN,
                          "GridWindow"),
          m_type{type}
    {
        SetBackgroundStyle(wxBackgroundStyle::Paint);
    }

    void ScrollWindow( int dx, int dy, const wxRect *rect ) override;

    bool AcceptsFocus() const override { return true; }

    wxGridWindowType GetType() const { return m_type; }

private:
    const wxGridWindowType m_type;

    void OnPaint( wxPaintEvent &event );
    void OnMouseWheel( wxMouseEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnKeyDown( wxKeyEvent& );
    void OnKeyUp( wxKeyEvent& );
    void OnChar( wxKeyEvent& );
    void OnFocus( wxFocusEvent& );

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// the internal data representation used by wxGridCellAttrProvider
// ----------------------------------------------------------------------------

// this class stores attributes set for cells
class wxGridCellAttrData
{
public:
    ~wxGridCellAttrData();

    void SetAttr(wxGridCellAttr *attr, int row, int col);
    wxGridCellAttr *GetAttr(int row, int col) const;
    void UpdateAttrRows( size_t pos, int numRows );
    void UpdateAttrCols( size_t pos, int numCols );

private:
    // Tries to search for the attr for given cell.
    wxGridCoordsToAttrMap::iterator FindIndex(int row, int col) const;

    mutable wxGridCoordsToAttrMap m_attrs;
};

// this class stores attributes set for rows or columns
class wxGridRowOrColAttrData
{
public:
    // empty ctor to suppress warnings
    wxGridRowOrColAttrData() = default;
    ~wxGridRowOrColAttrData();

    void SetAttr(wxGridCellAttr *attr, int rowOrCol);
    wxGridCellAttr *GetAttr(int rowOrCol) const;
    void UpdateAttrRowsOrCols( size_t pos, int numRowsOrCols );

private:
    std::vector<int> m_rowsOrCols;
    wxArrayAttrs m_attrs;
};

// NB: this is just a wrapper around 3 objects: one which stores cell
//     attributes, and 2 others for row/col ones
struct wxGridCellAttrProviderData
{
    wxGridCellAttrData     m_cellAttrs;
    wxGridRowOrColAttrData m_rowAttrs;
    wxGridRowOrColAttrData m_colAttrs;
};

// ----------------------------------------------------------------------------
// operations classes abstracting the difference between operating on rows and
// columns
// ----------------------------------------------------------------------------

// This class allows to write a function only once because by using its methods
// it will apply to both columns and rows.
//
// This is an abstract interface definition, the two concrete implementations
// below should be used when working with rows and columns respectively.
class wxGridOperations
{
public:
    // Returns the operations in the other direction, i.e. wxGridRowOperations
    // if this object is a wxGridColumnOperations and vice versa.
    virtual wxGridOperations& Dual() const = 0;

    // Return the total number of rows or columns.
    virtual int GetTotalNumberOfLines(const wxGrid *grid) const = 0;

    // Return the current number of rows or columns of a grid window.
    virtual int GetNumberOfLines(const wxGrid *grid, wxGridWindow *gridWindow) const = 0;

    // Return the first line for this grid type.
    virtual int GetFirstLine(const wxGrid *grid, wxGridWindow *gridWindow) const = 0;

    // Return the selection mode which allows selecting rows or columns.
    virtual wxGrid::wxGridSelectionModes GetSelectionMode() const = 0;

    // Make a wxGridCellCoords from the given components: thisDir is row or
    // column and otherDir is column or row
    virtual wxGridCellCoords MakeCoords(int thisDir, int otherDir) const = 0;

    // Calculate the scrolled position of the given abscissa or ordinate.
    virtual int CalcScrolledPosition(wxGrid *grid, int pos) const = 0;

    // Selects the horizontal or vertical component from the given object.
    virtual int Select(const wxGridCellCoords& coords) const = 0;
    virtual int Select(const wxPoint& pt) const = 0;
    virtual int Select(const wxSize& sz) const = 0;
    virtual int Select(const wxRect& r) const = 0;
    virtual int& Select(wxRect& r) const = 0;

    // Return or set left/top or right/bottom component of a block.
    virtual int SelectFirst(const wxGridBlockCoords& block) const = 0;
    virtual int SelectLast(const wxGridBlockCoords& block) const = 0;
    virtual void SetFirst(wxGridBlockCoords& block, int line) const = 0;
    virtual void SetLast(wxGridBlockCoords& block, int line) const = 0;

    // Returns width or height of the rectangle
    virtual int& SelectSize(wxRect& r) const = 0;

    // Make a wxSize such that Select() applied to it returns first component
    virtual wxSize MakeSize(int first, int second) const = 0;

    // Sets the row or column component of the given cell coordinates
    virtual void Set(wxGridCellCoords& coords, int line) const = 0;


    // Draws a line parallel to the row or column, i.e. horizontal or vertical:
    // pos is the horizontal or vertical position of the line and start and end
    // are the coordinates of the line extremities in the other direction
    virtual void
        DrawParallelLine(wxDC& dc, int start, int end, int pos) const = 0;

    // Draw a horizontal or vertical line across the given rectangle
    // (this is implemented in terms of above and uses Select() to extract
    // start and end from the given rectangle)
    void DrawParallelLineInRect(wxDC& dc, const wxRect& rect, int pos) const
    {
        const int posStart = Select(rect.GetPosition());
        DrawParallelLine(dc, posStart, posStart + Select(rect.GetSize()), pos);
    }


    // Return the index of the row or column at the given pixel coordinate.
    virtual int
        PosToLine(const wxGrid *grid, int pos, wxGridWindow *gridWindow, bool clip = false) const = 0;

    // Get the top/left position, in pixels, of the given row or column
    virtual int GetLineStartPos(const wxGrid *grid, int line) const = 0;

    // Get the bottom/right position, in pixels, of the given row or column
    virtual int GetLineEndPos(const wxGrid *grid, int line) const = 0;

    // Get the height/width of the given row/column
    virtual int GetLineSize(const wxGrid *grid, int line) const = 0;

    // Get wxGrid::m_rowBottoms/m_colRights array
    virtual const std::vector<int>& GetLineEnds(const wxGrid *grid) const = 0;

    // Get default height row height or column width
    virtual int GetDefaultLineSize(const wxGrid *grid) const = 0;

    // Return the minimal acceptable row height or column width
    virtual int GetMinimalAcceptableLineSize(const wxGrid *grid) const = 0;

    // Return the minimal row height or column width
    virtual int GetMinimalLineSize(const wxGrid *grid, int line) const = 0;

    // Set the row height or column width
    virtual void SetLineSize(wxGrid *grid, int line, int size) const = 0;

    // Set the row default height or column default width
    virtual void SetDefaultLineSize(wxGrid *grid, int size, bool resizeExisting) const = 0;


    // Return the index of the line at the given position
    //
    // NB: currently this is always identity for the rows as reordering is only
    //     implemented for the lines
    virtual int GetLineAt(const wxGrid *grid, int pos) const = 0;

    // Return the display position of the line with the given index.
    //
    // NB: As GetLineAt(), currently this is always identity for rows.
    virtual int GetLinePos(const wxGrid *grid, int line) const = 0;

    // Return the index of the line just before the given one or wxNOT_FOUND.
    virtual int GetLineBefore(const wxGrid* grid, int line) const = 0;

    // Get the row or column label window
    virtual wxWindow *GetHeaderWindow(wxGrid *grid) const = 0;

    // Get the width or height of the row or column label window
    virtual int GetHeaderWindowSize(wxGrid *grid) const = 0;

    // Get the row or column frozen grid window
    virtual wxGridWindow *GetFrozenGrid(wxGrid* grid) const = 0;

    // This class is never used polymorphically but give it a virtual dtor
    // anyhow to suppress g++ complaints about it
    virtual ~wxGridOperations() = default;
};

class wxGridRowOperations : public wxGridOperations
{
public:
    wxGridOperations& Dual() const override;

    int GetTotalNumberOfLines(const wxGrid *grid) const override
        { return grid->GetNumberRows(); }

    int GetNumberOfLines(const wxGrid *grid, wxGridWindow *gridWindow) const override;

    int GetFirstLine(const wxGrid *grid, wxGridWindow *gridWindow) const override;

    wxGrid::wxGridSelectionModes GetSelectionMode() const override
        { return wxGrid::wxGridSelectRows; }

    wxGridCellCoords MakeCoords(int thisDir, int otherDir) const override
        { return wxGridCellCoords(thisDir, otherDir); }

    int CalcScrolledPosition(wxGrid *grid, int pos) const override
        { return grid->CalcScrolledPosition(wxPoint(pos, 0)).x; }

    int Select(const wxGridCellCoords& c) const override { return c.GetRow(); }
    int Select(const wxPoint& pt) const override { return pt.x; }
    int Select(const wxSize& sz) const override { return sz.x; }
    int Select(const wxRect& r) const override { return r.x; }
    int& Select(wxRect& r) const override { return r.x; }
    int SelectFirst(const wxGridBlockCoords& block) const override
        { return block.GetTopRow(); }
    int SelectLast(const wxGridBlockCoords& block) const override
        { return block.GetBottomRow(); }
    void SetFirst(wxGridBlockCoords& block, int line) const override
        { block.SetTopRow(line); }
    void SetLast(wxGridBlockCoords& block, int line) const override
        { block.SetBottomRow(line); }
    int& SelectSize(wxRect& r) const override { return r.width; }
    wxSize MakeSize(int first, int second) const override
        { return wxSize(first, second); }
    void Set(wxGridCellCoords& coords, int line) const override
        { coords.SetRow(line); }

    void DrawParallelLine(wxDC& dc, int start, int end, int pos) const override
        { dc.DrawLine(start, pos, end, pos); }

    int PosToLine(const wxGrid *grid, int pos, wxGridWindow *gridWindow , bool clip = false) const override
        { return grid->YToRow(pos, clip, gridWindow); }
    int GetLineStartPos(const wxGrid *grid, int line) const override
        { return grid->GetRowTop(line); }
    int GetLineEndPos(const wxGrid *grid, int line) const override
        { return grid->GetRowBottom(line); }
    int GetLineSize(const wxGrid *grid, int line) const override
        { return grid->GetRowHeight(line); }
    const std::vector<int>& GetLineEnds(const wxGrid *grid) const override
        { return grid->m_rowBottoms; }
    int GetDefaultLineSize(const wxGrid *grid) const override
        { return grid->GetDefaultRowSize(); }
    int GetMinimalAcceptableLineSize(const wxGrid *grid) const override
        { return grid->GetRowMinimalAcceptableHeight(); }
    int GetMinimalLineSize(const wxGrid *grid, int line) const override
        { return grid->GetRowMinimalHeight(line); }
    void SetLineSize(wxGrid *grid, int line, int size) const override
        { grid->SetRowSize(line, size); }
    void SetDefaultLineSize(wxGrid *grid, int size, bool resizeExisting) const override
        {  grid->SetDefaultRowSize(size, resizeExisting); }

    int GetLineAt([[maybe_unused]] const wxGrid * grid, int pos) const override
        { return pos; } // TODO: implement row reordering
    int GetLinePos([[maybe_unused]] const wxGrid * grid, int line) const override
        { return line; } // TODO: implement row reordering

    int GetLineBefore([[maybe_unused]] const wxGrid* grid, int line) const override
        { return line - 1; }

    wxWindow *GetHeaderWindow(wxGrid *grid) const override
        { return grid->GetGridRowLabelWindow(); }
    int GetHeaderWindowSize(wxGrid *grid) const override
        { return grid->GetRowLabelSize(); }

    wxGridWindow *GetFrozenGrid(wxGrid* grid) const override
        { return (wxGridWindow*)grid->GetFrozenRowGridWindow(); }
};

class wxGridColumnOperations : public wxGridOperations
{
public:
    wxGridOperations& Dual() const override;

    int GetTotalNumberOfLines(const wxGrid *grid) const override
        { return grid->GetNumberCols(); }

    int GetNumberOfLines(const wxGrid *grid, wxGridWindow *gridWindow) const override;

    int GetFirstLine(const wxGrid *grid, wxGridWindow *gridWindow) const override;

    wxGrid::wxGridSelectionModes GetSelectionMode() const override
        { return wxGrid::wxGridSelectColumns; }

    wxGridCellCoords MakeCoords(int thisDir, int otherDir) const override
        { return wxGridCellCoords(otherDir, thisDir); }

    int CalcScrolledPosition(wxGrid *grid, int pos) const override
        { return grid->CalcScrolledPosition(wxPoint(0, pos)).y; }

    int Select(const wxGridCellCoords& c) const override { return c.GetCol(); }
    int Select(const wxPoint& pt) const override { return pt.y; }
    int Select(const wxSize& sz) const override { return sz.y; }
    int Select(const wxRect& r) const override { return r.y; }
    int& Select(wxRect& r) const override { return r.y; }
    int SelectFirst(const wxGridBlockCoords& block) const override
        { return block.GetLeftCol(); }
    int SelectLast(const wxGridBlockCoords& block) const override
        { return block.GetRightCol(); }
    void SetFirst(wxGridBlockCoords& block, int line) const override
        { block.SetLeftCol(line); }
    void SetLast(wxGridBlockCoords& block, int line) const override
        { block.SetRightCol(line); }
    int& SelectSize(wxRect& r) const override { return r.height; }
    wxSize MakeSize(int first, int second) const override
        { return wxSize(second, first); }
    void Set(wxGridCellCoords& coords, int line) const override
        { coords.SetCol(line); }

    void DrawParallelLine(wxDC& dc, int start, int end, int pos) const override
        { dc.DrawLine(pos, start, pos, end); }

    int PosToLine(const wxGrid *grid, int pos, wxGridWindow *gridWindow, bool clip = false) const override
        { return grid->XToCol(pos, clip, gridWindow); }
    int GetLineStartPos(const wxGrid *grid, int line) const override
        { return grid->GetColLeft(line); }
    int GetLineEndPos(const wxGrid *grid, int line) const override
        { return grid->GetColRight(line); }
    int GetLineSize(const wxGrid *grid, int line) const override
        { return grid->GetColWidth(line); }
    const std::vector<int>& GetLineEnds(const wxGrid *grid) const override
        { return grid->m_colRights; }
    int GetDefaultLineSize(const wxGrid *grid) const override
        { return grid->GetDefaultColSize(); }
    int GetMinimalAcceptableLineSize(const wxGrid *grid) const override
        { return grid->GetColMinimalAcceptableWidth(); }
    int GetMinimalLineSize(const wxGrid *grid, int line) const override
        { return grid->GetColMinimalWidth(line); }
    void SetLineSize(wxGrid *grid, int line, int size) const override
        { grid->SetColSize(line, size); }
    void SetDefaultLineSize(wxGrid *grid, int size, bool resizeExisting) const override
        {  grid->SetDefaultColSize(size, resizeExisting); }

    int GetLineAt(const wxGrid *grid, int pos) const override
        { return grid->GetColAt(pos); }
    int GetLinePos(const wxGrid *grid, int line) const override
        { return grid->GetColPos(line); }

    int GetLineBefore(const wxGrid* grid, int line) const override
    {
        const int posBefore = grid->GetColPos(line) - 1;
        return posBefore >= 0 ? grid->GetColAt(posBefore) : wxNOT_FOUND;
    }

    wxWindow *GetHeaderWindow(wxGrid *grid) const override
        { return grid->GetGridColLabelWindow(); }
    int GetHeaderWindowSize(wxGrid *grid) const override
        { return grid->GetColLabelSize(); }

    wxGridWindow *GetFrozenGrid(wxGrid* grid) const override
        { return (wxGridWindow*)grid->GetFrozenColGridWindow(); }
};

// This class abstracts the difference between operations going forward
// (down/right) and backward (up/left) and allows to use the same code for
// functions which differ only in the direction of grid traversal.
//
// Notice that all operations in this class work with display positions and not
// internal indices which can be different if the columns were reordered.
//
// Like wxGridOperations it's an ABC with two concrete subclasses below. Unlike
// it, this is a normal object and not just a function dispatch table and has a
// non-default ctor.
//
// Note: the explanation of this discrepancy is the existence of (very useful)
// Dual() method in wxGridOperations which forces us to make wxGridOperations a
// function dispatcher only.
class wxGridDirectionOperations
{
public:
    // The oper parameter to ctor selects whether we work with rows or columns
    wxGridDirectionOperations(wxGrid *grid, const wxGridOperations& oper)
        : m_grid{grid},
          m_oper{oper}
    {
    }

    // Check if the component of this point in our direction is at the
    // boundary, i.e. is the first/last row/column
    virtual bool IsAtBoundary(const wxGridCellCoords& coords) const = 0;

    // Check if the component of this point in our direction is
    // valid, i.e. not -1
    bool IsValid(const wxGridCellCoords& coords) const
    {
        return m_oper.Select(coords) != -1;
    }

    // Make the coordinates with the other component value of -1.
    wxGridCellCoords MakeWholeLineCoords(const wxGridCellCoords& coords) const
    {
        return m_oper.MakeCoords(m_oper.Select(coords), -1);
    }

    // Increment the component of this point in our direction
    //
    // Note that this can't be called if IsAtBoundary() is true, use
    // TryToAdvance() if this might be the case.
    virtual void Advance(wxGridCellCoords& coords) const = 0;

    // Try to advance in our direction, return true if succeeded or false
    // otherwise, i.e. if the coordinates are already at the grid boundary.
    bool TryToAdvance(wxGridCellCoords& coords) const
    {
        if ( IsAtBoundary(coords) )
            return false;

        Advance(coords);

        return true;
    }

    // Find the line at the given distance, in pixels, away from this one
    // (this uses clipping, i.e. anything after the last line is counted as the
    // last one and anything before the first one as 0)
    //
    // TODO: Implementation of this method currently doesn't support column
    //       reordering as it mixes up indices and positions. But this doesn't
    //       really matter as it's only called for rows (Page Up/Down only work
    //       vertically) and row reordering is not currently supported. We'd
    //       need to fix it if this ever changes however.
    virtual int MoveByPixelDistance(int line, int distance) const = 0;

    // This class is never used polymorphically but give it a virtual dtor
    // anyhow to suppress g++ complaints about it
    virtual ~wxGridDirectionOperations() = default;

protected:
    // Get the position of the row or column from the given coordinates pair.
    //
    // This is just a shortcut to avoid repeating m_oper and m_grid multiple
    // times in the derived classes code.
    int GetLinePos(const wxGridCellCoords& coords) const
    {
        return m_oper.GetLinePos(m_grid, m_oper.Select(coords));
    }

    // Get the index of the row or column from the position.
    int GetLineAt(int pos) const
    {
        return m_oper.GetLineAt(m_grid, pos);
    }

    // Check if the given line is visible, i.e. has non 0 size.
    bool IsLineVisible(int line) const
    {
        return m_oper.GetLineSize(m_grid, line) != 0;
    }

    wxGrid * const m_grid;
    const wxGridOperations& m_oper;
};

class wxGridBackwardOperations : public wxGridDirectionOperations
{
public:
    wxGridBackwardOperations(wxGrid *grid, const wxGridOperations& oper)
        : wxGridDirectionOperations(grid, oper)
    {
    }

    bool IsAtBoundary(const wxGridCellCoords& coords) const override
    {
        wxASSERT_MSG( m_oper.Select(coords) >= 0, "invalid row/column" );

        int pos = GetLinePos(coords);
        while ( pos )
        {
            // Check the previous line.
            const int line = GetLineAt(--pos);
            if ( IsLineVisible(line) )
            {
                // There is another visible line before this one, hence it's
                // not at boundary.
                return false;
            }
        }

        // We reached the boundary without finding any visible lines.
        return true;
    }

    void Advance(wxGridCellCoords& coords) const override
    {
        int pos = GetLinePos(coords);
        for ( ;; )
        {
            // This is not supposed to happen if IsAtBoundary() returned false.
            wxCHECK_RET( pos, "can't advance when already at boundary" );

            const int line = GetLineAt(--pos);
            if ( IsLineVisible(line) )
            {
                m_oper.Set(coords, line);
                break;
            }
        }
    }

    int MoveByPixelDistance(int line, int distance) const override
    {
        const int pos = m_oper.GetLineStartPos(m_grid, line);
        return m_oper.PosToLine(m_grid, pos - distance + 1, nullptr, true);
    }
};

// Please refer to the comments above when reading this class code, it's
// absolutely symmetrical to wxGridBackwardOperations.
class wxGridForwardOperations : public wxGridDirectionOperations
{
public:
    wxGridForwardOperations(wxGrid *grid, const wxGridOperations& oper)
        : wxGridDirectionOperations(grid, oper),
          m_numLines(oper.GetTotalNumberOfLines(grid))
    {
    }

    bool IsAtBoundary(const wxGridCellCoords& coords) const override
    {
        wxASSERT_MSG( m_oper.Select(coords) < m_numLines, "invalid row/column" );

        int pos = GetLinePos(coords);
        while ( pos < m_numLines - 1 )
        {
            if ( IsLineVisible(GetLineAt(++pos)) )
                return false;
        }

        return true;
    }

    void Advance(wxGridCellCoords& coords) const override
    {
        int pos = GetLinePos(coords);
        for ( ;; )
        {
            wxCHECK_RET( pos < m_numLines - 1,
                         "can't advance when already at boundary" );

            const int line = GetLineAt(++pos);
            if ( IsLineVisible(line) )
            {
                m_oper.Set(coords, line);
                break;
            }
        }
    }

    int MoveByPixelDistance(int line, int distance) const override
    {
        const int pos = m_oper.GetLineStartPos(m_grid, line);
        return m_oper.PosToLine(m_grid, pos + distance, nullptr, true);
    }

private:
    const int m_numLines;
};

// ----------------------------------------------------------------------------
// data structures used for the data type registry
// ----------------------------------------------------------------------------

struct wxGridDataTypeInfo
{
    wxGridDataTypeInfo(std::string_view typeName,
                       wxGridCellRenderer* renderer,
                       wxGridCellEditor* editor)
        : m_typeName(typeName), m_renderer(renderer), m_editor(editor)
        {}

    ~wxGridDataTypeInfo()
    {
        if (m_renderer)
            m_renderer->DecRef();
        if (m_editor)
            m_editor->DecRef();
    }

    wxGridDataTypeInfo(const wxGridDataTypeInfo&) = delete;
	wxGridDataTypeInfo& operator=(const wxGridDataTypeInfo&) = delete;

    std::string         m_typeName;
    wxGridCellRenderer* m_renderer;
    wxGridCellEditor*   m_editor;
};

using wxGridDataTypeInfoArray = std::vector<wxGridDataTypeInfo*>;

class wxGridTypeRegistry
{
public:
    wxGridTypeRegistry() = default;
    ~wxGridTypeRegistry();

    void RegisterDataType(std::string_view typeName,
                     wxGridCellRenderer* renderer,
                     wxGridCellEditor* editor);

    // find one of already registered data types
    int FindRegisteredDataType(std::string_view typeName);

    // try to FindRegisteredDataType(), if this fails and typeName is one of
    // standard typenames, register it and return its index
    int FindDataType(std::string_view typeName);

    // try to FindDataType(), if it fails see if it is not one of already
    // registered data types with some params in which case clone the
    // registered data type and set params for it
    int FindOrCloneDataType(std::string_view typeName);

    wxGridCellRenderer* GetRenderer(int index);
    wxGridCellEditor*   GetEditor(int index);

private:
    wxGridDataTypeInfoArray m_typeinfo;
};

// Returns the rectangle for showing something of the given size in a cell with
// the given alignment.
//
// The function is used by wxGridCellBoolEditor and wxGridCellBoolRenderer to
// draw a check mark and position wxCheckBox respectively.
wxRect
wxGetContentRect(wxSize contentSize,
                 const wxRect& cellRect,
                 int hAlign,
                 int vAlign);

namespace wxGridPrivate
{

#if wxUSE_DATETIME

// This is used as TryGetValueAsDate() parameter.
class DateParseParams
{
public:
    // Unfortunately we have to provide the default ctor (and also make the
    // members non-const) because we use these objects as out-parameters as
    // they are not fully declared in the public headers. The factory functions
    // below must be used to create a really usable object.
    DateParseParams()  = default;

    // Use these functions to really initialize the object.
    static DateParseParams WithFallback(const std::string& format)
    {
        return DateParseParams(format, true);
    }

    static DateParseParams WithoutFallback(const std::string& format)
    {
        return DateParseParams(format, false);
    }

    // The usual format, e.g. "%x" or "%Y-%m-%d".
    std::string format;

    // Whether fall back to ParseDate() is allowed.
    bool fallbackParseDate{false};

private:
    DateParseParams(const std::string& format_, bool fallbackParseDate_)
        : format(format_),
          fallbackParseDate(fallbackParseDate_)
    {
    }
};

// Helper function trying to get a date from the given cell: if possible, get
// the date value from the table directly, otherwise get the string value for
// this cell and try to parse it using the specified date format and, if this
// doesn't work and fallbackParseDate is true, try using ParseDate() as a
// fallback. If this still fails, returns false.
bool
TryGetValueAsDate(wxDateTime& result,
                  const DateParseParams& params,
                  const wxGrid& grid,
                  int row, int col);

#endif // wxUSE_DATETIME

} // namespace wxGridPrivate

#endif // wxUSE_GRID
#endif // _WX_GENERIC_GRID_PRIVATE_H_
