/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/splitter.h
// Purpose:     wxSplitterWindow class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_SPLITTER_H_
#define _WX_GENERIC_SPLITTER_H_

#include "wx/window.h"                      // base class declaration
#include "wx/containr.h"                    // wxControlContainer
#include "wx/pen.h"

#include <memory>

class wxSplitterEvent;

// ---------------------------------------------------------------------------
// splitter constants
// ---------------------------------------------------------------------------

enum class wxSplitMode
{
    Horizontal,
    Vertical
};

enum class wxSplitDragMode
{
    None,
    Dragging,
    LeftDown
};

// ---------------------------------------------------------------------------
// wxSplitterWindow maintains one or two panes, with
// an optional vertical or horizontal split which
// can be used with the mouse or programmatically.
// ---------------------------------------------------------------------------

// TODO:
// 1) Perhaps make the borders sensitive to dragging in order to create a split.
//    The MFC splitter window manages scrollbars as well so is able to
//    put sash buttons on the scrollbars, but we probably don't want to go down
//    this path.
// 2) for wxWidgets 2.0, we must find a way to set the WS_CLIPCHILDREN style
//    to prevent flickering. (WS_CLIPCHILDREN doesn't work in all cases so can't be
//    standard).

class wxSplitterWindow: public wxNavigationEnabled<wxWindow>
{
public:

////////////////////////////////////////////////////////////////////////////
// Public API

    // Default constructor
    wxSplitterWindow()
        : m_sashTrackerPen(std::make_unique<wxPen>(*wxBLACK, 2, wxPenStyle::Solid))
    {
    }

