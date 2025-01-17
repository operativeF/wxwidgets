///////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/spinctrl.h
// Purpose:     Private functions used in wxSpinCtrl implementation.
// Author:      Vadim Zeitlin
// Created:     2019-11-13
// Copyright:   (c) 2019 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_SPINCTRL_H_
#define _WX_PRIVATE_SPINCTRL_H_

#include "wx/control.h"
#include "wx/string.h"

namespace wxSpinCtrlImpl
{

constexpr bool IsBaseCompatibleWithRange(int minVal, int maxVal, int base)
{
    // Negative values in the range are allowed only if base == 10
    return base == 10 || (minVal >= 0 && maxVal >= 0);
}

// This is an internal helper function currently used by all ports: return the
// string containing hexadecimal representation of the given number.
extern wxString FormatAsHex(long val, long maxVal);

// Another helper returning the maximum length of a string representing a value
// valid in the given control.
extern int GetMaxValueLength(int minVal, int maxVal, int base);

// The helper function to determine the best size for the given control.
// We can't implement this function in the wxSpinCtrlBase because MSW implementation
// of wxSpinCtrl is derived from wxSpinButton but uses the same algorithm.
extern wxSize GetBestSize(const wxControl* spin, int minVal, int maxVal, int base);

} // namespace wxSpinCtrlImpl

#endif // _WX_PRIVATE_SPINCTRL_H_
