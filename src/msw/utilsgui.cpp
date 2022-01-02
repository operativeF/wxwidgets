///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/utilsgui.cpp
// Purpose:     Various utility functions only available in wxMSW GUI
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003 (extracted from msw/utils.cpp)
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"     // includes <windows.h>

#include "wx/cursor.h"
#include "wx/window.h"
#include "wx/utils.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import WX.WinDef;
import WX.Win.UniqueHnd;

import <string>;

// ============================================================================
// implementation
// ============================================================================

// Emit a beeeeeep
void wxBell()
{
    ::MessageBeep((WXUINT)-1);        // default sound
}


// Check whether this window wants to process messages, e.g. Stop button
// in long calculations.
bool wxCheckForInterrupt(wxWindow *wnd)
{
    wxCHECK( wnd, false );

    MSG msg;
    while ( ::PeekMessageW(&msg, GetHwndOf(wnd), 0, 0, PM_REMOVE) )
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    return true;
}

// ---------------------------------------------------------------------------
// window information functions
// ---------------------------------------------------------------------------

std::string wxGetWindowText(WXHWND hWnd)
{
    if ( hWnd )
    {
        std::wstring windowText;

        const int len = ::GetWindowTextLengthW((WXHWND)hWnd) + 1;

        windowText.resize(len);

        ::GetWindowTextW((WXHWND)hWnd, &windowText[0], len);

        // Avoid bogus double-NUL termination
        windowText.resize(len - 1);

        return boost::nowide::narrow(windowText);
    }

    return {};
}

wxString wxGetWindowClass(WXHWND hWnd)
{
    wxString str;

    if ( hWnd )
    {
        int len = 256; // some starting value

        for ( ;; )
        {
            int count = ::GetClassNameW((WXHWND)hWnd, wxStringBuffer(str, len), len);

            if ( count == len )
            {
                // the class name might have been truncated, retry with larger
                // buffer
                len *= 2;
            }
            else
            {
                break;
            }
        }
    }

    return str;
}

int wxGetWindowId(WXHWND hWnd)
{
    return ::GetWindowLongPtrW((WXHWND)hWnd, GWL_ID);
}

// ----------------------------------------------------------------------------
// Metafile helpers
// ----------------------------------------------------------------------------

void PixelToHIMETRIC(LONG *x, LONG *y, WXHDC hdcRef)
{
    int iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE),
        iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE),
        iWidthPels = GetDeviceCaps(hdcRef, HORZRES),
        iHeightPels = GetDeviceCaps(hdcRef, VERTRES);

    // Take care to use MulDiv() here to avoid overflow.
    *x = ::MulDiv(*x, iWidthMM * 100, iWidthPels);
    *y = ::MulDiv(*y, iHeightMM * 100, iHeightPels);
}

void HIMETRICToPixel(LONG *x, LONG *y, WXHDC hdcRef)
{
    int iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE),
        iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE),
        iWidthPels = GetDeviceCaps(hdcRef, HORZRES),
        iHeightPels = GetDeviceCaps(hdcRef, VERTRES);

    *x = ::MulDiv(*x, iWidthPels, iWidthMM * 100);
    *y = ::MulDiv(*y, iHeightPels, iHeightMM * 100);
}

using msw::utils::unique_dcwnd;

void HIMETRICToPixel(LONG *x, LONG *y)
{
    unique_dcwnd screenDC{::GetDC(nullptr)};

    HIMETRICToPixel(x, y, screenDC.get());
}

void PixelToHIMETRIC(LONG *x, LONG *y)
{
    unique_dcwnd screenDC{::GetDC(nullptr)};

    PixelToHIMETRIC(x, y, screenDC.get());
}

void wxDrawLine(WXHDC hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, nullptr); LineTo(hdc, x2, y2);
}

// Function dedicated to drawing horizontal/vertical lines with solid color
// It fills rectangle representing the line with ::ExtTextOut() API which
// apparently is faster than ::MoveTo()/::LineTo() on DC with a non-rotated
// coordinate system.
void wxDrawHVLine(WXHDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int width)
{
    wxASSERT(x1 == x2 || y1 == y2);

    int w1 = width / 2;
    int w2 = width - w1;
    RECT r;
    if ( y1 == y2 )
    {
        if ( x1 == x2 )
            return;
        ::SetRect(&r, x1, y1 - w1, x2, y1 + w2);
    }
    else
    {
        ::SetRect(&r, x1 - w1, y1, x2 + w2, y2);
    }

    COLORREF bgColorOrig = ::GetBkColor(hdc);
    ::SetBkColor(hdc, color);

    ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, L"", 0, nullptr);

    ::SetBkColor(hdc, bgColorOrig);
}

// ----------------------------------------------------------------------------
// Shell API wrappers
// ----------------------------------------------------------------------------

extern bool wxEnableFileNameAutoComplete(WXHWND hwnd)
{
    HRESULT hr = ::SHAutoComplete(hwnd, 0x10 /* SHACF_FILESYS_ONLY */);
    if ( FAILED(hr) )
    {
        wxLogApiError("SHAutoComplete", hr);
        return false;
    }

    return true;
}
