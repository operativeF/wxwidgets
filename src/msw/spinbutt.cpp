/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/spinbutt.cpp
// Purpose:     wxSpinButton
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SPINBTN

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"

#include "wx/spinbutt.h"

import WX.WinDef;

#ifndef UDM_SETRANGE32
    #define UDM_SETRANGE32 (WM_USER+111)
#endif

#ifndef UDM_SETPOS32
    #define UDM_SETPOS32 (WM_USER+113)
    #define UDM_GETPOS32 (WM_USER+114)
#endif

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxWin macros
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// wxSpinButton
// ----------------------------------------------------------------------------

bool wxSpinButton::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& size,
                          unsigned int style,
                          std::string_view name)
{
    // basic initialization
    m_windowId = (id == wxID_ANY) ? NewControlId() : id;

    SetName(name);

    wxPoint pt = pos;
    wxSize sz = size;

    m_windowStyle = style;

    SetParent(parent);

    // get the right size for the control
    if ( sz.x <= 0 || sz.y <= 0 )
    {
        wxSize bestSize = DoGetBestSize();
        if ( sz.x <= 0 )
            sz.x = bestSize.x;
        if ( sz.y <= 0 )
            sz.y = bestSize.y;
    }

    if ( pt.x < 0 )
        pt.x = 0;
    if ( pt.y < 0 )
        pt.y = 0;

    // translate the styles
    WXDWORD wstyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | /*  WS_CLIPSIBLINGS | */
                   UDS_NOTHOUSANDS | // never useful, sometimes harmful
                   UDS_ALIGNRIGHT  | // these styles are effectively used only
                   UDS_SETBUDDYINT;  //  by wxSpinCtrl but do no harm otherwise

    if ( m_windowStyle & wxCLIP_SIBLINGS )
        wstyle |= WS_CLIPSIBLINGS;
    if ( m_windowStyle & wxSP_HORIZONTAL )
        wstyle |= UDS_HORZ;
    if ( m_windowStyle & wxSP_ARROW_KEYS )
        wstyle |= UDS_ARROWKEYS;
    if ( m_windowStyle & wxSP_WRAP )
        wstyle |= UDS_WRAP;

    // create the UpDown control.
    m_hWnd = (WXHWND)CreateUpDownControl
                     (
                       wstyle,
                       pt.x, pt.y, sz.x, sz.y,
                       GetHwndOf(parent),
                       m_windowId,
                       wxGetInstance(),
                       nullptr, // no buddy
                       m_max, m_min,
                       m_min // initial position
                     );

    if ( !m_hWnd )
    {
        wxLogLastError("CreateUpDownControl");

        return false;
    }

    if ( parent )
    {
        parent->AddChild(this);
    }

    SubclassWin(m_hWnd);

    SetInitialSize(size);

    return true;
}

// ----------------------------------------------------------------------------
// size calculation
// ----------------------------------------------------------------------------

wxSize wxSpinButton::DoGetBestSize() const
{
    const bool vert = HasFlag(wxSP_VERTICAL);

    wxSize bestSize(wxGetSystemMetrics(vert ? SM_CXVSCROLL : SM_CXHSCROLL, m_parent),
                    wxGetSystemMetrics(vert ? SM_CYVSCROLL : SM_CYHSCROLL, m_parent));

    if ( vert )
        bestSize.y *= 2;
    else
        bestSize.x *= 2;

    return bestSize;
}

// ----------------------------------------------------------------------------
// Attributes
// ----------------------------------------------------------------------------

int wxSpinButton::GetValue() const
{
#ifdef UDM_GETPOS32
    // use the full 32 bit range if available
    auto n = ::SendMessageW(GetHwnd(), UDM_GETPOS32, 0, 0);
#else
    // we're limited to 16 bit
    auto n = (short)LOWORD(::SendMessageW(GetHwnd(), UDM_GETPOS, 0, 0));
#endif // UDM_GETPOS32

    if (n < m_min) n = m_min;
    if (n > m_max) n = m_max;

    return n;
}

void wxSpinButton::SetValue(int val)
{
    // wxSpinButtonBase::SetValue(val); -- no, it is pure virtual

#ifdef UDM_SETPOS32
    // use the full 32 bit range if available
    ::SendMessageW(GetHwnd(), UDM_SETPOS32, 0, val);
#else
    ::SendMessageW(GetHwnd(), UDM_SETPOS, 0, MAKELONG((short) val, 0));
#endif // UDM_SETPOS32
}

void wxSpinButton::NormalizeValue()
{
    SetValue( GetValue() );
}

void wxSpinButton::SetRange(int minVal, int maxVal)
{
    const bool hadRange = m_min < m_max;

    wxSpinButtonBase::SetRange(minVal, maxVal);

#ifdef UDM_SETRANGE32
    // use the full 32 bit range if available
    ::SendMessageW(GetHwnd(), UDM_SETRANGE32, minVal, maxVal);
#else
    // we're limited to 16 bit
    ::SendMessageW(GetHwnd(), UDM_SETRANGE, 0,
                  (WXLPARAM) MAKELONG((short)maxVal, (short)minVal));
#endif // UDM_SETRANGE32

    // the current value might be out of the new range, force it to be in it
    NormalizeValue();

    // if range was valid but becomes degenerated (min == max) now or vice
    // versa then the spin buttons are automatically disabled/enabled back
    // but don't update themselves for some reason, so do it manually
    if ( hadRange != (m_min < m_max) )
    {
        // update the visual state of the button
        Refresh();
    }
}

bool wxSpinButton::MSWOnScroll([[maybe_unused]] int orientation, WXWORD wParam,
                               [[maybe_unused]] WXWORD pos, WXHWND control)
{
    wxCHECK_MSG( control, false, "scrolling what?" );

    if ( wParam != SB_THUMBPOSITION )
    {
        // probable SB_ENDSCROLL - we don't react to it
        return false;
    }

    wxSpinEvent event(wxEVT_SCROLL_THUMBTRACK, m_windowId);
    // We can't use 16 bit position provided in this message for spin buttons
    // using 32 bit range.
    event.SetPosition(GetValue());
    event.SetEventObject(this);

    return HandleWindowEvent(event);
}

bool wxSpinButton::MSWOnNotify([[maybe_unused]] int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{
    NM_UPDOWN *lpnmud = (NM_UPDOWN *)lParam;

    if ( lpnmud->hdr.hwndFrom != GetHwnd() || // make sure it is the right control
         lpnmud->hdr.code != UDN_DELTAPOS )   // and the right notification 
        return false;

    int newVal = lpnmud->iPos + lpnmud->iDelta;
    if ( newVal < m_min )
    {
        newVal = HasFlag(wxSP_WRAP) ? m_max : m_min;
    }
    else if ( newVal > m_max )
    {
        newVal = HasFlag(wxSP_WRAP) ? m_min : m_max;
    }

    // Don't send an event if the value hasn't actually changed (for compatibility with wxGTK and wxOSX).
    if ( newVal == lpnmud->iPos )
    {
        *result = 1;
        return true;
    }

    wxSpinEvent event(lpnmud->iDelta > 0 ? wxEVT_SCROLL_LINEUP
                                         : wxEVT_SCROLL_LINEDOWN,
                      m_windowId);
    event.SetPosition(newVal);
    event.SetEventObject(this);

    bool processed = HandleWindowEvent(event);

    *result = event.IsAllowed() ? 0 : 1;

    return processed;
}

bool wxSpinButton::MSWCommand([[maybe_unused]] WXUINT cmd, [[maybe_unused]] WXWORD id)
{
    // No command messages
    return false;
}

#endif // wxUSE_SPINBTN
