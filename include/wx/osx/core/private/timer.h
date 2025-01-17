/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/core/private/timer.h
// Purpose:     wxTimer class based on core foundation
// Author:      Stefan Csomor
// Created:     2008-07-16
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_CORE_PRIVATE_TIMER_H_
#define _WX_OSX_CORE_PRIVATE_TIMER_H_

#include "wx/private/timer.h"

struct wxOSXTimerInfo;

class wxOSXTimerImpl : public wxTimerImpl
{
public:
    wxOSXTimerImpl(wxTimer *timer);
    virtual ~wxOSXTimerImpl();

    bool Start(int milliseconds = -1, bool one_shot = false) override;
    void Stop() override;

    bool IsRunning() const override;

private:
    wxOSXTimerInfo *m_info;
};

#endif // _WX_OSX_CORE_PRIVATE_TIMER_H_
