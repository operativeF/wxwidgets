///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/evtloop.h
// Purpose:     simply forwards to wx/osx/carbon/evtloop.h or
//              wx/osx/cocoa/evtloop.h for consistency with the other Mac
//              headers
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2006-01-12
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_EVTLOOP_H_
#define _WX_OSX_EVTLOOP_H_

#include "wx/osx/cocoa/evtloop.h"

class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxNonOwnedWindow;

class wxModalEventLoop : public wxGUIEventLoop
{
public:
    wxModalEventLoop(wxWindow *modalWindow);
    wxModalEventLoop(WXWindow modalNativeWindow);

#ifdef __WXOSX_COCOA__
    // skip wxGUIEventLoop to avoid missing Enter/Exit notifications
    int Run() override { return wxCFEventLoop::Run(); }
#endif
protected:
    void OSXDoRun() override;
    void OSXDoStop() override;

    // (in case) the modal window for this event loop
    wxNonOwnedWindow* m_modalWindow;
    WXWindow m_modalNativeWindow;
};

#endif // _WX_OSX_EVTLOOP_H_
