/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cursor.h
// Purpose:     wxCursor base header
// Author:      Julian Smart, Vadim Zeitlin
// Created:
// Copyright:   (c) Julian Smart
//              (c) 2014 Vadim Zeitlin (wxCursorBase)
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CURSOR_H_BASE_
#define _WX_CURSOR_H_BASE_

#include "wx/gdiobj.h"
#include "wx/gdicmn.h"
#include "wx/geometry/point.h"

// Under most ports, wxCursor derives directly from wxGDIObject, but in wxMSW
// there is an intermediate wxGDIImage class.
#ifdef __WXMSW__
    #include "wx/msw/gdiimage.h"
#else
    using wxGDIImage = wxGDIObject;
#endif

#include <string>

class wxCursorBase : public wxGDIImage
{
public:
/*
    wxCursor classes should provide the following ctors:

    wxCursor();
    wxCursor(const wxImage& image);
    wxCursor(const std::string& name,
             wxBitmapType type = wxCURSOR_DEFAULT_TYPE,
             int hotSpotX = 0, int hotSpotY = 0);
    wxCursor(wxStockCursor id) { InitFromStock(id); }
#if WXWIN_COMPATIBILITY_2_8
    wxCursor(int id) { InitFromStock((wxStockCursor)id); }
#endif
*/

    virtual wxPoint GetHotSpot() const { return wxDefaultPosition; }
};

#if defined(__WXMSW__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::CUR_Resource
    #include "wx/msw/cursor.h"
#elif defined(__WXMOTIF__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::XBM
    #include "wx/motif/cursor.h"
#elif defined(__WXGTK20__)
    #ifdef WX_WINDOWS
        #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::CUR_Resource
    #else
        #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::XPM
    #endif
    #include "wx/gtk/cursor.h"
#elif defined(__WXGTK__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::XPM
    #include "wx/gtk1/cursor.h"
#elif defined(__WXX11__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::XPM
    #include "wx/x11/cursor.h"
#elif defined(__WXDFB__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::CUR_Resource
    #include "wx/dfb/cursor.h"
#elif defined(__WXMAC__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::MACCURSOR_Resource
    #include "wx/osx/cursor.h"
#elif defined(__WXQT__)
    #define wxCURSOR_DEFAULT_TYPE   wxBitmapType::CUR
    #include "wx/qt/cursor.h"
#endif

#include "wx/utils.h"

/* This is a small class which can be used by all ports
   to temporarily suspend the busy cursor. Useful in modal
   dialogs.

   Actually that is not (any longer) quite true..  currently it is
   only used in wxGTK Dialog::ShowModal() and now uses static
   wxBusyCursor methods that are only implemented for wxGTK so far.
   The BusyCursor handling code should probably be implemented in
   common code somewhere instead of the separate implementations we
   currently have.  Also the name BusyCursorSuspender is a little
   misleading since it doesn't actually suspend the BusyCursor, just
   masks one that is already showing.
   If another call to wxBeginBusyCursor is made while this is active
   the Busy Cursor will again be shown.  But at least now it doesn't
   interfere with the state of wxIsBusy() -- RL

*/
class wxBusyCursorSuspender
{
public:
    wxBusyCursorSuspender()
    {
        if( wxIsBusy() )
        {
            wxSetCursor( wxBusyCursor::GetStoredCursor() );
        }
    }
    ~wxBusyCursorSuspender()
    {
        if( wxIsBusy() )
        {
            wxSetCursor( wxBusyCursor::GetBusyCursor() );
        }
    }
};
#endif
    // _WX_CURSOR_H_BASE_
