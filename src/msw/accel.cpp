/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/accel.cpp
// Purpose:     wxAcceleratorTable
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_ACCEL

#include "wx/window.h"
#include "wx/accel.h"

#include "wx/msw/private/keyboard.h"

#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

#include <memory>

import WX.Utils.Cast;
import WX.Win.UniqueHnd;

import <span>;
import <string>;

// ----------------------------------------------------------------------------
// wxAcceleratorTable
// ----------------------------------------------------------------------------

// Load from .rc resource
wxAcceleratorTable::wxAcceleratorTable(const std::string& resource)
{
    m_hAccel = msw::utils::unique_accel(::LoadAcceleratorsW(wxGetInstance(),
                                                             boost::nowide::widen(resource).c_str()));
    m_ok = m_hAccel != nullptr;
}

// Create from a continguous container
wxAcceleratorTable::wxAcceleratorTable(std::span<wxAcceleratorEntry> entries)
{
    auto arr = std::make_unique<ACCEL[]>(entries.size());

    for ( size_t i = 0; i < entries.size(); ++i )
    {
        unsigned int flags = entries[i].GetFlags();

        BYTE fVirt = FVIRTKEY;
        if ( flags & wxACCEL_ALT )
            fVirt |= FALT;
        if ( flags & wxACCEL_SHIFT )
            fVirt |= FSHIFT;
        if ( flags & wxACCEL_CTRL )
            fVirt |= FCONTROL;

        WXWORD key = wxMSWKeyboard::WXToVK(entries[i].GetKeyCode());

        arr[i] = { .fVirt{fVirt},
                   .key{key},
                   .cmd{(WXWORD)entries[i].GetCommand()} };    
    }

    m_hAccel = msw::utils::unique_accel(::CreateAcceleratorTableW(arr.get(), wx::narrow_cast<int>(entries.size())));

    m_ok = m_hAccel != nullptr;
}

bool wxAcceleratorTable::IsOk() const
{
    return m_hAccel && m_ok;
}

void wxAcceleratorTable::SetHACCEL(WXHACCEL hAccel)
{
    m_hAccel = msw::utils::unique_accel(hAccel);
}

WXHACCEL wxAcceleratorTable::GetHACCEL() const
{
    if (m_hAccel)
        return m_hAccel.get();

    return nullptr;
}

bool wxAcceleratorTable::Translate(wxWindow *window, WXMSG *wxmsg) const
{
    return m_ok && ::TranslateAcceleratorW(GetHwndOf(window), GetHaccel(), wxmsg);
}

#endif // wxUSE_ACCEL

