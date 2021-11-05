/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/splitter.cpp
// Purpose:     wxSplitterWindow implementation
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_SPLITTER

#include "wx/splitter.h"

#ifndef WX_PRECOMP
    #include <limits>
    #include <string>
#endif

#include "wx/utils.h"
#include "wx/log.h"

#include "wx/dcclient.h"
#include "wx/dcscreen.h"

#include "wx/window.h"
#include "wx/dialog.h"
#include "wx/frame.h"

#include "wx/settings.h"


#ifdef __WXOSX__
    #include "wx/osx/private/available.h"
#endif

#include "wx/renderer.h"

wxDEFINE_EVENT( wxEVT_SPLITTER_SASH_POS_CHANGED, wxSplitterEvent );
wxDEFINE_EVENT( wxEVT_SPLITTER_SASH_POS_CHANGING, wxSplitterEvent );
wxDEFINE_EVENT( wxEVT_SPLITTER_DOUBLECLICKED, wxSplitterEvent );
wxDEFINE_EVENT( wxEVT_SPLITTER_UNSPLIT, wxSplitterEvent );

wxIMPLEMENT_DYNAMIC_CLASS(wxSplitterWindow, wxWindow);

/*
    TODO PROPERTIES
        style wxSP_3D
        sashpos (long , 0 )
        minsize (long -1 )
        object, object_ref
        orientation
*/

wxIMPLEMENT_DYNAMIC_CLASS(wxSplitterEvent, wxNotifyEvent);

wxBEGIN_EVENT_TABLE(wxSplitterWindow, wxWindow)
    EVT_PAINT(wxSplitterWindow::OnPaint)
    EVT_SIZE(wxSplitterWindow::OnSize)
    EVT_MOUSE_EVENTS(wxSplitterWindow::OnMouseEvent)
    EVT_MOUSE_CAPTURE_LOST(wxSplitterWindow::OnMouseCaptureLost)

#if defined( __WXMSW__ ) || defined( __WXMAC__)
    EVT_SET_CURSOR(wxSplitterWindow::OnSetCursor)
#endif // wxMSW
wxEND_EVENT_TABLE()

static bool IsLive(wxSplitterWindow* wnd)
{
    // with wxSP_LIVE_UPDATE style the splitter windows are always resized
    // following the mouse movement while it drags the sash, without it we only
    // draw the sash at the new position but only resize the windows when the
    // dragging is finished
#if defined( __WXMAC__ ) && defined(TARGET_API_MAC_OSX) && TARGET_API_MAC_OSX == 1
    return true; // Mac can't paint outside paint event - always need live mode
#else
    return wnd->HasFlag(wxSP_LIVE_UPDATE);
#endif
}

bool wxSplitterWindow::Create(wxWindow *parent, wxWindowID id,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   unsigned int style,
                                   const std::string& name)
{
    // allow TABbing from one window to the other
    style |= wxTAB_TRAVERSAL;

    if ( !wxWindow::Create(parent, id, pos, size, style, name) )
        return false;

    m_lastSize = GetClientSize();

    m_permitUnsplitAlways = (style & wxSP_PERMIT_UNSPLIT) != 0;

#ifdef __WXOSX__
  #if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_16
    if ( WX_IS_MACOS_AVAILABLE(10, 16) )
    {
        // Nothing to do: see OnPaint()
    }
    else
  #endif
#endif
    {
        // FIXME: with this line the background is not erased at all under GTK1,
        //        so temporary avoid it there
#if !defined(__WXGTK__) || defined(__WXGTK20__)
        // don't erase the splitter background, it's pointless as we overwrite it
        // anyhow
        SetBackgroundStyle(wxBackgroundStyle::Paint);
#endif
    }

    return true;
}

// ----------------------------------------------------------------------------
// entering/leaving sash
// ----------------------------------------------------------------------------

