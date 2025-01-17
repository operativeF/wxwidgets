///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/headerctrlg.h
// Purpose:     Generic wxHeaderCtrl implementation
// Author:      Vadim Zeitlin
// Created:     2008-12-01
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_HEADERCTRLG_H_
#define _WX_GENERIC_HEADERCTRLG_H_

#include "wx/event.h"
import <vector>;
#include "wx/overlay.h"

// ----------------------------------------------------------------------------
// wxHeaderCtrl
// ----------------------------------------------------------------------------

class wxHeaderCtrl : public wxHeaderCtrlBase
{
public:
    wxHeaderCtrl()
    {
        Init();
    }

    wxHeaderCtrl(wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxHD_DEFAULT_STYLE,
                 std::string_view name = wxHeaderCtrlNameStr)
    {
        Init();

        Create(parent, id, pos, size, style, name);
    }

    wxHeaderCtrl(wxHeaderCtrl&&) = delete;
    
    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxHD_DEFAULT_STYLE,
                std::string_view name = wxHeaderCtrlNameStr);

    ~wxHeaderCtrl();

protected:
    wxSize DoGetBestSize() const override;


private:
    
    void DoSetCount(unsigned int count) override;
    unsigned int DoGetCount() const override;
    void DoUpdate(unsigned int idx) override;

    void DoScrollHorz(int dx) override;

    void DoSetColumnsOrder(const std::vector<int>& order) override;
    std::vector<int> DoGetColumnsOrder() const override;

    // common part of all ctors
    void Init();

    // event handlers
    void OnPaint(wxPaintEvent& event);
    void OnMouse(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnCaptureLost(wxMouseCaptureLostEvent& event);

    // move the column with given idx at given position (this doesn't generate
    // any events but does refresh the display)
    void DoMoveCol(unsigned int idx, unsigned int pos);

    // return the horizontal start position of the given column in physical
    // coordinates
    int GetColStart(unsigned int idx) const;

    // and the end position
    int GetColEnd(unsigned int idx) const;

    // refresh the given column [only]; idx must be valid
    void RefreshCol(unsigned int idx);

    // refresh the given column if idx is valid
    void RefreshColIfNotNone(unsigned int idx);

    // refresh all the controls starting from (and including) the given one
    void RefreshColsAfter(unsigned int idx);

    // return the column at the given position or -1 if it is beyond the
    // rightmost column and put true into onSeparator output parameter if the
    // position is near the divider at the right end of this column (notice
    // that this means that we return column 0 even if the position is over
    // column 1 but close enough to the divider separating it from column 0)
    unsigned int FindColumnAtPoint(int x, bool *onSeparator = NULL) const;

    // return the result of FindColumnAtPoint() if it is a valid column,
    // otherwise the index of the last (rightmost) displayed column
    unsigned int FindColumnClosestToPoint(int xPhysical) const;

    // return true if a drag resizing operation is currently in progress
    bool IsResizing() const;

    // return true if a drag reordering operation is currently in progress
    bool IsReordering() const;

    // return true if any drag operation is currently in progress
    bool IsDragging() const { return IsResizing() || IsReordering(); }

    // end any drag operation currently in progress (resizing or reordering)
    void EndDragging();

    // cancel the drag operation currently in progress and generate an event
    // about it
    void CancelDragging();

    // start (if m_colBeingResized is -1) or continue resizing the column
    //
    // this generates wxEVT_HEADER_BEGIN_RESIZE/RESIZING events and can
    // cancel the operation if the user handler decides so
    void StartOrContinueResizing(unsigned int col, int xPhysical);

    // end the resizing operation currently in progress and generate an event
    // about it with its cancelled flag set if xPhysical is -1
    void EndResizing(int xPhysical);

    // same functions as above but for column moving/reordering instead of
    // resizing
    void StartReordering(unsigned int col, int xPhysical);

    // returns true if we did drag the column somewhere else or false if we
    // didn't really move it -- in this case we consider that no reordering
    // took place and that a normal column click event should be generated
    bool EndReordering(int xPhysical);

    // constrain the given position to be larger than the start position of the
    // given column plus its minimal width and return the effective width
    int ConstrainByMinWidth(unsigned int col, int& xPhysical);

    // update the information displayed while a column is being moved around
    void UpdateReorderingMarker(int xPhysical);

    // clear any overlaid markers
    void ClearMarkers();


    // number of columns in the control currently
    unsigned int m_numColumns;

    // index of the column under mouse or -1 if none
    unsigned int m_hover;

    // the column being resized or -1 if there is no resizing operation in
    // progress
    unsigned int m_colBeingResized;

    // the column being moved or -1 if there is no reordering operation in
    // progress
    unsigned int m_colBeingReordered;

    // the distance from the start of m_colBeingReordered and the mouse
    // position when the user started to drag it
    int m_dragOffset;

    // the horizontal scroll offset
    int m_scrollOffset;

    // the overlay display used during the dragging operations
    wxOverlay m_overlay;

    // the indices of the column appearing at the given position on the display
    // (its size is always m_numColumns)
    std::vector<int> m_colIndices;

    bool m_wasSeparatorDClick;

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_HEADERCTRLG_H_