    // Normal constructor
    wxSplitterWindow(wxWindow *parent, wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxSP_3D,
                     const std::string& name = "splitter")
        : m_sashTrackerPen(std::make_unique<wxPen>(*wxBLACK, 2, wxPenStyle::Solid))
    {
        Create(parent, id, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxSP_3D,
                     const std::string& name = "splitter");

    // Gets the only or left/top pane
    wxWindow *GetWindow1() const { return m_windowOne; }

    // Gets the right/bottom pane
    wxWindow *GetWindow2() const { return m_windowTwo; }

    // Sets the split mode
    void SetSplitMode(wxSplitMode mode)
    {
        wxASSERT_MSG( mode == wxSplitMode::Vertical || mode == wxSplitMode::Horizontal,
                      "invalid split mode" );

        m_splitMode = (wxSplitMode)mode;
    }

    // Gets the split mode
    wxSplitMode GetSplitMode() const { return m_splitMode; }

    // Initialize with one window
    void Initialize(wxWindow *window);

    // Associates the given window with window 2, drawing the appropriate sash
    // and changing the split mode.
    // Does nothing and returns false if the window is already split.
    // A sashPosition of 0 means choose a default sash position,
    // negative sashPosition specifies the size of right/lower pane as its
    // absolute value rather than the size of left/upper pane.
    virtual bool SplitVertically(wxWindow *window1,
                                 wxWindow *window2,
                                 int sashPosition = 0)
        { return DoSplit(wxSplitMode::Vertical, window1, window2, sashPosition); }
    virtual bool SplitHorizontally(wxWindow *window1,
                                   wxWindow *window2,
                                   int sashPosition = 0)
        { return DoSplit(wxSplitMode::Horizontal, window1, window2, sashPosition); }

    // Removes the specified (or second) window from the view
    // Doesn't actually delete the window.
    bool Unsplit(wxWindow *toRemove = nullptr);

    // Replaces one of the windows with another one (neither old nor new
    // parameter should be NULL)
    bool ReplaceWindow(wxWindow *winOld, wxWindow *winNew);

    // Make sure the child window sizes are updated. This is useful
    // for reducing flicker by updating the sizes before a
    // window is shown, if you know the overall size is correct.
    void UpdateSize();

    // Is the window split?
    bool IsSplit() const { return (m_windowTwo != nullptr); }

    // Sets the border size
    void SetBorderSize([[maybe_unused]] int width) { }

    // Hide or show the sash and test whether it's currently hidden.
    void SetSashInvisible(bool invisible = true);
    bool IsSashInvisible() const { return HasFlag(wxSP_NOSASH); }

    // Gets the current sash size which may be 0 if it's hidden and the default
    // sash size.
    int GetSashSize() const;
    int GetDefaultSashSize() const;

    // Gets the border size
    int GetBorderSize() const;

    // Set the sash position
    void SetSashPosition(int position, bool redraw = true);

    // Gets the sash position
    int GetSashPosition() const { return m_sashPosition; }

    // Set the sash gravity
    void SetSashGravity(double gravity);

    // Gets the sash gravity
    double GetSashGravity() const { return m_sashGravity; }

    // If this is zero, we can remove panes by dragging the sash.
    void SetMinimumPaneSize(int min);
    int GetMinimumPaneSize() const { return m_minimumPaneSize; }

    // NB: the OnXXX() functions below are for backwards compatibility only,
    //     don't use them in new code but handle the events instead!

    // called when the sash position is about to change, may return a new value
    // for the sash or -1 to prevent the change from happening at all
    virtual int OnSashPositionChanging(int newSashPosition);

    // Called when the sash position is about to be changed, return
    // false from here to prevent the change from taking place.
    // Repositions sash to minimum position if pane would be too small.
    // newSashPosition here is always positive or zero.
    virtual bool OnSashPositionChange(int newSashPosition);

    // If the sash is moved to an extreme position, a subwindow
    // is removed from the splitter window, and the app is
    // notified. The app should delete or hide the window.
    virtual void OnUnsplit(wxWindow *removed);

    // Called when the sash is double-clicked.
    // The default behaviour is to remove the sash if the
    // minimum pane size is zero.
    virtual void OnDoubleClickSash(int x, int y);

////////////////////////////////////////////////////////////////////////////
// Implementation

    // Paints the border and sash
    void OnPaint(wxPaintEvent& event);

    // Handles mouse events
    void OnMouseEvent(wxMouseEvent& ev);

    // Aborts dragging mode
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    // Adjusts the panes
    void OnSize(wxSizeEvent& event);

    // In live mode, resize child windows in idle time
    void OnInternalIdle() override;

    // Draws the sash
    virtual void DrawSash(wxDC& dc);

    // Draws the sash tracker (for whilst moving the sash)
    virtual void DrawSashTracker(int x, int y);

    // Tests for pt over sash
    virtual bool SashHitTest(wxPoint pt);

    // Resizes subwindows
    virtual void SizeWindows();

#ifdef __WXMAC__
    bool MacClipGrandChildren() const override { return true ; }
#endif

protected:
    // event handlers
#if defined(__WXMSW__) || defined(__WXMAC__)
    void OnSetCursor(wxSetCursorEvent& event);
#endif // wxMSW

    // send the given event, return false if the event was processed and vetoed
    // by the user code
    [[maybe_unused]] bool DoSendEvent(wxSplitterEvent& event);

    // common part of all ctors
    void Init();

    // common part of SplitVertically() and SplitHorizontally()
    bool DoSplit(wxSplitMode mode,
                 wxWindow *window1, wxWindow *window2,
                 int sashPosition);

    // adjusts sash position with respect to min. pane and window sizes
    int AdjustSashPosition(int sashPos) const;

    // get either width or height depending on the split mode
    int GetWindowSize() const;

    // convert the user specified sash position which may be > 0 (as is), < 0
    // (specifying the size of the right pane) or 0 (use default) to the real
    // position to be passed to DoSetSashPosition()
    int ConvertSashPosition(int sashPos) const;

    // set the real sash position, sashPos here must be positive
    //
    // returns true if the sash position has been changed, false otherwise
    bool DoSetSashPosition(int sashPos);

    // set the sash position and send an event about it having been changed
    void SetSashPositionAndNotify(int sashPos);

    // callbacks executed when we detect that the mouse has entered or left
    // the sash
    virtual void OnEnterSash();
    virtual void OnLeaveSash();

    // set the cursor appropriate for the current split mode
    void SetResizeCursor();

    // redraw the splitter if its "hotness" changed if necessary
    void RedrawIfHotSensitive(bool isHot);

    // return the best size of the splitter equal to best sizes of its
    // subwindows
    wxSize DoGetBestSize() const override;

    wxCursor    m_sashCursorWE{wxCURSOR_SIZEWE};
    wxCursor    m_sashCursorNS{wxCURSOR_SIZENS};

    wxPoint     m_ptStart;      // mouse position when dragging started
    wxSize      m_lastSize;

    std::unique_ptr<wxPen> m_sashTrackerPen;

    wxWindow*   m_windowOne{nullptr};
    wxWindow*   m_windowTwo{nullptr};
    
    double      m_sashGravity{0.0};

    int         m_oldX{0};         // current tracker position if not live mode
    int         m_oldY{0};         // current tracker position if not live mode
    int         m_sashPosition{0}; // Number of pixels from left or top
    int         m_requestedSashPosition{std::numeric_limits<int>::max()};
    int         m_sashPositionCurrent; // while dragging
    int         m_sashStart{0};    // sash position when dragging started
    int         m_minimumPaneSize{0};

    wxSplitDragMode m_dragMode{wxSplitDragMode::None};
    wxSplitMode m_splitMode{wxSplitMode::Vertical};

    // when in live mode, set this to true to resize children in idle
    bool        m_needUpdating{false};
    bool        m_permitUnsplitAlways{true};
    bool        m_isHot{false};

private:
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// event class and macros
// ----------------------------------------------------------------------------

// we reuse the same class for all splitter event types because this is the
// usual wxWin convention, but the three event types have different kind of
// data associated with them, so the accessors can be only used if the real
// event type matches with the one for which the accessors make sense
class wxSplitterEvent : public wxNotifyEvent
{
public:
    wxSplitterEvent(wxEventType type = wxEVT_NULL,
                    wxSplitterWindow *splitter = nullptr)
        : wxNotifyEvent(type)
    {
        SetEventObject(splitter);
        if (splitter) m_id = splitter->GetId();
    }
    wxSplitterEvent(const wxSplitterEvent& event) = default;

    // SASH_POS_CHANGED methods

    // setting the sash position to -1 prevents the change from taking place at
    // all
    void SetSashPosition(int pos)
    {
        wxASSERT( GetEventType() == wxEVT_SPLITTER_SASH_POS_CHANGED
                || GetEventType() == wxEVT_SPLITTER_SASH_POS_CHANGING);

        m_data.pos = pos;
    }

    int GetSashPosition() const
    {
        wxASSERT( GetEventType() == wxEVT_SPLITTER_SASH_POS_CHANGED
                || GetEventType() == wxEVT_SPLITTER_SASH_POS_CHANGING);

        return m_data.pos;
    }

    // UNSPLIT event methods
    wxWindow *GetWindowBeingRemoved() const
    {
        wxASSERT( GetEventType() == wxEVT_SPLITTER_UNSPLIT );

        return m_data.win;
    }

    // DCLICK event methods
    int GetX() const
    {
        wxASSERT( GetEventType() == wxEVT_SPLITTER_DOUBLECLICKED );

        return m_data.pt.x;
    }

    int GetY() const
    {
        wxASSERT( GetEventType() == wxEVT_SPLITTER_DOUBLECLICKED );

        return m_data.pt.y;
    }

    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxSplitterEvent>(*this); }

private:
    friend class wxSplitterWindow;

    // data for the different types of event
    union
    {
        struct
        {
            int x, y;
        } pt;               // position of double click for DCLICK event
        wxWindow *win;      // window being removed for UNSPLIT event
        int pos;            // position for SASH_POS_CHANGED event
    } m_data;

public:
	wxSplitterEvent& operator=(const wxSplitterEvent&) = delete;
};

typedef void (wxEvtHandler::*wxSplitterEventFunction)(wxSplitterEvent&);

#define wxSplitterEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSplitterEventFunction, func)

#define wx__DECLARE_SPLITTEREVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_SPLITTER_ ## evt, id, wxSplitterEventHandler(fn))

#define EVT_SPLITTER_SASH_POS_CHANGED(id, fn) \
    wx__DECLARE_SPLITTEREVT(SASH_POS_CHANGED, id, fn)

#define EVT_SPLITTER_SASH_POS_CHANGING(id, fn) \
    wx__DECLARE_SPLITTEREVT(SASH_POS_CHANGING, id, fn)

#define EVT_SPLITTER_DCLICK(id, fn) \
    wx__DECLARE_SPLITTEREVT(DOUBLECLICKED, id, fn)

#define EVT_SPLITTER_UNSPLIT(id, fn) \
    wx__DECLARE_SPLITTEREVT(UNSPLIT, id, fn)

#endif // _WX_GENERIC_SPLITTER_H_