void wxSplitterWindow::RedrawIfHotSensitive(bool isHot)
{
    if ( wxRendererNative::Get().GetSplitterParams(this).isHotSensitive )
    {
        m_isHot = isHot;

        wxClientDC dc(this);
        DrawSash(dc);
    }
    //else: we don't change our appearance, don't redraw to avoid flicker
}

void wxSplitterWindow::OnEnterSash()
{
    SetResizeCursor();

    RedrawIfHotSensitive(true);
}

void wxSplitterWindow::OnLeaveSash()
{
    SetCursor(*wxSTANDARD_CURSOR);

    RedrawIfHotSensitive(false);
}

void wxSplitterWindow::SetResizeCursor()
{
    SetCursor(m_splitMode == wxSplitMode::Vertical ? m_sashCursorWE
                                              : m_sashCursorNS);
}

// ----------------------------------------------------------------------------
// other event handlers
// ----------------------------------------------------------------------------

void wxSplitterWindow::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
#ifdef __WXOSX__
  #if __MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_16
    if ( WX_IS_MACOS_AVAILABLE(10, 16) )
    {
        // Nothing to do: since macOS 10.14, views are layer-backed or using a shared
        // layer and explicitly clearing the background isn't needed. This only
        // started mattering here with macOS 11 (aka 10.16 when built with older SDK),
        // where we must avoid explicitly painting window backgrounds
    }
    else
  #endif
    {
        // as subpanels might have a transparent background we must erase the background
        // at least on OSX, otherwise traces of the sash will remain
        // test with: splitter sample->replace right window
        dc.Clear();
    }
#endif // __WXOSX__

    DrawSash(dc);
}

void wxSplitterWindow::OnInternalIdle()
{
    wxWindow::OnInternalIdle();

    // We may need to update the children sizes if we're in the middle of
    // a live update as indicated by m_needUpdating. The other possible case,
    // when we have a requested but not yet set sash position (as indicated
    // by m_requestedSashPosition having a valid value) is handled by OnSize.
    if ( m_needUpdating )
    {
        m_needUpdating = false;
        SizeWindows();
    }
}

