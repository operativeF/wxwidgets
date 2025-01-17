/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/uiaction.cpp
// Purpose:     wxUIActionSimulator implementation
// Author:      Kevin Ollivier, Steven Lamerton, Vadim Zeitlin
// Modified by:
// Created:     2010-03-06
// Copyright:   (c) Kevin Ollivier
//              (c) 2010 Steven Lamerton
//              (c) 2010 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_UIACTIONSIMULATOR    

#include "wx/msw/private.h"             // For wxGetCursorPosMSW()

#include "wx/display.h"
#include "wx/uiaction.h"
#include "wx/private/uiaction.h"

#include "wx/msw/private/keyboard.h"

import WX.WinDef;

import <cmath>;

namespace
{

class wxUIActionSimulatorMSWImpl : public wxUIActionSimulatorImpl
{
public:
    wxUIActionSimulatorMSWImpl(const wxUIActionSimulatorMSWImpl&) = delete;
	wxUIActionSimulatorMSWImpl& operator=(const wxUIActionSimulatorMSWImpl&) = delete;
    
    // Returns a pointer to the global simulator object: as it's stateless, we
    // can reuse the same one without having to allocate it on the heap all the
    // time.
    static wxUIActionSimulatorMSWImpl* Get()
    {
        static wxUIActionSimulatorMSWImpl s_impl;
        return &s_impl;
    }

    bool MouseMove(long x, long y) override;
    bool MouseDown(int button = wxMOUSE_BTN_LEFT) override;
    bool MouseUp(int button = wxMOUSE_BTN_LEFT) override;

    bool DoKey(int keycode, int modifiers, bool isDown) override;

private:
    // This class has no public ctors, use Get() instead.
    wxUIActionSimulatorMSWImpl() = default;
};

WXDWORD EventTypeForMouseButton(int button, bool isDown)
{
    switch (button)
    {
        case wxMOUSE_BTN_LEFT:
            return isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;

        case wxMOUSE_BTN_RIGHT:
            return isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;

        case wxMOUSE_BTN_MIDDLE:
            return isDown ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;

        default:
            wxFAIL_MSG("Unsupported button passed in.");
            return (WXDWORD)-1;
    }
}

} // anonymous namespace

bool wxUIActionSimulatorMSWImpl::MouseDown(int button)
{
    // FIXME: Return cursor pos
    POINT p;
    wxGetCursorPosMSW(&p);
    ::mouse_event(EventTypeForMouseButton(button, true), p.x, p.y, 0, 0);
    return true;
}

bool wxUIActionSimulatorMSWImpl::MouseMove(long x, long y)
{
    // Because MOUSEEVENTF_ABSOLUTE takes measurements scaled between 0 & 65535
    // we need to scale our input too
    auto displaySize = wxDisplay().GetGeometry().GetSize();

    // Casts are safe because x and y are supposed to be less than the display
    // size, so there is no danger of overflow.
    WXDWORD scaledx = static_cast<WXDWORD>(std::ceil(x * 65535.0 / (displaySize.x - 1)));
    WXDWORD scaledy = static_cast<WXDWORD>(std::ceil(y * 65535.0 / (displaySize.y - 1)));
    ::mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, scaledx, scaledy, 0, 0);

    return true;
}

bool wxUIActionSimulatorMSWImpl::MouseUp(int button)
{
    // FIXME: Return cursor pos
    POINT p;
    wxGetCursorPosMSW(&p);
    mouse_event(EventTypeForMouseButton(button, false), p.x, p.y, 0, 0);
    return true;
}

bool
wxUIActionSimulatorMSWImpl::DoKey(int keycode, [[maybe_unused]] int modifiers, bool isDown)
{
    bool isExtended;
    WXDWORD vkkeycode = wxMSWKeyboard::WXToVK(keycode, &isExtended);

    WXDWORD flags = 0;
    if ( isExtended )
        flags |= KEYEVENTF_EXTENDEDKEY;
    if ( !isDown )
        flags |= KEYEVENTF_KEYUP;

    ::keybd_event(vkkeycode, 0, flags, 0);

    return true;
}

wxUIActionSimulator::wxUIActionSimulator()
                   : m_impl(wxUIActionSimulatorMSWImpl::Get())
{
}

#endif // wxUSE_UIACTIONSIMULATOR
