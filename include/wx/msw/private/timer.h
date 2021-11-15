/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/timer.h
// Purpose:     wxTimer class
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_TIMER_H_
#define _WX_MSW_PRIVATE_TIMER_H_

#if wxUSE_TIMER

#include "wx/private/timer.h"

#include <chrono>

using namespace std::chrono_literals;

class wxMSWTimerImpl : public wxTimerImpl
{
public:
    wxMSWTimerImpl(wxTimer *timer) : wxTimerImpl(timer) { m_id = 0; }

    bool Start(std::chrono::milliseconds startTime = -1ms, bool oneShot = false) override;
    void Stop() override;

    bool IsRunning() const override { return m_id != 0; }

protected:
    // this must be 64 bit under Win64 as WPARAM (storing timer ids) is 64 bit
    // there and so the ids may possibly not fit in 32 bits
    WPARAM m_id;
};

#endif // wxUSE_TIMER

#endif // _WX_TIMERH_
