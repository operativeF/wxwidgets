/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/grid.h
// Purpose:     wxGrid and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: Santiago Palacios
// Created:     1/08/1999
// Copyright:   (c) Michael Bedward
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/control.h"
#include "wx/event.h"
#include "wx/kbdstate.h"

export module WX.Grid.Event;

import WX.Grid.CellCoords;

export
{

class wxGridEvent : public wxNotifyEvent,
                                    public wxKeyboardState
{
public:
    wxGridEvent() = default;
    wxGridEvent(int id,
                wxEventType type,
                wxObject* obj,
                int row = -1, int col = -1,
                int x = -1, int y = -1,
                bool sel = true,
                const wxKeyboardState& kbd = wxKeyboardState())
        : wxNotifyEvent(type, id),
          wxKeyboardState(kbd),
          m_row{row},
          m_col{col},
          m_x{x},
          m_y{y},
          m_selecting{sel}
    {
        SetEventObject(obj);
    }

    wxGridEvent& operator=(const wxGridEvent&) = delete;

    int GetRow() const { return m_row; }
    int GetCol() const { return m_col; }
    wxPoint GetPosition() const { return wxPoint( m_x, m_y ); }
    bool Selecting() const { return m_selecting; }

    wxEvent *Clone() const override { return new wxGridEvent(*this); }

protected:
    int         m_row{-1};
    int         m_col{-1};
    int         m_x{-1};
    int         m_y{-1};

    bool        m_selecting{false};
};

class wxGridSizeEvent : public wxNotifyEvent,
                                        public wxKeyboardState
{
public:
    wxGridSizeEvent() = default;
    wxGridSizeEvent(int id,
                    wxEventType type,
                    wxObject* obj,
                    int rowOrCol = -1,
                    int x = -1, int y = -1,
                    const wxKeyboardState& kbd = wxKeyboardState())
        : wxNotifyEvent(type, id),
          wxKeyboardState(kbd),
          m_rowOrCol{rowOrCol},
          m_x{x},
          m_y{y}
    {
        SetEventObject(obj);
    }

    wxGridSizeEvent& operator=(const wxGridSizeEvent&) = delete;

    int GetRowOrCol() const { return m_rowOrCol; }
    wxPoint GetPosition() const { return wxPoint( m_x, m_y ); }

    wxEvent *Clone() const override { return new wxGridSizeEvent(*this); }

protected:
    int         m_rowOrCol{-1};
    int         m_x{-1};
    int         m_y{-1};
};


class wxGridRangeSelectEvent : public wxNotifyEvent,
                                               public wxKeyboardState
{
public:
    wxGridRangeSelectEvent()
         
    {
        Init(wxGridNoCellCoords, wxGridNoCellCoords, false);
    }

    wxGridRangeSelectEvent(int id,
                           wxEventType type,
                           wxObject* obj,
                           const wxGridCellCoords& topLeft,
                           const wxGridCellCoords& bottomRight,
                           bool sel = true,
                           const wxKeyboardState& kbd = wxKeyboardState())
        : wxNotifyEvent(type, id),
          wxKeyboardState(kbd)
    {
        Init(topLeft, bottomRight, sel);

        SetEventObject(obj);
    }

    wxGridRangeSelectEvent& operator=(const wxGridRangeSelectEvent&) = delete;

    wxGridCellCoords GetTopLeftCoords() const { return m_topLeft; }
    wxGridCellCoords GetBottomRightCoords() const { return m_bottomRight; }
    int GetTopRow() const { return m_topLeft.GetRow(); }
    int GetBottomRow() const { return m_bottomRight.GetRow(); }
    int GetLeftCol() const { return m_topLeft.GetCol(); }
    int GetRightCol() const { return m_bottomRight.GetCol(); }
    bool Selecting() const { return m_selecting; }

    wxEvent *Clone() const override { return new wxGridRangeSelectEvent(*this); }

protected:
    void Init(const wxGridCellCoords& topLeft,
              const wxGridCellCoords& bottomRight,
              bool selecting)
    {
        m_topLeft = topLeft;
        m_bottomRight = bottomRight;
        m_selecting = selecting;
    }

    wxGridCellCoords  m_topLeft;
    wxGridCellCoords  m_bottomRight;
    bool              m_selecting;
};


class wxGridEditorCreatedEvent : public wxCommandEvent
{
public:
    wxGridEditorCreatedEvent() = default;
    wxGridEditorCreatedEvent(int id, wxEventType type, wxObject* obj,
                             int row, int col, wxWindow* window);

    wxGridEditorCreatedEvent& operator=(const wxGridEditorCreatedEvent&) = delete;

    int GetRow() const { return m_row; }
    int GetCol() const { return m_col; }
    wxWindow* GetWindow() const { return m_window; }
    void SetRow(int row)                { m_row = row; }
    void SetCol(int col)                { m_col = col; }
    void SetWindow(wxWindow* window)    { m_window = window; }

    wxEvent *Clone() const override { return new wxGridEditorCreatedEvent(*this); }

private:
    wxWindow* m_window{ nullptr };

    int m_row{};
    int m_col{};
};

inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_LEFT_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_RIGHT_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_LEFT_DCLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_RIGHT_DCLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_BEGIN_DRAG( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_LABEL_LEFT_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_LABEL_RIGHT_CLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_LABEL_LEFT_DCLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_LABEL_RIGHT_DCLICK( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_COL_MOVE( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_COL_SORT( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_CHANGING( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_CELL_CHANGED( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_SELECT_CELL( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_EDITOR_SHOWN( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_EDITOR_HIDDEN( wxNewEventType() );
inline const wxEventTypeTag<wxGridEvent> wxEVT_GRID_TABBING( wxNewEventType() );

inline const wxEventTypeTag<wxGridSizeEvent> wxEVT_GRID_ROW_SIZE( wxNewEventType() );
inline const wxEventTypeTag<wxGridSizeEvent> wxEVT_GRID_COL_SIZE( wxNewEventType() );
inline const wxEventTypeTag<wxGridSizeEvent> wxEVT_GRID_COL_AUTO_SIZE( wxNewEventType() );

inline const wxEventTypeTag<wxGridRangeSelectEvent> wxEVT_GRID_RANGE_SELECTING( wxNewEventType() );
inline const wxEventTypeTag<wxGridRangeSelectEvent> wxEVT_GRID_RANGE_SELECTED( wxNewEventType() );

inline const wxEventTypeTag<wxGridEditorCreatedEvent> wxEVT_GRID_EDITOR_CREATED( wxNewEventType() );

typedef void (wxEvtHandler::*wxGridEventFunction)(wxGridEvent&);
typedef void (wxEvtHandler::*wxGridSizeEventFunction)(wxGridSizeEvent&);
typedef void (wxEvtHandler::*wxGridRangeSelectEventFunction)(wxGridRangeSelectEvent&);
typedef void (wxEvtHandler::*wxGridEditorCreatedEventFunction)(wxGridEditorCreatedEvent&);

#define wxGridEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxGridEventFunction, func)

#define wxGridSizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxGridSizeEventFunction, func)

#define wxGridRangeSelectEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxGridRangeSelectEventFunction, func)

#define wxGridEditorCreatedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxGridEditorCreatedEventFunction, func)

#define wx__DECLARE_GRIDEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_GRID_ ## evt, id, wxGridEventHandler(fn))

#define wx__DECLARE_GRIDSIZEEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_GRID_ ## evt, id, wxGridSizeEventHandler(fn))

#define wx__DECLARE_GRIDRANGESELEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_GRID_ ## evt, id, wxGridRangeSelectEventHandler(fn))

#define wx__DECLARE_GRIDEDITOREVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_GRID_ ## evt, id, wxGridEditorCreatedEventHandler(fn))

#define EVT_GRID_CMD_CELL_LEFT_CLICK(id, fn)     wx__DECLARE_GRIDEVT(CELL_LEFT_CLICK, id, fn)
#define EVT_GRID_CMD_CELL_RIGHT_CLICK(id, fn)    wx__DECLARE_GRIDEVT(CELL_RIGHT_CLICK, id, fn)
#define EVT_GRID_CMD_CELL_LEFT_DCLICK(id, fn)    wx__DECLARE_GRIDEVT(CELL_LEFT_DCLICK, id, fn)
#define EVT_GRID_CMD_CELL_RIGHT_DCLICK(id, fn)   wx__DECLARE_GRIDEVT(CELL_RIGHT_DCLICK, id, fn)
#define EVT_GRID_CMD_LABEL_LEFT_CLICK(id, fn)    wx__DECLARE_GRIDEVT(LABEL_LEFT_CLICK, id, fn)
#define EVT_GRID_CMD_LABEL_RIGHT_CLICK(id, fn)   wx__DECLARE_GRIDEVT(LABEL_RIGHT_CLICK, id, fn)
#define EVT_GRID_CMD_LABEL_LEFT_DCLICK(id, fn)   wx__DECLARE_GRIDEVT(LABEL_LEFT_DCLICK, id, fn)
#define EVT_GRID_CMD_LABEL_RIGHT_DCLICK(id, fn)  wx__DECLARE_GRIDEVT(LABEL_RIGHT_DCLICK, id, fn)
#define EVT_GRID_CMD_ROW_SIZE(id, fn)            wx__DECLARE_GRIDSIZEEVT(ROW_SIZE, id, fn)
#define EVT_GRID_CMD_COL_SIZE(id, fn)            wx__DECLARE_GRIDSIZEEVT(COL_SIZE, id, fn)
#define EVT_GRID_CMD_COL_AUTO_SIZE(id, fn)       wx__DECLARE_GRIDSIZEEVT(COL_AUTO_SIZE, id, fn)
#define EVT_GRID_CMD_COL_MOVE(id, fn)            wx__DECLARE_GRIDEVT(COL_MOVE, id, fn)
#define EVT_GRID_CMD_COL_SORT(id, fn)            wx__DECLARE_GRIDEVT(COL_SORT, id, fn)
#define EVT_GRID_CMD_RANGE_SELECTING(id, fn)     wx__DECLARE_GRIDRANGESELEVT(RANGE_SELECTING, id, fn)
#define EVT_GRID_CMD_RANGE_SELECTED(id, fn)      wx__DECLARE_GRIDRANGESELEVT(RANGE_SELECTED, id, fn)
#define EVT_GRID_CMD_CELL_CHANGING(id, fn)       wx__DECLARE_GRIDEVT(CELL_CHANGING, id, fn)
#define EVT_GRID_CMD_CELL_CHANGED(id, fn)        wx__DECLARE_GRIDEVT(CELL_CHANGED, id, fn)
#define EVT_GRID_CMD_SELECT_CELL(id, fn)         wx__DECLARE_GRIDEVT(SELECT_CELL, id, fn)
#define EVT_GRID_CMD_EDITOR_SHOWN(id, fn)        wx__DECLARE_GRIDEVT(EDITOR_SHOWN, id, fn)
#define EVT_GRID_CMD_EDITOR_HIDDEN(id, fn)       wx__DECLARE_GRIDEVT(EDITOR_HIDDEN, id, fn)
#define EVT_GRID_CMD_EDITOR_CREATED(id, fn)      wx__DECLARE_GRIDEDITOREVT(EDITOR_CREATED, id, fn)
#define EVT_GRID_CMD_CELL_BEGIN_DRAG(id, fn)     wx__DECLARE_GRIDEVT(CELL_BEGIN_DRAG, id, fn)
#define EVT_GRID_CMD_TABBING(id, fn)             wx__DECLARE_GRIDEVT(TABBING, id, fn)

// same as above but for any id (exists mainly for backwards compatibility but
// then it's also true that you rarely have multiple grid in the same window)
#define EVT_GRID_CELL_LEFT_CLICK(fn)     EVT_GRID_CMD_CELL_LEFT_CLICK(wxID_ANY, fn)
#define EVT_GRID_CELL_RIGHT_CLICK(fn)    EVT_GRID_CMD_CELL_RIGHT_CLICK(wxID_ANY, fn)
#define EVT_GRID_CELL_LEFT_DCLICK(fn)    EVT_GRID_CMD_CELL_LEFT_DCLICK(wxID_ANY, fn)
#define EVT_GRID_CELL_RIGHT_DCLICK(fn)   EVT_GRID_CMD_CELL_RIGHT_DCLICK(wxID_ANY, fn)
#define EVT_GRID_LABEL_LEFT_CLICK(fn)    EVT_GRID_CMD_LABEL_LEFT_CLICK(wxID_ANY, fn)
#define EVT_GRID_LABEL_RIGHT_CLICK(fn)   EVT_GRID_CMD_LABEL_RIGHT_CLICK(wxID_ANY, fn)
#define EVT_GRID_LABEL_LEFT_DCLICK(fn)   EVT_GRID_CMD_LABEL_LEFT_DCLICK(wxID_ANY, fn)
#define EVT_GRID_LABEL_RIGHT_DCLICK(fn)  EVT_GRID_CMD_LABEL_RIGHT_DCLICK(wxID_ANY, fn)
#define EVT_GRID_ROW_SIZE(fn)            EVT_GRID_CMD_ROW_SIZE(wxID_ANY, fn)
#define EVT_GRID_COL_SIZE(fn)            EVT_GRID_CMD_COL_SIZE(wxID_ANY, fn)
#define EVT_GRID_COL_AUTO_SIZE(fn)       EVT_GRID_CMD_COL_AUTO_SIZE(wxID_ANY, fn)
#define EVT_GRID_COL_MOVE(fn)            EVT_GRID_CMD_COL_MOVE(wxID_ANY, fn)
#define EVT_GRID_COL_SORT(fn)            EVT_GRID_CMD_COL_SORT(wxID_ANY, fn)
#define EVT_GRID_RANGE_SELECTING(fn)     EVT_GRID_CMD_RANGE_SELECTING(wxID_ANY, fn)
#define EVT_GRID_RANGE_SELECTED(fn)      EVT_GRID_CMD_RANGE_SELECTED(wxID_ANY, fn)
#define EVT_GRID_CELL_CHANGING(fn)       EVT_GRID_CMD_CELL_CHANGING(wxID_ANY, fn)
#define EVT_GRID_CELL_CHANGED(fn)        EVT_GRID_CMD_CELL_CHANGED(wxID_ANY, fn)
#define EVT_GRID_SELECT_CELL(fn)         EVT_GRID_CMD_SELECT_CELL(wxID_ANY, fn)
#define EVT_GRID_EDITOR_SHOWN(fn)        EVT_GRID_CMD_EDITOR_SHOWN(wxID_ANY, fn)
#define EVT_GRID_EDITOR_HIDDEN(fn)       EVT_GRID_CMD_EDITOR_HIDDEN(wxID_ANY, fn)
#define EVT_GRID_EDITOR_CREATED(fn)      EVT_GRID_CMD_EDITOR_CREATED(wxID_ANY, fn)
#define EVT_GRID_CELL_BEGIN_DRAG(fn)     EVT_GRID_CMD_CELL_BEGIN_DRAG(wxID_ANY, fn)
#define EVT_GRID_TABBING(fn)             EVT_GRID_CMD_TABBING(wxID_ANY, fn)

#if 0  // TODO: implement these ?  others ?

extern const int wxEVT_GRID_CREATE_CELL;
extern const int wxEVT_GRID_CHANGE_LABELS;
extern const int wxEVT_GRID_CHANGE_SEL_LABEL;

#define EVT_GRID_CREATE_CELL(fn)      wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_GRID_CREATE_CELL,      wxID_ANY, wxID_ANY, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxGridEventFunction, &fn ), NULL ),
#define EVT_GRID_CHANGE_LABELS(fn)    wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_GRID_CHANGE_LABELS,    wxID_ANY, wxID_ANY, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxGridEventFunction, &fn ), NULL ),
#define EVT_GRID_CHANGE_SEL_LABEL(fn) wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_GRID_CHANGE_SEL_LABEL, wxID_ANY, wxID_ANY, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxGridEventFunction, &fn ), NULL ),

#endif

} // export
