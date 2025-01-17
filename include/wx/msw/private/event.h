///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/event.h
// Purpose:     Simple Windows 'event object' wrapper.
// Author:      Troelsk, Vadim Zeitlin
// Created:     2014-05-07
// Copyright:   (c) 2014 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_EVENT_H_
#define _WX_MSW_PRIVATE_EVENT_H_

#include "wx/msw/private.h"

namespace wxWinAPI
{

class Event : public AutoHANDLE<0>
{
public:
    enum Kind
    {
        ManualReset,
        AutomaticReset
    };

    enum InitialState
    {
        Signaled,
        Nonsignaled
    };

	Event& operator=(Event&&) = delete;
    
    // Wrappers around {Create,Set,Reset}Event() Windows API functions, with
    // the same semantics.
    [[maybe_unused]] bool Create(Kind kind = AutomaticReset,
                InitialState initialState = Nonsignaled,
                const wxChar* name = nullptr);
    bool Set();
    bool Reset();
};

} // namespace wxWinAPI

// ----------------------------------------------------------------------------
// Implementations requiring windows.h; these are to moved out-of-line if
// this class is moved to a public header, or if [parts of] msw/private.h is
// changed to not depend on windows.h being included.
// ----------------------------------------------------------------------------

inline bool
wxWinAPI::Event::Create(wxWinAPI::Event::Kind kind,
                        wxWinAPI::Event::InitialState initialState,
                        const wxChar* name)
{
    wxCHECK_MSG( !IsOk(), false, "Event can't be created twice" );

    WXHANDLE handle = ::CreateEventW(nullptr,
                                    kind == ManualReset,
                                    initialState == Signaled,
                                    name);
    if ( !handle )
    {
        wxLogLastError("CreateEvent");
        return false;
    }

    m_handle = handle;
    return true;
}

inline bool wxWinAPI::Event::Set()
{
    wxCHECK_MSG( m_handle, false, "Event must be valid" );

    if ( !::SetEvent(m_handle) )
    {
        wxLogLastError("SetEvent");
        return false;
    }

    return true;
}

inline bool wxWinAPI::Event::Reset()
{
    wxCHECK_MSG( m_handle, false, "Event must be valid" );

    if ( !::ResetEvent(m_handle) )
    {
        wxLogLastError("ResetEvent");
        return false;
    }

    return true;
}

#endif // _WX_MSW_PRIVATE_EVENT_H_
