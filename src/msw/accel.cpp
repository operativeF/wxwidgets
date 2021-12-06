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

import WX.Utils.Cast;

import WX.Win.UniqueHnd;

// ----------------------------------------------------------------------------
// data defining wxAcceleratorTable
// ----------------------------------------------------------------------------

using msw::utils::unique_accel;

class wxAcceleratorRefData: public wxObjectRefData
{
    friend class wxAcceleratorTable;
public:
    inline WXHACCEL GetHACCEL() const { return m_hAccel.get(); }

protected:
    unique_accel m_hAccel;
    bool         m_ok{false};
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxAcceleratorRefData
// ----------------------------------------------------------------------------

#define M_ACCELDATA ((wxAcceleratorRefData *)m_refData)

// ----------------------------------------------------------------------------
// wxAcceleratorTable
// ----------------------------------------------------------------------------

// Load from .rc resource
wxAcceleratorTable::wxAcceleratorTable(const std::string& resource)
{
    m_refData = new wxAcceleratorRefData;

    M_ACCELDATA->m_hAccel = unique_accel(::LoadAcceleratorsW(wxGetInstance(),
                                                             boost::nowide::widen(resource).c_str()));
    M_ACCELDATA->m_ok = M_ACCELDATA->m_hAccel != nullptr;
}

// Create from an array
wxAcceleratorTable::wxAcceleratorTable(std::span<wxAcceleratorEntry> entries)
{
    m_refData = new wxAcceleratorRefData;

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

        arr[i].fVirt = fVirt;
        arr[i].key = key;
        arr[i].cmd = (WXWORD)entries[i].GetCommand();
    }

    M_ACCELDATA->m_hAccel = unique_accel(::CreateAcceleratorTableW(arr.get(), wx::narrow_cast<int>(entries.size())));

    M_ACCELDATA->m_ok = (M_ACCELDATA->m_hAccel != nullptr);
}

bool wxAcceleratorTable::IsOk() const
{
    return (M_ACCELDATA && (M_ACCELDATA->m_ok));
}

void wxAcceleratorTable::SetHACCEL(WXHACCEL hAccel)
{
    if (!M_ACCELDATA)
        m_refData = new wxAcceleratorRefData;

    M_ACCELDATA->m_hAccel = unique_accel(hAccel);
}

WXHACCEL wxAcceleratorTable::GetHACCEL() const
{
    if (!M_ACCELDATA)
        return nullptr;
    return (WXHACCEL) M_ACCELDATA->m_hAccel.get();
}

bool wxAcceleratorTable::Translate(wxWindow *window, WXMSG *wxmsg) const
{
    MSG *msg = (MSG *)wxmsg;
    return IsOk() && ::TranslateAcceleratorW(GetHwndOf(window), GetHaccel(), msg);
}

#endif // wxUSE_ACCEL

