///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/mousemanager.cpp
// Purpose:     implementation of wxMouseEventsManager class
// Author:      Vadim Zeitlin
// Created:     2009-04-21
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/window.h"
#include "wx/mousemanager.h"

import WX.Utils.Settings;

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxMouseEventsManager, wxEvtHandler)
    EVT_MOUSE_CAPTURE_LOST(wxMouseEventsManager::OnCaptureLost)
    EVT_LEFT_DOWN(wxMouseEventsManager::OnLeftDown)
    EVT_LEFT_UP(wxMouseEventsManager::OnLeftUp)
    EVT_MOTION(wxMouseEventsManager::OnMove)
wxEND_EVENT_TABLE()

// ============================================================================
// wxMouseEventsManager implementation
// ============================================================================



bool wxMouseEventsManager::Create(wxWindow *win)
{
    wxASSERT_MSG( !m_win, "Create() must not be called twice" );

    m_win = win;
    win->PushEventHandler(this);

    return true;
}

wxMouseEventsManager::~wxMouseEventsManager()
{
    if ( m_win )
        m_win->RemoveEventHandler(this);
}

void wxMouseEventsManager::OnCaptureLost([[maybe_unused]] wxMouseCaptureLostEvent& event)
{
    switch ( m_state )
    {
        case State::Normal:
            wxFAIL_MSG( "mouse shouldn't be captured in normal state" );
            break;

        case State::Pressed:
            MouseClickCancelled(m_item);
            break;

        case State::Dragging:
            MouseDragCancelled(m_item);
            break;
    }

    m_state = State::Normal;
    m_item = wxNOT_FOUND;
}

void wxMouseEventsManager::OnLeftDown(wxMouseEvent& event)
{
    wxASSERT_MSG( m_state == State::Normal,
                  "state hasn't been reset to normal somehow" );

    m_posLast = event.GetPosition();
    m_item = MouseHitTest(m_posLast);
    if ( m_item == wxNOT_FOUND )
    {
        event.Skip();
        return;
    }

    m_state = State::Pressed;
    m_win->SetFocus();
    m_win->CaptureMouse();
    MouseClickBegin(m_item);
}

void wxMouseEventsManager::OnLeftUp(wxMouseEvent& event)
{
    switch ( m_state )
    {
        case State::Normal:
            // ignore it, the mouse hasn't been pressed over any item initially
            // so releasing it shouldn't do anything
            event.Skip();

            // skip releasing the capture below
            return;

        case State::Pressed:
            if ( MouseHitTest(event.GetPosition()) == m_item )
            {
                // mouse released over the same item, so it was a click
                MouseClicked(m_item);
            }
            break;

        case State::Dragging:
            MouseDragEnd(m_item, event.GetPosition());
            break;
    }

    m_state = State::Normal;
    m_item = wxNOT_FOUND;
    m_win->ReleaseMouse();
}

void wxMouseEventsManager::OnMove(wxMouseEvent& event)
{
    switch ( m_state )
    {
        case State::Normal:
            event.Skip();
            break;

        case State::Pressed:
            wxASSERT_MSG( event.LeftIsDown(),
                          "should have detected mouse being released" );

            {
                // it's probably a bad idea to query the system for these
                // values every time the mouse is moved so cache them on the
                // assumption that they don't change -- which is wrong, of
                // course, the user can change them but it doesn't happen often
                static const int
                    dragMinX = wxSystemSettings::GetMetric(wxSYS_DRAG_X, m_win);
                static const int
                    dragMinY = wxSystemSettings::GetMetric(wxSYS_DRAG_Y, m_win);

                const wxPoint& pos = event.GetPosition();
                const wxPoint ofs = pos - m_posLast;
                if ( std::abs(ofs.x) > dragMinX || std::abs(ofs.y) > dragMinY )
                {
                    // the mouse left the rectangle inside which its movements
                    // are considered to be too small to constitute a start of
                    // drag operation, do [attempt to] start it now
                    if ( MouseDragBegin(m_item, pos) )
                    {
                        m_state = State::Dragging;
                    }
                }
                else // still didn't move far enough away
                {
                    event.Skip();
                }
            }
            break;

        case State::Dragging:
            m_posLast = event.GetPosition();
            MouseDragging(m_item, m_posLast);
            break;
    }
}

