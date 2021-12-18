///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/basemsw.cpp
// Purpose:     misc stuff only used in console applications under MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     22.06.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/apptrait.h"
#include "wx/evtloop.h"
#include "wx/msw/private/timer.h"

#include "wx/crt.h"

#include <boost/nowide/convert.hpp>
#include <fmt/printf.h>

import WX.WinDef;

// ============================================================================
// wxAppTraits implementation
// ============================================================================

bool wxAppTraits::SafeMessageBox(std::string_view text,
                                 std::string_view title)
{
    const WXHWND hwndParent = GetMainHWND();
    unsigned int flags = MB_OK | MB_ICONSTOP;

    // Using MB_TASKMODAL with valid parent doesn't work well because it
    // prevents the typical behaviour of modal message boxes, e.g. the message
    // box doesn't come up to front when the parent is clicked. But if we don't
    // have any parent anyhow, we can just as well use it, as we don't lose
    // anything and it has a useful side effect of disabling any existing TLWs
    // if there are any.
    //
    // Note that we also might have chosen to always use MB_TASKMODAL and NULL
    // parent. This would have the advantage of always disabling all the window
    // which, but at the cost of the behaviour mentioned above and other
    // related problems, e.g. showing ugly default icon in Alt-Tab list and an
    // extra taskbar button for the message box, so we don't do this, although
    // perhaps we still should, at least in case when there is more than one
    // TLW (but we can't check for this easily as this is non-GUI code and
    // wxTopLevelWindows is not accessible from it).
    if ( !hwndParent )
        flags |= MB_TASKMODAL;

    ::MessageBoxW(hwndParent, boost::nowide::widen(text).c_str(), boost::nowide::widen(title).c_str(), flags);

    return true;
}

#if wxUSE_THREADS
WXDWORD wxAppTraits::DoSimpleWaitForThread(WXHANDLE hThread)
{
    return ::WaitForSingleObject((HANDLE)hThread, INFINITE);
}
#endif // wxUSE_THREADS

// ============================================================================
// wxConsoleAppTraits implementation
// ============================================================================

void *wxConsoleAppTraits::BeforeChildWaitLoop()
{
    // nothing to do here
    return nullptr;
}

void wxConsoleAppTraits::AfterChildWaitLoop([[maybe_unused]] void * data)
{
    // nothing to do here
}

#if wxUSE_THREADS
bool wxConsoleAppTraits::DoMessageFromThreadWait()
{
    // nothing to process here
    return true;
}

WXDWORD wxConsoleAppTraits::WaitForThread(WXHANDLE hThread, [[maybe_unused]] wxThreadWait flags)
{
    return DoSimpleWaitForThread(hThread);
}
#endif // wxUSE_THREADS

#if wxUSE_TIMER

wxTimerImpl *wxConsoleAppTraits::CreateTimerImpl(wxTimer *timer)
{
    return new wxMSWTimerImpl(timer);
}

#endif // wxUSE_TIMER

// Why can't this be disabled for __WXQT__ ??? There is an implementation in src/qt/apptraits.cpp
std::unique_ptr<wxEventLoopBase> wxConsoleAppTraits::CreateEventLoop()
{
#if wxUSE_CONSOLE_EVENTLOOP
    return std::make_unique<wxEventLoop>();
#else // !wxUSE_CONSOLE_EVENTLOOP
    return {};
#endif // wxUSE_CONSOLE_EVENTLOOP/!wxUSE_CONSOLE_EVENTLOOP
}


bool wxConsoleAppTraits::WriteToStderr(const std::string& text)
{
    return fmt::fprintf(stderr, "%s", text) != -1;
}
