/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/init.h
// Purpose:     Windows-specific wxEntry() overload
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_INIT_H_
#define _WX_MSW_INIT_H_

// ----------------------------------------------------------------------------
// Windows-specific wxEntry() overload and wxIMPLEMENT_WXWIN_MAIN definition
// ----------------------------------------------------------------------------

// wxEntry() overload using the command line for the current process, instead
// of argc/argv provided by the CRT. This is only really useful when using
// Unicode with a compiler not providing wmain() or similar entry point, but is
// always provided for consistency.
extern int wxEntry();

#if wxUSE_GUI

#ifndef SW_SHOWNORMAL
    #define SW_SHOWNORMAL 1
#endif

// WinMain() is always ANSI, even in Unicode build.
using wxCmdLineArgType = char *;

// Windows-only overloads of wxEntry() and wxEntryStart() which take the
// parameters passed to WinMain() instead of those passed to main()
extern bool
    wxEntryStart(WXHINSTANCE hInstance,
                WXHINSTANCE hPrevInstance = nullptr,
                wxCmdLineArgType pCmdLine = nullptr,
                int nCmdShow = SW_SHOWNORMAL);

extern int
    wxEntry(WXHINSTANCE hInstance,
            WXHINSTANCE hPrevInstance = nullptr,
            wxCmdLineArgType pCmdLine = nullptr,
            int nCmdShow = SW_SHOWNORMAL);

#define wxIMPLEMENT_WXWIN_MAIN                                              \
    extern "C" int WINAPI WinMain(WXHINSTANCE hInstance,                      \
                                  WXHINSTANCE hPrevInstance,                  \
                                  wxCmdLineArgType lpCmdLine,               \
                                  int nCmdShow)                             \
    {                                                                       \
        wxDISABLE_DEBUG_SUPPORT();                                          \
                                                                            \
        return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);      \
    }

#endif // wxUSE_GUI

#endif // _WX_MSW_INIT_H_