void wxSplitterWindow::OnMouseEvent(wxMouseEvent& event)
{
    int x = (int)event.GetX(),
        y = (int)event.GetY();

    if ( wxGetWindowStyle() & wxSP_NOSASH )
    {
        event.Skip();
        return;
    }

    bool isLive = IsLive(this);

    if (event.LeftDown())
    {
        if ( SashHitTest(wxPoint{x, y}) )
        {
            // Start the drag now
            m_dragMode = wxSplitDragMode::Dragging;

            // Capture mouse and set the cursor
            CaptureMouse();
            SetResizeCursor();

            if ( !isLive )
            {
                // remember the initial sash position and draw the initial
                // shadow sash
                m_sashPositionCurrent = m_sashPosition;

                m_oldX = (m_splitMode == wxSplitMode::Vertical ? m_sashPositionCurrent : x);
                m_oldY = (m_splitMode != wxSplitMode::Vertical ? m_sashPositionCurrent : y);
                DrawSashTracker(m_oldX, m_oldY);
            }

            m_ptStart = wxPoint(x,y);
            m_sashStart = m_sashPosition;
            return;
        }
    }
    else if (event.LeftUp() && m_dragMode == wxSplitDragMode::Dragging)
    {
        // We can stop dragging now and see what we've got.
        m_dragMode = wxSplitDragMode::None;

        // Release mouse and unset the cursor
        ReleaseMouse();
        SetCursor(* wxSTANDARD_CURSOR);

        // exit if unsplit after doubleclick
        if ( !IsSplit() )
        {
            return;
        }

        // Erase old tracker
        if ( !isLive )
        {
            DrawSashTracker(m_oldX, m_oldY);
        }

        // the position of the click doesn't exactly correspond to
        // m_sashPosition, rather it changes it by the distance by which the
        // mouse has moved
        int diff = m_splitMode == wxSplitMode::Vertical ? x - m_ptStart.x : y - m_ptStart.y;

        int posSashNew = OnSashPositionChanging(m_sashStart + diff);
        if ( posSashNew == -1 )
        {
            // change not allowed
            return;
        }

        if ( m_permitUnsplitAlways || m_minimumPaneSize == 0 )
        {
            // Deal with possible unsplit scenarios
            if ( posSashNew == 0 )
            {
                // We remove the first window from the view
                wxWindow *removedWindow = m_windowOne;
                m_windowOne = m_windowTwo;
                m_windowTwo = nullptr;
                OnUnsplit(removedWindow);
                wxSplitterEvent eventUnsplit(wxEVT_SPLITTER_UNSPLIT, this);
                eventUnsplit.m_data.win = removedWindow;
                DoSendEvent(eventUnsplit);
                SetSashPositionAndNotify(0);
            }
            else if ( posSashNew == GetWindowSize() )
            {
                // We remove the second window from the view
                wxWindow *removedWindow = m_windowTwo;
                m_windowTwo = nullptr;
                OnUnsplit(removedWindow);
                wxSplitterEvent eventUnsplit(wxEVT_SPLITTER_UNSPLIT, this);
                eventUnsplit.m_data.win = removedWindow;
                DoSendEvent(eventUnsplit);
                SetSashPositionAndNotify(0);
            }
            else
            {
                SetSashPositionAndNotify(posSashNew);
            }
        }
        else
        {
            SetSashPositionAndNotify(posSashNew);
        }

        SizeWindows();
    }  // left up && dragging
    else if ((event.Moving() || event.Leaving() || event.Entering()) && (m_dragMode == wxSplitDragMode::None))
    {
        if ( event.Leaving() || !SashHitTest(wxPoint{x, y}) )
            OnLeaveSash();
        else
            OnEnterSash();
    }
    else if (event.Dragging() && (m_dragMode == wxSplitDragMode::Dragging))
    {
        int diff = m_splitMode == wxSplitMode::Vertical ? x - m_ptStart.x : y - m_ptStart.y;

        int posSashNew = OnSashPositionChanging(m_sashStart + diff);
        if ( posSashNew == -1 )
        {
            // change not allowed
            return;
        }

        if ( !isLive )
        {
            if ( posSashNew == m_sashPositionCurrent )
                return;

            m_sashPositionCurrent = posSashNew;

            // Erase old tracker
            DrawSashTracker(m_oldX, m_oldY);

            m_oldX = (m_splitMode == wxSplitMode::Vertical ? m_sashPositionCurrent : x);
            m_oldY = (m_splitMode != wxSplitMode::Vertical ? m_sashPositionCurrent : y);

#ifdef __WXMSW__
            // As we captured the mouse, we may get the mouse events from outside
            // our window - for example, negative values in x, y. This has a weird
            // consequence under MSW where we use unsigned values sometimes and
            // signed ones other times: the coordinates turn as big positive
            // numbers and so the sash is drawn on the *right* side of the window
            // instead of the left (or bottom instead of top). Correct this.
            if ( (short)m_oldX < 0 )
                m_oldX = 0;
            if ( (short)m_oldY < 0 )
                m_oldY = 0;
#endif // __WXMSW__

            // Draw new one
            DrawSashTracker(m_oldX, m_oldY);
        }
        else
        {
            if ( posSashNew == m_sashPosition )
                return;

            DoSetSashPosition(posSashNew);

            // in live mode, the new position is the actual sash position, clear requested position!
            m_requestedSashPosition = std::numeric_limits<int>::max();
            m_needUpdating = true;
        }
    }
    else if ( event.LeftDClick() && m_windowTwo )
    {
        OnDoubleClickSash(x, y);
    }
    else
    {
        event.Skip();
    }
}

void wxSplitterWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
{
    if (m_dragMode != wxSplitDragMode::Dragging)
        return;

    m_dragMode = wxSplitDragMode::None;

    SetCursor(* wxSTANDARD_CURSOR);

    // Erase old tracker
    if ( !IsLive(this) )
    {
        DrawSashTracker(m_oldX, m_oldY);
    }
}

void wxSplitterWindow::OnSize(wxSizeEvent& event)
{
    // only process this message if we're not iconized - otherwise iconizing
    // and restoring a window containing the splitter has a funny side effect
    // of changing the splitter position!
    wxWindow *parent = wxGetTopLevelParent(this);
    bool iconized;

    wxTopLevelWindow *winTop = wxDynamicCast(parent, wxTopLevelWindow);
    if ( winTop )
    {
        iconized = winTop->IsIconized();
    }
    else
    {
        wxFAIL_MSG("should have a top level parent!");

        iconized = false;
    }

    if ( iconized )
    {
        m_lastSize = wxSize(0,0);

        event.Skip();

        return;
    }

    const wxSize curSize = event.GetSize();

    // Update the sash position if needed.
    //
    // Notice that we shouldn't do this if the sash position requested by user
    // couldn't be set yet as it would never be taken into account at all if we
    // modified it before this happens.
    if ( m_windowTwo && m_requestedSashPosition == std::numeric_limits<int>::max() )
    {
        int size = m_splitMode == wxSplitMode::Vertical ? curSize.x : curSize.y;

        int old_size = m_splitMode == wxSplitMode::Vertical ? m_lastSize.x : m_lastSize.y;

        // Don't do anything if the size didn't really change.
        if ( size != old_size )
        {
            int newPosition = -1;

            // Apply gravity if we use it.
            int delta = (int) ( (size - old_size)*m_sashGravity );
            if ( delta != 0 )
            {
                newPosition = m_sashPosition + delta;
                if( newPosition < m_minimumPaneSize )
                    newPosition = m_minimumPaneSize;
            }

            // Also check if the second window became too small.
            newPosition = AdjustSashPosition(newPosition == -1
                                                 ? m_sashPosition
                                                 : newPosition);
            if ( newPosition != m_sashPosition )
                SetSashPositionAndNotify(newPosition);
        }
    }

    m_lastSize = curSize;

    SizeWindows();
}

void wxSplitterWindow::SetSashGravity(double gravity)
{
    wxCHECK_RET( gravity >= 0. && gravity <= 1.,
                    "invalid gravity value" );

    m_sashGravity = gravity;
}

bool wxSplitterWindow::SashHitTest(wxPoint pt)
{
    if ( m_windowTwo == nullptr || m_sashPosition == 0)
        return false; // No sash

    int z = m_splitMode == wxSplitMode::Vertical ? pt.x : pt.y;
    int hitMax = m_sashPosition + GetSashSize() - 1;

    return z >= m_sashPosition && z <= hitMax;
}

void wxSplitterWindow::SetSashInvisible(bool invisible)
{
    if ( IsSashInvisible() != invisible )
        ToggleWindowStyle(wxSP_NOSASH);
}

int wxSplitterWindow::GetSashSize() const
{
    return IsSashInvisible() ? 0 : GetDefaultSashSize();
}

int wxSplitterWindow::GetDefaultSashSize() const
{
    return wxRendererNative::Get().GetSplitterParams(this).widthSash;
}

int wxSplitterWindow::GetBorderSize() const
{
    return wxRendererNative::Get().GetSplitterParams(this).border;
}

// Draw the sash
void wxSplitterWindow::DrawSash(wxDC& dc)
{
    if (HasFlag(wxSP_3DBORDER))
        wxRendererNative::Get().DrawSplitterBorder
                            (
                                this,
                                dc,
                                GetClientRect()
                            );

    // don't draw sash if we're not split
    if ( m_sashPosition == 0 || !m_windowTwo )
        return;

    // nor if we're configured to not show it
    if ( IsSashInvisible() )
        return;

    wxRendererNative::Get().DrawSplitterSash
                            (
                                this,
                                dc,
                                GetClientSize(),
                                m_sashPosition,
                                m_splitMode == wxSplitMode::Vertical ? wxVERTICAL
                                                                : wxHORIZONTAL,
                                m_isHot ? (int)wxCONTROL_CURRENT : 0
                            );
}

