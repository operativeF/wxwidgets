/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/splash.h
// Purpose:     Splash screen class
// Author:      Julian Smart
// Modified by:
// Created:     28/6/2000
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SPLASH_H_
#define _WX_SPLASH_H_

#include "wx/bitmap.h"
#include "wx/eventfilter.h"
#include "wx/frame.h"
#include "wx/timer.h"


/*
 * A window for displaying a splash screen
 */

#define wxSPLASH_CENTRE_ON_PARENT   0x01
#define wxSPLASH_CENTRE_ON_SCREEN   0x02
#define wxSPLASH_NO_CENTRE          0x00
#define wxSPLASH_TIMEOUT            0x04
#define wxSPLASH_NO_TIMEOUT         0x00

class WXDLLIMPEXP_FWD_CORE wxSplashScreenWindow;

/*
 * wxSplashScreen
 */

class WXDLLIMPEXP_CORE wxSplashScreen: public wxFrame,
                                      public wxEventFilter
{
public:
    // for RTTI macros only
    wxSplashScreen() { wxEvtHandler::AddFilter(this); }
    wxSplashScreen(const wxBitmap& bitmap, long splashStyle, int milliseconds,
                   wxWindow* parent, wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxSIMPLE_BORDER|wxFRAME_NO_TASKBAR|wxSTAY_ON_TOP);

    wxSplashScreen(const wxSplashScreen&) = delete;
    wxSplashScreen& operator=(const wxSplashScreen&) = delete;

    ~wxSplashScreen();

    void OnCloseWindow(wxCloseEvent& event);
    void OnNotify(wxTimerEvent& event);

    long GetSplashStyle() const { return m_splashStyle; }
    wxSplashScreenWindow* GetSplashWindow() const { return m_window; }
    int GetTimeout() const { return m_milliseconds; }

    // Override wxEventFilter method to hide splash screen on any user input.
    int FilterEvent(wxEvent& event) override;

protected:
    // Common part of all ctors.
    void Init();

    wxSplashScreenWindow*   m_window{nullptr};
    long                    m_splashStyle{wxSPLASH_NO_TIMEOUT}; // TODO: Correct default style?
    int                     m_milliseconds{0};
    wxTimer                 m_timer;

    wxDECLARE_DYNAMIC_CLASS(wxSplashScreen);
    wxDECLARE_EVENT_TABLE();
};

/*
 * wxSplashScreenWindow
 */

class WXDLLIMPEXP_CORE wxSplashScreenWindow: public wxWindow
{
public:
    wxSplashScreenWindow(const wxBitmap& bitmap,
                         wxWindow* parent,
                         wxWindowID id,
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxDefaultSize,
                         long style = wxNO_BORDER);
    
    wxSplashScreenWindow(const wxSplashScreenWindow&) = delete;
    wxSplashScreenWindow& operator=(const wxSplashScreenWindow&) = delete;

    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);

    void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }
    wxBitmap& GetBitmap() { return m_bitmap; }

protected:
    wxBitmap    m_bitmap;

    wxDECLARE_EVENT_TABLE();
};


#endif
    // _WX_SPLASH_H_
