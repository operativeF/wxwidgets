/////////////////////////////////////////////////////////////////////////////
// Name:        wx/wxprec.h
// Purpose:     Includes the appropriate files for precompiled headers
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// compiler detection; includes setup.h
#include "wx/defs.h"

// check if to use precompiled headers: do it for most Windows compilers unless
// explicitly disabled by defining NOPCH
#if defined(__VISUALC__)
    // If user did not request NOCPH and we're not building using configure
    // then assume user wants precompiled headers.
    #if !defined(NOPCH) && !defined(__WX_SETUP_H__)
        #define WX_PRECOMP
    #endif
#endif

#ifdef WX_PRECOMP

// include "wx/chartype.h" first to ensure that UNICODE macro is correctly set
// _before_ including <windows.h>
#include "wx/chartype.h"

// include standard Windows headers
#if defined(WX_WINDOWS)
    #include "wx/msw/wrapwin.h"
    #include "wx/msw/private.h"
#endif
#if defined(__WXMSW__)
    #include "wx/msw/wrapcctl.h"
    #include "wx/msw/wrapcdlg.h"
    #include "wx/msw/wrap/utils.h"
#endif

// Include standard headers
import <algorithm>;
import <array>;
#include <cassert>
#include <cerrno>
import <cctype>;
import <charconv>;
#include <chrono>
import <cmath>;
import <cstdlib>;
import <cstdio>;
import <cstring>;
import <ctime>;
import <filesystem>;
import <functional>;
import <iostream>;
import <limits>;
#include <memory>
import <numeric>;
import <numbers>;
import <span>;
import <string>;
import <string_view>;
import <tuple>;
import <unordered_map>;
import <vector>;

// Include external library headers that are necessary.
#include <fmt/core.h>
#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>
#include <gsl/gsl>

#endif // WX_PRECOMP
