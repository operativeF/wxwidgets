/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/spinctrlcmn.cpp
// Purpose:     define wxSpinCtrl event types
// Author:      Peter Most
// Created:     01.11.08
// Copyright:   (c) 2008-2009 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SPINCTRL

#include "wx/spinbutt.h"
#include "wx/spinctrl.h"

#include "wx/private/spinctrl.h"

#include <fmt/core.h>

wxDEFINE_EVENT(wxEVT_SPINCTRL, wxSpinEvent);
wxDEFINE_EVENT(wxEVT_SPINCTRLDOUBLE, wxSpinDoubleEvent);

using namespace wxSpinCtrlImpl;

wxString wxSpinCtrlImpl::FormatAsHex(long val, long maxVal)
{
    // We format the value like this is for compatibility with (native
    // behaviour of) wxMSW
    wxString text;
    if ( maxVal < 0x10000 )
        text.Printf("0x%04lx", val);
    else
        text.Printf("0x%08lx", val);

    return text;
}

int wxSpinCtrlImpl::GetMaxValueLength(int minVal, int maxVal, int base)
{
    const int lenMin = (base == 16 ?
                       FormatAsHex(minVal, maxVal).ToStdString() :
                       fmt::format("{:d}", minVal)).length();
    const int lenMax = (base == 16 ?
                       FormatAsHex(maxVal, maxVal).ToStdString() :
                       fmt::format("{:d}", maxVal)).length();
    return std::max(lenMin, lenMax);
}

wxSize wxSpinCtrlImpl::GetBestSize(const wxControl* spin,
                                   int minVal, int maxVal, int base)
{
    const std::string largestString('8', GetMaxValueLength(minVal, maxVal, base));
    return spin->GetSizeFromText(largestString);
}

#endif // wxUSE_SPINCTRL
