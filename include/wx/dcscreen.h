/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcscreen.h
// Purpose:     wxScreenDC base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCSCREEN_H_BASE_
#define _WX_DCSCREEN_H_BASE_

#include "wx/dc.h"

import Utils.Geometry;

class wxScreenDC : public wxDC
{
public:
    wxScreenDC();

    static bool StartDrawingOnTop([[maybe_unused]] wxWindow * window)
        { return true; }
    static bool StartDrawingOnTop([[maybe_unused]] wxRect * rect =  nullptr)
        { return true; }
    static bool EndDrawingOnTop()
        { return true; }
};


#endif
    // _WX_DCSCREEN_H_BASE_
