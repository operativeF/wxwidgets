///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/tooltip.h
// Purpose:     wxToolTip class - tooltip control
// Author:      Vadim Zeitlin
// Modified by:
// Created:     31.01.99
// Copyright:   (c) 1999 Robert Roebling, Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_TOOLTIP_H_
#define _WX_MSW_TOOLTIP_H_

#include <chrono>

import WX.WinDef;

import Utils.Geometry;

import <string>;

class wxWindow;
class wxToolTipOtherWindows;

class wxToolTip
{
public:
    wxToolTip(const std::string& tip);
    ~wxToolTip();

    wxToolTip& operator=(wxToolTip&&) = delete;

    // ctor used by wxStatusBar to associate a tooltip to a portion of
    // the status bar window:
    wxToolTip(wxWindow* win, unsigned int id,
              const std::string& tip, const wxRect& rc);

    // tip text
    void SetTip(const std::string& tip);
    const std::string& GetTip() const { return m_text; }

        // the window we're associated with
    void SetWindow(wxWindow *win);
    wxWindow *GetWindow() const { return m_window; }

    // controlling tooltip behaviour: globally change tooltip parameters
        // enable or disable the tooltips globally
    static void Enable(bool flag);
        // set the delay after which the tooltip appears
    static void SetDelay(std::chrono::milliseconds delay);
        // set the delay after which the tooltip disappears or how long the
        // tooltip remains visible
    static void SetAutoPop(std::chrono::milliseconds autopopTime);
        // set the delay between subsequent tooltips to appear
    static void SetReshow(std::chrono::milliseconds reshowDelay);
        // set maximum width for the new tooltips: -1 disables wrapping
        // entirely, 0 restores the default behaviour
    static void SetMaxWidth(int width);

    // implementation only from now on
    // -------------------------------

    // should be called in response to WM_MOUSEMOVE
    static void RelayEvent(WXMSG *msg);

    // add a window to the tooltip control
    void AddOtherWindow(WXHWND hwnd);

    // remove any tooltip from the window
    static void Remove(WXHWND hwnd, unsigned int id, const wxRect& rc);

    // Set the rectangle we're associated with. This rectangle is only used for
    // the main window, not any sub-windows added with Add() so in general it
    // makes sense to use it for tooltips associated with a single window only.
    void SetRect(const wxRect& rc);

    // Called when TLW shown state is changed and hides the tooltip itself if
    // the window it's associated with is hidden.
    static void UpdateVisibility();

private:
    // This module calls our DeleteToolTipCtrl().
    friend class wxToolTipModule;

    // Adds a window other than our main m_window to this tooltip.
    void DoAddHWND(WXHWND hWnd);

    // Perform the specified operation for the given window only.
    void DoSetTip(WXHWND hWnd);
    void DoRemove(WXHWND hWnd);

    // Call the given function for all windows we're associated with.
    void DoForAllWindows(void (wxToolTip::*func)(WXHWND));


    // the one and only one tooltip control we use - never access it directly
    // but use GetToolTipCtrl() which will create it when needed
    static WXHWND ms_hwndTT;

    // create the tooltip ctrl if it doesn't exist yet and return its WXHWND
    [[maybe_unused]] static WXHWND GetToolTipCtrl();

    // to be used in wxModule for deleting tooltip ctrl window when exiting mainloop
    static void DeleteToolTipCtrl();

    // new tooltip maximum width, default value is set on first call to wxToolTip::Add()
    inline static int ms_maxWidth{0};

    // remove this tooltip from the tooltip control
    void Remove();

    // adjust tooltip max width based on current tooltip text
    bool AdjustMaxWidth();

    std::string  m_text;           // tooltip text
    wxWindow* m_window{nullptr};         // main window we're associated with
    wxToolTipOtherWindows *m_others{nullptr}; // other windows associated with it or NULL
    wxRect    m_rect;           // the rect of the window for which this tooltip is shown
                                // (or a rect with width/height == 0 to show it for the entire window)
    unsigned int m_id{0};          // the id of this tooltip (ignored when m_rect width/height is 0)
};

#endif // _WX_MSW_TOOLTIP_H_