// Draw the sash tracker (for whilst moving the sash)
void wxSplitterWindow::DrawSashTracker(int x, int y)
{
    wxSize client_size = GetClientSize();

    wxScreenDC screenDC;
    int x1, y1;
    int x2, y2;

    if ( m_splitMode == wxSplitMode::Vertical )
    {
        x1 = x2 = std::clamp(x, 0, client_size.x) + m_sashTrackerPen->GetWidth()/2;
        y1 = 2;
        y2 = client_size.y - 2;
    }
    else
    {
        y1 = y2 = std::clamp(y, 0, client_size.y) + m_sashTrackerPen->GetWidth()/2;
        x1 = 2;
        x2 = client_size.x-2;
    }

    ClientToScreen(&x1, &y1);
    ClientToScreen(&x2, &y2);

    screenDC.SetLogicalFunction(wxRasterOperationMode::Invert);
    screenDC.SetPen(*m_sashTrackerPen);
    screenDC.SetBrush(*wxTRANSPARENT_BRUSH);

    screenDC.DrawLine(x1, y1, x2, y2);

    screenDC.SetLogicalFunction(wxRasterOperationMode::Copy);
}

int wxSplitterWindow::GetWindowSize() const
{
    wxSize size = GetClientSize();

    return m_splitMode == wxSplitMode::Vertical ? size.x : size.y;
}

int wxSplitterWindow::AdjustSashPosition(int sashPos) const
{
    wxWindow *win;

    win = GetWindow1();
    if ( win )
    {
        // the window shouldn't be smaller than its own minimal size nor
        // smaller than the minimual pane size specified for this splitter
        int minSize = m_splitMode == wxSplitMode::Vertical ? win->GetMinWidth()
                                                      : win->GetMinHeight();

        if ( minSize == -1 || m_minimumPaneSize > minSize )
            minSize = m_minimumPaneSize;

        minSize += GetBorderSize();

        if ( sashPos < minSize )
            sashPos = minSize;
    }

    win = GetWindow2();
    if ( win )
    {
        int minSize = m_splitMode == wxSplitMode::Vertical ? win->GetMinWidth()
                                                      : win->GetMinHeight();

        if ( minSize == -1 || m_minimumPaneSize > minSize )
            minSize = m_minimumPaneSize;

        int maxSize = GetWindowSize() - minSize - GetBorderSize() - GetSashSize();
        if ( maxSize > 0 && sashPos > maxSize && maxSize >= m_minimumPaneSize)
            sashPos = maxSize;
    }

    return sashPos;
}

bool wxSplitterWindow::DoSetSashPosition(int sashPos)
{
    int newSashPosition = AdjustSashPosition(sashPos);

    if ( newSashPosition == m_sashPosition )
        return false;

    m_sashPosition = newSashPosition;

    return true;
}

void wxSplitterWindow::SetSashPositionAndNotify(int sashPos)
{
    // we must reset the request here, otherwise the sash would be stuck at
    // old position if the user attempted to move the sash after invalid
    // (e.g. smaller than minsize) sash position was requested using
    // SetSashPosition():
    m_requestedSashPosition = std::numeric_limits<int>::max();

    // note that we must send the event in any case, i.e. even if the sash
    // position hasn't changed and DoSetSashPosition() returns false because we
    // must generate a CHANGED event at the end of resizing
    DoSetSashPosition(sashPos);

    wxSplitterEvent event(wxEVT_SPLITTER_SASH_POS_CHANGED, this);
    event.m_data.pos = m_sashPosition;

    DoSendEvent(event);
}

