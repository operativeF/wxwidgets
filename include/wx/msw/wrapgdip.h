///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/wrapgdip.h
// Purpose:     wrapper around <gdiplus.h> header
// Author:      Vadim Zeitlin
// Created:     2007-03-15
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_WRAPGDIP_H_
#define _WX_MSW_WRAPGDIP_H_

import Utils.Wrap.Windows;

// min and max must be available for gdiplus.h but we cannot define them as
// macros because they conflict with std::numeric_limits<T>::min and max when
// compiling with mingw-w64 and -std=c++17. This happens because with c++17,
// math.h includes bessel_function which requires std::numeric_limits.

import <cmath>;
using std::min;
using std::max;

#include <gdiplus.h>
using namespace Gdiplus;

#endif // _WX_MSW_WRAPGDIP_H_

