/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/splash.cpp
// Purpose:     wxSplashScreen class
// Author:      Julian Smart
// Modified by:
// Created:     28/6/2000
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SPLASH

#ifdef __WXGTK20__
    #include "wx/gtk/private/wrapgtk.h"
#endif

#include "wx/splash.h"
#include "wx/dcmemory.h"
#include "wx/dcclient.h"
#include "wx/display.h"

import Utils.Geometry;

// ----------------------------------------------------------------------------
// wxSplashScreen
// ----------------------------------------------------------------------------

constexpr int wxSPLASH_TIMER_ID =       9999;

wxBEGIN_EVENT_TABLE(wxSplashScreen, wxFrame)
    EVT_TIMER(wxSPLASH_TIMER_ID, wxSplashScreen::OnNotify)
    EVT_CLOSE(wxSplashScreen::OnCloseWindow)
wxEND_EVENT_TABLE()

/* Note that unless we pass a non-default size to the frame, SetClientSize
 * won't work properly under Windows, and the splash screen frame is sized
 * slightly too small.
 */

wxSplashScreen::wxSplashScreen(const wxBitmap& bitmap, unsigned int splashStyle,
                               std::chrono::milliseconds displayTime,
                               wxWindow* parent, wxWindowID id, const wxPoint& pos,
                               const wxSize& size, unsigned int style)
    : wxFrame(parent, id, "", wxPoint(0,0), wxSize(100, 100),
              style | wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR),
      m_splashStyle(splashStyle),
      m_displayTime(displayTime),
      m_window(new wxSplashScreenWindow(bitmap, this, wxID_ANY, pos, size, wxNO_BORDER))
{
    wxEvtHandler::AddFilter(this);

    // splash screen must not be used as parent by the other windows because it
    // is going to disappear soon, indicate it by giving it this special style
    SetExtraStyle(GetExtraStyle() | wxWS_EX_TRANSIENT);

#if defined(__WXGTK20__)
    gtk_window_set_type_hint(GTK_WINDOW(m_widget),
                             GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
#endif
    SetClientSize(std::lround(bitmap.GetScaledWidth()), std::lround(bitmap.GetScaledHeight()));

    if (m_splashStyle & wxSPLASH_CENTRE_ON_PARENT)
        CentreOnParent();
    else if (m_splashStyle & wxSPLASH_CENTRE_ON_SCREEN)
        CentreOnScreen();

    if (m_splashStyle & wxSPLASH_TIMEOUT)
    {
        m_timer.SetOwner(this, wxSPLASH_TIMER_ID);
        m_timer.Start(displayTime, true);
    }

    Show(true);
    m_window->SetFocus();
#if defined( __WXMSW__ ) || defined(__WXMAC__)
    Update(); // Without this, you see a blank screen for an instant
#elif defined(__WXGTK20__)
    // we don't need to do anything at least on wxGTK with GTK+ 2.12.9
#else
    wxYieldIfNeeded(); // Should eliminate this
#endif
}

wxSplashScreen::~wxSplashScreen()
{
    m_timer.Stop();

    wxEvtHandler::RemoveFilter(this);
}

int wxSplashScreen::FilterEvent(wxEvent& event)
{
    const wxEventType t = event.GetEventType();
    if ( t == wxEVT_KEY_DOWN ||
            t == wxEVT_LEFT_DOWN ||
                t == wxEVT_RIGHT_DOWN ||
                    t == wxEVT_MIDDLE_DOWN )
        Close(true);

    return -1;
}

void wxSplashScreen::OnNotify([[maybe_unused]] wxTimerEvent& event)
{
    Close(true);
}

void wxSplashScreen::OnCloseWindow([[maybe_unused]] wxCloseEvent& event)
{
    m_timer.Stop();
    this->Destroy();
}

// ----------------------------------------------------------------------------
// wxSplashScreenWindow
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxSplashScreenWindow, wxWindow)
#ifdef __WXGTK__
    EVT_PAINT(wxSplashScreenWindow::OnPaint)
#endif
    EVT_ERASE_BACKGROUND(wxSplashScreenWindow::OnEraseBackground)
wxEND_EVENT_TABLE()

wxSplashScreenWindow::wxSplashScreenWindow(const wxBitmap& bitmap, wxWindow* parent,
                                           wxWindowID id, const wxPoint& pos,
                                           const wxSize& size, unsigned int style)
    : wxWindow(parent, id, pos, size, style)
    , m_bitmap(bitmap)
{

#if !defined(__WXGTK__) && wxUSE_PALETTE
    const bool hiColour = (wxDisplay().GetDepth() >= 16) ;

    if (bitmap.GetPalette() && !hiColour)
    {
        SetPalette(* bitmap.GetPalette());
    }
#endif
}

// VZ: why don't we do it under wxGTK?
#if !defined(__WXGTK__) && wxUSE_PALETTE
    #define USE_PALETTE_IN_SPLASH
#endif

static void wxDrawSplashBitmap(wxDC& dc, const wxBitmap& bitmap, [[maybe_unused]] int x, [[maybe_unused]] int y)
{
    wxMemoryDC dcMem;

#ifdef USE_PALETTE_IN_SPLASH
    bool hiColour = (wxDisplay().GetDepth() >= 16) ;

    if (bitmap.GetPalette() && !hiColour)
    {
        dcMem.SetPalette(* bitmap.GetPalette());
    }
#endif // USE_PALETTE_IN_SPLASH

    dcMem.SelectObjectAsSource(bitmap);
    dc.Blit(wxPoint{0, 0}, wxSize{std::lround(bitmap.GetScaledWidth()), std::lround(bitmap.GetScaledHeight())}, &dcMem, wxPoint{0, 0}, wxRasterOperationMode::Copy,
            true /* use mask */);
    dcMem.SelectObject(wxNullBitmap);

#ifdef USE_PALETTE_IN_SPLASH
    if (bitmap.GetPalette() && !hiColour)
    {
        dcMem.SetPalette(wxNullPalette);
    }
#endif // USE_PALETTE_IN_SPLASH
}

void wxSplashScreenWindow::OnPaint([[maybe_unused]] wxPaintEvent& event)
{
    wxPaintDC dc(this);
    if (m_bitmap.IsOk())
        wxDrawSplashBitmap(dc, m_bitmap, 0, 0);
}

void wxSplashScreenWindow::OnEraseBackground(wxEraseEvent& event)
{
    if (event.GetDC() && m_bitmap.IsOk())
    {
        wxDrawSplashBitmap(* event.GetDC(), m_bitmap, 0, 0);
    }
    else
    {
        wxClientDC dc(this);
        if (m_bitmap.IsOk())
            wxDrawSplashBitmap(dc, m_bitmap, 0, 0);
    }
}

#endif // wxUSE_SPLASH