// Position and size subwindows.
// Note that the border size applies to each subwindow, not
// including the edges next to the sash.
void wxSplitterWindow::SizeWindows()
{
    // check if we have delayed setting the real sash position
    if ( m_requestedSashPosition != std::numeric_limits<int>::max() )
    {
        int newSashPosition = ConvertSashPosition(m_requestedSashPosition);
        if ( newSashPosition != m_sashPosition )
        {
            DoSetSashPosition(newSashPosition);
        }

        if ( newSashPosition <= m_sashPosition
            && newSashPosition >= m_sashPosition - GetBorderSize() )
        {
            // don't update it any more
            m_requestedSashPosition = std::numeric_limits<int>::max();
        }
    }

    wxSize client_size = GetClientSize();

    if ( GetWindow1() && !GetWindow2() )
    {
        GetWindow1()->SetSize(wxRect{GetBorderSize(),
                                     GetBorderSize(),
                                     client_size.x - 2*GetBorderSize(),
                                     client_size.y - 2*GetBorderSize()});
    }
    else if ( GetWindow1() && GetWindow2() )
    {
        const int border = GetBorderSize(),
                  sash = GetSashSize();

        int size1 = GetSashPosition() - border,
            size2 = GetSashPosition() + sash;

        int x2, y2, w1, h1, w2, h2;
        if ( GetSplitMode() == wxSplitMode::Vertical )
        {
            w1 = size1;
            w2 = client_size.x - 2*border - sash - w1;
            if (w2 < 0)
                w2 = 0;
            h2 = client_size.y - 2*border;
            if (h2 < 0)
                h2 = 0;
            h1 = h2;
            x2 = size2;
            y2 = border;
        }
        else // horz splitter
        {
            w2 = client_size.x - 2*border;
            if (w2 < 0)
                w2 = 0;
            w1 = w2;
            h1 = size1;
            h2 = client_size.y - 2*border - sash - h1;
            if (h2 < 0)
                h2 = 0;
            x2 = border;
            y2 = size2;
        }

        GetWindow2()->SetSize(wxRect{x2, y2, w2, h2});
        GetWindow1()->SetSize(wxRect{border, border, w1, h1});
    }

    wxClientDC dc(this);
    DrawSash(dc);
}

// Set pane for unsplit window
void wxSplitterWindow::Initialize(wxWindow *window)
{
    wxASSERT_MSG( (!window || window->GetParent() == this),
                  "windows in the splitter should have it as parent!" );

    if (window && !window->IsShown())
        window->Show();

    m_windowOne = window;
    m_windowTwo = nullptr;
    DoSetSashPosition(0);
}

// Associates the given window with window 2, drawing the appropriate sash
// and changing the split mode.
// Does nothing and returns false if the window is already split.
bool wxSplitterWindow::DoSplit(wxSplitMode mode,
                               wxWindow *window1, wxWindow *window2,
                               int sashPosition)
{
    if ( IsSplit() )
        return false;

    wxCHECK_MSG( window1 && window2, false,
                 "cannot split with NULL window(s)" );

    wxCHECK_MSG( window1->GetParent() == this && window2->GetParent() == this, false,
                  "windows in the splitter should have it as parent!" );

    if (! window1->IsShown())
        window1->Show();
    if (! window2->IsShown())
        window2->Show();

    m_splitMode = mode;
    m_windowOne = window1;
    m_windowTwo = window2;


    SetSashPosition(sashPosition, true);
    return true;
}

int wxSplitterWindow::ConvertSashPosition(int sashPosition) const
{
    if ( sashPosition > 0 )
    {
        return sashPosition;
    }
    else if ( sashPosition < 0 )
    {
        // It's negative so adding is subtracting
        return GetWindowSize() + sashPosition;
    }
    else // sashPosition == 0
    {
        // default, put it in the centre
        return GetWindowSize() / 2;
    }
}

