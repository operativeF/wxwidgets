///////////////////////////////////////////////////////////////////////////////
// Name:        testableframe.cpp
// Purpose:     An improved wxFrame for unit-testing
// Author:      Steven Lamerton
// Copyright:   (c) 2010 Steven Lamerton
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "testableframe.h"

import WX.Test.Prec;

wxTestableFrame::wxTestableFrame() : wxFrame(nullptr, wxID_ANY, wxASCII_STR("Test Frame"))
{
    // Use fixed position to facilitate debugging.
    Move(wxPoint{200, 200});

    Show();
}

void wxTestableFrame::OnEvent(wxEvent& evt)
{
    m_count[evt.GetEventType()]++;

    if(! evt.IsCommandEvent() )
        evt.Skip();
}

int wxTestableFrame::GetEventCount(wxEventType type)
{
    return m_count[type];
}

void wxTestableFrame::ClearEventCount(wxEventType type)
{
    m_count[type] = 0;
}

EventCounter::EventCounter(wxWindow* win, wxEventType type) : m_type(type),
                                                              m_win(win)

{
    m_frame = wxStaticCast(wxTheApp->GetTopWindow(), wxTestableFrame);

    m_win->Connect(m_type, wxEventHandler(wxTestableFrame::OnEvent),
                   nullptr, m_frame);
}

EventCounter::~EventCounter()
{
    m_win->Disconnect(m_type, wxEventHandler(wxTestableFrame::OnEvent),
                      nullptr, m_frame);

    //This stops spurious counts from previous tests
    Clear();

    m_frame = nullptr;
    m_win = nullptr;
}

bool EventCounter::WaitEvent(int timeInMs)
{
    static constexpr int SINGLE_WAIT_DURATION = 50;

    for ( int i = 0; i < timeInMs / SINGLE_WAIT_DURATION; ++i )
    {
        wxYield();

        const int count = GetCount();
        if ( count )
        {
            CHECK( count == 1 );

            Clear();
            return true;
        }

        wxMilliSleep(SINGLE_WAIT_DURATION);
    }

    return false;
}
