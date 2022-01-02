///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/paint.h
// Purpose:     Helpers for handling repainting
// Author:      Vadim Zeitlin
// Created:     2020-02-10
// Copyright:   (c) 2020 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_PAINT_H_
#define _WX_MSW_PRIVATE_PAINT_H_

import <stack>;

namespace wxMSWImpl
{

// Data used by WM_PAINT handler
struct PaintData
{
    explicit PaintData(wxWindowMSW* window_)
        : window(window_),
          createdPaintDC(false)
    {
    }

    // The window being repainted (never null).
    wxWindowMSW* const window;

    // True if the user-defined paint handler created wxPaintDC.
    bool createdPaintDC;
};

// this variable is used to check that a paint event handler which processed
// the event did create a wxPaintDC inside its code and called BeginPaint() to
// validate the invalidated window area as otherwise we'd keep getting an
// endless stream of WM_PAINT messages for this window resulting in a lot of
// difficult to debug problems (e.g. impossibility to repaint other windows,
// lack of timer and idle events and so on)
inline std::stack<PaintData> paintStack;

} // namespace wxMSWImpl

#endif // _WX_MSW_PRIVATE_PAINT_H_