// Remove the specified (or second) window from the view
// Doesn't actually delete the window.
bool wxSplitterWindow::Unsplit(wxWindow *toRemove)
{
    if ( ! IsSplit() )
        return false;

    wxWindow *win;
    if ( toRemove == nullptr || toRemove == m_windowTwo)
    {
        win = m_windowTwo ;
        m_windowTwo = nullptr;
    }
    else if ( toRemove == m_windowOne )
    {
        win = m_windowOne ;
        m_windowOne = m_windowTwo;
        m_windowTwo = nullptr;
    }
    else
    {
        wxFAIL_MSG("splitter: attempt to remove a non-existent window");

        return false;
    }

    OnUnsplit(win);
    DoSetSashPosition(0);
    SizeWindows();

    return true;
}

// Replace a window with another one
bool wxSplitterWindow::ReplaceWindow(wxWindow *winOld, wxWindow *winNew)
{
    wxCHECK_MSG( winOld, false, "use one of Split() functions instead" );
    wxCHECK_MSG( winNew, false, "use Unsplit() functions instead" );

    if ( winOld == m_windowTwo )
    {
        m_windowTwo = winNew;
    }
    else if ( winOld == m_windowOne )
    {
        m_windowOne = winNew;
    }
    else
    {
        wxFAIL_MSG("splitter: attempt to replace a non-existent window");

        return false;
    }

    SizeWindows();

    return true;
}

void wxSplitterWindow::SetMinimumPaneSize(int min)
{
    m_minimumPaneSize = min;
    int pos = m_requestedSashPosition != std::numeric_limits<int>::max() ? m_requestedSashPosition : m_sashPosition;
    SetSashPosition(pos); // re-check limits
}

void wxSplitterWindow::SetSashPosition(int position, bool redraw)
{
    // remember the sash position we want to set for later if we can't set it
    // right now (e.g. because the window is too small)
    m_requestedSashPosition = position;

    DoSetSashPosition(ConvertSashPosition(position));

    if ( redraw )
    {
        SizeWindows();
    }
}

// Make sure the child window sizes are updated. This is useful
// for reducing flicker by updating the sizes before a
// window is shown, if you know the overall size is correct.
void wxSplitterWindow::UpdateSize()
{
    SizeWindows();
}

bool wxSplitterWindow::DoSendEvent(wxSplitterEvent& event)
{
    return !GetEventHandler()->ProcessEvent(event) || event.IsAllowed();
}

wxSize wxSplitterWindow::DoGetBestSize() const
{
    // get best sizes of subwindows
    wxSize size1, size2;
    if ( m_windowOne )
        size1 = m_windowOne->GetEffectiveMinSize();
    if ( m_windowTwo )
        size2 = m_windowTwo->GetEffectiveMinSize();

    // sum them
    //
    // pSash points to the size component to which sash size must be added
    int *pSash;
    wxSize sizeBest;
    if ( m_splitMode == wxSplitMode::Vertical )
    {
        sizeBest.y = std::max(size1.y, size2.y);
        sizeBest.x = std::max(size1.x, m_minimumPaneSize) +
                        std::max(size2.x, m_minimumPaneSize);

        pSash = &sizeBest.x;
    }
    else // wxSplitMode::Horizontal
    {
        sizeBest.x = std::max(size1.x, size2.x);
        sizeBest.y = std::max(size1.y, m_minimumPaneSize) +
                        std::max(size2.y, m_minimumPaneSize);

        pSash = &sizeBest.y;
    }

    // account for the sash if the window is actually split
    if ( m_windowOne && m_windowTwo )
        *pSash += GetSashSize();

    // account for the border too
    int border = 2*GetBorderSize();
    sizeBest.x += border;
    sizeBest.y += border;

    return sizeBest;
}

// ---------------------------------------------------------------------------
// wxSplitterWindow virtual functions: they now just generate the events
// ---------------------------------------------------------------------------

