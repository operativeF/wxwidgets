/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/accel.cpp
// Purpose:     wxAcceleratorTable
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_ACCEL

#ifndef WX_PRECOMP
    #include "wx/window.h"

    #include <boost/nowide/convert.hpp>
    #include <gsl/gsl>
#endif

#include "wx/accel.h"

#include "wx/msw/private.h"
#include "wx/msw/private/keyboard.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxAcceleratorTable, wxObject);

// ----------------------------------------------------------------------------
// data defining wxAcceleratorTable
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxAcceleratorRefData: public wxObjectRefData
{
    friend class WXDLLIMPEXP_FWD_CORE wxAcceleratorTable;
public:
    wxAcceleratorRefData() = default;
    ~wxAcceleratorRefData() override;
    wxAcceleratorRefData(const wxAcceleratorRefData&) = delete;
	wxAcceleratorRefData& operator=(const wxAcceleratorRefData&) = delete;

    inline HACCEL GetHACCEL() const { return m_hAccel; }

protected:
    HACCEL      m_hAccel{nullptr};
    bool        m_ok{false};
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxAcceleratorRefData
// ----------------------------------------------------------------------------

#define M_ACCELDATA ((wxAcceleratorRefData *)m_refData)

wxAcceleratorRefData::~wxAcceleratorRefData()
{
    if (m_hAccel)
    {
        ::DestroyAcceleratorTable((HACCEL) m_hAccel);
    }
}

// ----------------------------------------------------------------------------
// wxAcceleratorTable
// ----------------------------------------------------------------------------

// Load from .rc resource
wxAcceleratorTable::wxAcceleratorTable(const std::string& resource)
{
    m_refData = new wxAcceleratorRefData;

    HACCEL hAccel = ::LoadAcceleratorsW(wxGetInstance(), boost::nowide::widen(resource).c_str());
    M_ACCELDATA->m_hAccel = hAccel;
    M_ACCELDATA->m_ok = hAccel != nullptr;
}

// Create from an array
wxAcceleratorTable::wxAcceleratorTable(std::span<wxAcceleratorEntry> entries)
{
    m_refData = new wxAcceleratorRefData;

    auto arr = std::make_unique<ACCEL[]>(entries.size());

    for ( size_t i = 0; i < entries.size(); ++i )
    {
        int flags = entries[i].GetFlags();

        BYTE fVirt = FVIRTKEY;
        if ( flags & wxACCEL_ALT )
            fVirt |= FALT;
        if ( flags & wxACCEL_SHIFT )
            fVirt |= FSHIFT;
        if ( flags & wxACCEL_CTRL )
            fVirt |= FCONTROL;

        WORD key = wxMSWKeyboard::WXToVK(entries[i].GetKeyCode());

        arr[i].fVirt = fVirt;
        arr[i].key = key;
        arr[i].cmd = (WORD)entries[i].GetCommand();
    }

    M_ACCELDATA->m_hAccel = ::CreateAcceleratorTableW(arr.get(), gsl::narrow_cast<int>(entries.size()));

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

    M_ACCELDATA->m_hAccel = (HACCEL) hAccel;
}

WXHACCEL wxAcceleratorTable::GetHACCEL() const
{
    if (!M_ACCELDATA)
        return nullptr;
    return (WXHACCEL) M_ACCELDATA->m_hAccel;
}

bool wxAcceleratorTable::Translate(wxWindow *window, WXMSG *wxmsg) const
{
    MSG *msg = (MSG *)wxmsg;
    return IsOk() && ::TranslateAcceleratorW(GetHwndOf(window), GetHaccel(), msg);
}

#endif // wxUSE_ACCEL

