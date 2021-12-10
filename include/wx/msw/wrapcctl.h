///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/wrapcctl.h
// Purpose:     Wrapper for the standard <commctrl.h> header
// Author:      Vadim Zeitlin
// Modified by:
// Created:     03.08.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_WRAPCCTL_H_
#define _WX_MSW_WRAPCCTL_H_

import WX.WinDef;

// Set Unicode format for a common control
inline void wxSetCCUnicodeFormat(WXHWND hwnd)
{
    ::SendMessageW(hwnd, CCM_SETUNICODEFORMAT, 1, 0);
}

#if wxUSE_GUI
// Return the default font for the common controls
//
// this is implemented in msw/settings.cpp
class wxFont;
extern wxFont wxGetCCDefaultFont();

// this is just a wrapper for HDITEM which we can't use in the public header
// because we don't want to include commctrl.h (and hence windows.h) from there
struct wxHDITEM : public HDITEMW
{
    wxHDITEM()
    {
        ::ZeroMemory(this, sizeof(*this));
    }
};

#endif // wxUSE_GUI

#endif // _WX_MSW_WRAPCCTL_H_