bool wxSplitterWindow::OnSashPositionChange(int WXUNUSED(newSashPosition))
{
    // always allow by default
    return true;
}

int wxSplitterWindow::OnSashPositionChanging(int newSashPosition)
{
    // If within UNSPLIT_THRESHOLD from edge, set to edge to cause closure.
    const int UNSPLIT_THRESHOLD = 4;

    // first of all, check if OnSashPositionChange() doesn't forbid this change
    if ( !OnSashPositionChange(newSashPosition) )
    {
        // it does
        return -1;
    }

    // Obtain relevant window dimension for bottom / right threshold check
    int window_size = GetWindowSize();

    bool unsplit_scenario = false;
    if ( m_permitUnsplitAlways || m_minimumPaneSize == 0 )
    {
        // Do edge detection if unsplit premitted
        if ( newSashPosition <= UNSPLIT_THRESHOLD )
        {
            // threshold top / left check
            newSashPosition = 0;
            unsplit_scenario = true;
        }
        if ( newSashPosition >= window_size - UNSPLIT_THRESHOLD )
        {
            // threshold bottom/right check
            newSashPosition = window_size;
            unsplit_scenario = true;
        }
    }

    if ( !unsplit_scenario )
    {
        // If resultant pane would be too small, enlarge it
        newSashPosition = AdjustSashPosition(newSashPosition);

        // If the result is out of bounds it means minimum size is too big,
        // so split window in half as best compromise.
        if ( newSashPosition < 0 || newSashPosition > window_size )
            newSashPosition = window_size / 2;
    }

    // now let the event handler have it
    //
    // FIXME: shouldn't we do it before the adjustments above so as to ensure
    //        that the sash position is always reasonable?
    wxSplitterEvent event(wxEVT_SPLITTER_SASH_POS_CHANGING, this);
    event.m_data.pos = newSashPosition;

    if ( !DoSendEvent(event) )
    {
        // the event handler vetoed the change
        newSashPosition = -1;
    }
    else
    {
        // it could have been changed by it
        newSashPosition = event.GetSashPosition();
    }

    return newSashPosition;
}

// Called when the sash is double-clicked. The default behaviour is to remove
// the sash if the minimum pane size is zero.
void wxSplitterWindow::OnDoubleClickSash(int x, int y)
{
    wxCHECK_RET(m_windowTwo, "splitter: no window to remove");

    // new code should handle events instead of using the virtual functions
    wxSplitterEvent event(wxEVT_SPLITTER_DOUBLECLICKED, this);
    event.m_data.pt.x = x;
    event.m_data.pt.y = y;
    if ( DoSendEvent(event) )
    {
        if ( GetMinimumPaneSize() == 0 || m_permitUnsplitAlways )
        {
            wxWindow* win = m_windowTwo;
            if ( Unsplit(win) )
            {
                wxSplitterEvent unsplitEvent(wxEVT_SPLITTER_UNSPLIT, this);
                unsplitEvent.m_data.win = win;
                DoSendEvent(unsplitEvent);
            }
        }
    }
    //else: blocked by user
}

void wxSplitterWindow::OnUnsplit(wxWindow *winRemoved)
{
    // call this before calling the event handler which may delete the window
    winRemoved->Show(false);
}

#if defined( __WXMSW__ ) || defined( __WXMAC__)

// this is currently called (and needed) under MSW only...
void wxSplitterWindow::OnSetCursor(wxSetCursorEvent& event)
{
    // if we don't do it, the resizing cursor might be set for child window:
    // and like this we explicitly say that our cursor should not be used for
    // children windows which overlap us

    if ( SashHitTest(wxPoint{event.GetX(), event.GetY()}) )
    {
        // default processing is ok
        event.Skip();
    }
    //else: do nothing, in particular, don't call Skip()
}

#endif // wxMSW || wxMac

#endif // wxUSE_SPLITTER

