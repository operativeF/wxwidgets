///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/tooltip.cpp
// Purpose:     wxToolTip class implementation for MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     31.01.99
// Copyright:   (c) 1999 Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_TOOLTIPS

#include "wx/tooltip.h"

#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"
#include "wx/msw/private.h"

#include "wx/app.h"
#include "wx/control.h"
#include "wx/module.h"
#include "wx/toplevel.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>
#include <fmt/core.h>

#include <chrono>

import WX.WinDef;
import Utils.Strings;

import <string>;
import <vector>;

#ifndef TTTOOLINFO_V1_SIZE
    #define TTTOOLINFO_V1_SIZE 0x28
#endif

#ifndef TTF_TRANSPARENT
    #define TTF_TRANSPARENT 0x0100
#endif

// VZ: normally, the trick with subclassing the tooltip control and processing
//     TTM_WINDOWFROMPOINT should work but, somehow, it doesn't. I leave the
//     code here for now (but it's not compiled) in case we need it later.
//
//     For now I use an ugly workaround and process TTN_NEEDTEXT directly in
//     radio button wnd proc - fixing TTM_WINDOWFROMPOINT code would be nice
//     because it would then work for all controls, not only radioboxes but for
//     now I don't understand what's wrong with it...
#define wxUSE_TTM_WINDOWFROMPOINT   0

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

// the tooltip parent window
WXHWND wxToolTip::ms_hwndTT = (WXHWND)nullptr;

#if wxUSE_TTM_WINDOWFROMPOINT

// the tooltip window proc
static WNDPROC gs_wndprocToolTip = (WNDPROC)NULL;

#endif // wxUSE_TTM_WINDOWFROMPOINT

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// This is simply a wrapper for vector<WXHWND> but defined as a class to hide the
// details from the public header.
class wxToolTipOtherWindows : public std::vector<WXHWND>
{
};

class wxToolInfo : public TOOLINFOW
{
public:
    wxToolInfo(WXHWND hwndOwner, unsigned int id, const wxRect& rc)
    {
        // initialize all members
        ::ZeroMemory(this, sizeof(TOOLINFOW));

        // the structure TOOLINFO has been extended with a 4 byte field in
        // version 4.70 of comctl32.dll and another one in 5.01 but we don't
        // use these extended fields so use the old struct size to ensure that
        // the tooltips work on old (Windows 95) systems too
        cbSize = TTTOOLINFO_V1_SIZE;

        hwnd = hwndOwner;

        if (rc.IsEmpty())
        {
            uFlags = TTF_IDISHWND;
            uId = (UINT_PTR)hwndOwner;
        }
        else
        {
            // this tooltip must be shown only if the mouse hovers a specific rect
            // of the hwnd parameter!
            rect.left = rc.GetLeft();
            rect.right = rc.GetRight();
            rect.top = rc.GetTop();
            rect.bottom = rc.GetBottom();

            // note that not setting TTF_IDISHWND from the uFlags member means that the
            // ti.uId field should not contain the WXHWND but rather as MSDN says an
            // "Application-defined identifier of the tool"; this is used internally by
            // Windows to distinguish the different tooltips attached to the same window
            uId = id;
        }

        // we use TTF_TRANSPARENT to fix a problem which arises at least with
        // the text controls but may presumably happen with other controls
        // which display the tooltip at mouse position: it can start flashing
        // then as the control gets "focus lost" events and dismisses the
        // tooltip which then reappears because mouse remains hovering over the
        // control, see SF patch 1821229
        uFlags |= TTF_TRANSPARENT;
    }
};

// Takes care of deleting ToolTip control window when shutting down the library.
class wxToolTipModule : public wxModule
{
public:
    bool OnInit() override
    {
        return true;
    }

    void OnExit() override
    {
        wxToolTip::DeleteToolTipCtrl();
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxToolTipModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxToolTipModule, wxModule);

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// send a message to the tooltip control if it exists
//
// NB: wParam is always 0 for the TTM_XXX messages we use
static inline LRESULT SendTooltipMessage(WXHWND hwnd, WXUINT msg, void *lParam)
{
    return hwnd ? ::SendMessageW((WXHWND)hwnd, msg, 0, (WXLPARAM)lParam) : 0;
}

// send a message to all existing tooltip controls
static inline void
SendTooltipMessageToAll(WXHWND hwnd, WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( hwnd )
        ::SendMessageW((WXHWND)hwnd, msg, wParam, lParam);
}

// ============================================================================
// implementation
// ============================================================================

#if wxUSE_TTM_WINDOWFROMPOINT

// ----------------------------------------------------------------------------
// window proc for our tooltip control
// ----------------------------------------------------------------------------

LRESULT APIENTRY wxToolTipWndProc(WXHWND hwndTT,
                                  WXUINT msg,
                                  WXWPARAM wParam,
                                  WXLPARAM lParam)
{
    if ( msg == TTM_WINDOWFROMPOINT )
    {
        LPPOINT ppt = (LPPOINT)lParam;

        // the window on which event occurred
        WXHWND hwnd = ::WindowFromPoint(*ppt);

        OutputDebugString("TTM_WINDOWFROMPOINT: ");
        OutputDebugString(fmt::format("0x{:08x} => ", hwnd));

        // return a WXHWND corresponding to a wxWindow because only wxWidgets are
        // associated with tooltips using TTM_ADDTOOL
        wxWindow *win = wxGetWindowFromHWND((WXHWND)hwnd);

        if ( win )
        {
            hwnd = GetHwndOf(win);
            OutputDebugString(fmt::format("0x{:08x}\r\n", hwnd));

#if 0
            // modify the point too!
            RECT rect;
            GetWindowRect(hwnd, &rect);

            ppt->x = (rect.right - rect.left) / 2;
            ppt->y = (rect.bottom - rect.top) / 2;
#endif // 0
            return (LRESULT)hwnd;
        }
        else
        {
            OutputDebugString("no window\r\n");
        }
    }

    return ::CallWindowProc(CASTWNDPROC gs_wndprocToolTip, hwndTT, msg, wParam, lParam);
}

#endif // wxUSE_TTM_WINDOWFROMPOINT

// ----------------------------------------------------------------------------
// static functions
// ----------------------------------------------------------------------------

void wxToolTip::Enable(bool flag)
{
    // Make sure the tooltip has been created
    GetToolTipCtrl();

    SendTooltipMessageToAll(ms_hwndTT, TTM_ACTIVATE, flag, 0);
}

void wxToolTip::SetDelay(std::chrono::milliseconds delay)
{
    // Make sure the tooltip has been created
    GetToolTipCtrl();

    SendTooltipMessageToAll(ms_hwndTT, TTM_SETDELAYTIME,
                            TTDT_INITIAL, delay.count());
}

void wxToolTip::SetAutoPop(std::chrono::milliseconds autopopTime)
{
    SendTooltipMessageToAll(ms_hwndTT, TTM_SETDELAYTIME,
                            TTDT_AUTOPOP, autopopTime.count());
}

void wxToolTip::SetReshow(std::chrono::milliseconds reshowDelay)
{
    SendTooltipMessageToAll(ms_hwndTT, TTM_SETDELAYTIME,
                            TTDT_RESHOW, reshowDelay.count());
}

void wxToolTip::SetMaxWidth(int width)
{
    wxASSERT_MSG( width == -1 || width >= 0, "invalid width value" );

    ms_maxWidth = width;
}

void wxToolTip::DeleteToolTipCtrl()
{
    if ( ms_hwndTT )
    {
        ::DestroyWindow((WXHWND)ms_hwndTT);
        ms_hwndTT = (WXHWND)nullptr;
    }
}

// ---------------------------------------------------------------------------
// implementation helpers
// ---------------------------------------------------------------------------

// create the tooltip ctrl for our parent frame if it doesn't exist yet
/* static */
WXHWND wxToolTip::GetToolTipCtrl()
{
    if ( !ms_hwndTT )
    {
        WXDWORD exflags = 0;
        if ( wxApp::MSWGetDefaultLayout() == wxLayoutDirection::RightToLeft )
        {
            exflags |= WS_EX_LAYOUTRTL;
        }

        // we want to show the tooltips always (even when the window is not
        // active) and we don't want to strip "&"s from them
        ms_hwndTT = (WXHWND)::CreateWindowExW(exflags,
                                             TOOLTIPS_CLASSW,
                                             (LPCWSTR)nullptr,
                                             TTS_ALWAYSTIP | TTS_NOPREFIX,
                                             CW_USEDEFAULT, CW_USEDEFAULT,
                                             CW_USEDEFAULT, CW_USEDEFAULT,
                                             nullptr, (WXHMENU)nullptr,
                                             wxGetInstance(),
                                             nullptr);
       if ( ms_hwndTT )
       {
           WXHWND hwnd = (WXHWND)ms_hwndTT;
           ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

#if wxUSE_TTM_WINDOWFROMPOINT
           // subclass the newly created control
           gs_wndprocToolTip = wxSetWindowProc(hwnd, wxToolTipWndProc);
#endif // wxUSE_TTM_WINDOWFROMPOINT
       }
    }

    return ms_hwndTT;
}

/* static */
void wxToolTip::UpdateVisibility()
{
    wxToolInfo ti(nullptr, 0, wxRect());
    ti.uFlags = 0;

    if ( !SendTooltipMessage(ms_hwndTT, TTM_GETCURRENTTOOL, &ti) )
        return;

    wxWindow* const associatedWindow = wxFindWinFromHandle(ti.hwnd);
    if ( !associatedWindow )
        return;

    bool hideTT = false;
    if ( !associatedWindow->IsShownOnScreen() )
    {
        // If the associated window or its parent is hidden, the tooltip
        // shouldn't remain shown.
        hideTT = true;
    }
    else
    {
        // Even if it's not hidden, it could also be iconized.
        wxTopLevelWindow* const
            frame = dynamic_cast<wxTopLevelWindow*>(wxGetTopLevelParent(associatedWindow));

        if ( frame && frame->IsIconized() )
            hideTT = true;
    }

    if ( hideTT )
        ::ShowWindow(ms_hwndTT, SW_HIDE);
}

/* static */
void wxToolTip::RelayEvent(WXMSG *msg)
{
    SendTooltipMessage(GetToolTipCtrl(), TTM_RELAYEVENT, msg);
}

// ----------------------------------------------------------------------------
// ctor & dtor
// ----------------------------------------------------------------------------

wxToolTip::wxToolTip(const std::string& tip)
         : m_text(tip)
{
    // make sure m_rect.IsEmpty() == true
    m_rect.SetWidth(0);
    m_rect.SetHeight(0);
}

wxToolTip::wxToolTip(wxWindow* win, unsigned int id, const std::string& tip, const wxRect& rc)
         : m_text(tip), m_rect(rc), m_id(id)
{
    SetWindow(win);
}

wxToolTip::~wxToolTip()
{
    // the tooltip has to be removed before deleting. Otherwise, if it is visible
    // while being deleted, there will be a delay before it goes away.
    Remove();

    delete m_others;
}

// ----------------------------------------------------------------------------
// others
// ----------------------------------------------------------------------------

/* static */
void wxToolTip::Remove(WXHWND hWnd, unsigned int id, const wxRect& rc)
{
    wxToolInfo ti((WXHWND)hWnd, id, rc);

    SendTooltipMessage(GetToolTipCtrl(), TTM_DELTOOL, &ti);
}

void wxToolTip::DoRemove(WXHWND hWnd)
{
    if ( m_window && hWnd == m_window->GetHWND() )
    {
        // Remove the tooltip from the main window.
        Remove(hWnd, m_id, m_rect);
    }
    else
    {
        // Not really sure what to pass to remove in this case...
        Remove(hWnd, 0, wxRect());
    }
}

void wxToolTip::Remove()
{
    DoForAllWindows(&wxToolTip::DoRemove);
}

void wxToolTip::AddOtherWindow(WXHWND hWnd)
{
    if ( !m_others )
        m_others = new wxToolTipOtherWindows;

    m_others->push_back(hWnd);

    DoAddHWND(hWnd);
}

void wxToolTip::DoAddHWND(WXHWND hWnd)
{
    WXHWND hwnd = (WXHWND)hWnd;

    wxToolInfo ti(hwnd, m_id, m_rect);

    // another possibility would be to specify LPSTR_TEXTCALLBACK here as we
    // store the tooltip text ourselves anyhow, and provide it in response to
    // TTN_NEEDTEXT (sent via WM_NOTIFY), but then we would be limited to 79
    // character tooltips as this is the size of the szText buffer in
    // NMTTDISPINFO struct -- and setting the tooltip here we can have tooltips
    // of any length
    ti.hwnd = hwnd;

    boost::nowide::wstackstring stackText(m_text.c_str());
    ti.lpszText = stackText.get();

    if ( !SendTooltipMessage(GetToolTipCtrl(), TTM_ADDTOOL, &ti) )
    {
        wxLogDebug("Failed to create the tooltip '%s'", m_text.c_str());

        return;
    }

#ifdef TTM_SETMAXTIPWIDTH
    if ( !AdjustMaxWidth() )
#endif // TTM_SETMAXTIPWIDTH
    {
        // replace the '\n's with spaces because otherwise they appear as
        // unprintable characters in the tooltip string
        wx::utils::ReplaceAll(m_text, "\n", " ");
        boost::nowide::wstackstring noNewLineText(m_text.c_str());
        ti.lpszText = noNewLineText.get();

        if ( !SendTooltipMessage(GetToolTipCtrl(), TTM_ADDTOOL, &ti) )
        {
            wxLogDebug("Failed to create the tooltip '%s'", m_text.c_str());
        }
    }
}

void wxToolTip::SetWindow(wxWindow *win)
{
    Remove();

    m_window = win;

    // add the window itself
    if ( m_window )
    {
        DoAddHWND(m_window->GetHWND());
    }
#if !defined(__WXUNIVERSAL__)
    // and all of its subcontrols (e.g. radio buttons in a radiobox) as well
    auto* control = dynamic_cast<wxControl*>(m_window);
    if ( control )
    {
        for ( const auto id : control->GetSubcontrols() )
        {
            WXHWND hwnd = ::GetDlgItem(GetHwndOf(m_window), id);
            if ( !hwnd )
            {
                // maybe it's a child of parent of the control, in fact?
                // (radiobuttons are subcontrols, i.e. children of the radiobox
                // for wxWidgets but are its siblings at Windows level)
                hwnd = ::GetDlgItem(GetHwndOf(m_window->GetParent()), id);
            }

            // must have it by now!
            wxASSERT_MSG( hwnd, "no hwnd for subcontrol?" );

            AddOtherWindow((WXHWND)hwnd);
        }
    }
#endif // !defined(__WXUNIVERSAL__)
}

void wxToolTip::SetRect(const wxRect& rc)
{
    m_rect = rc;

    if ( m_window )
    {
        wxToolInfo ti(GetHwndOf(m_window), m_id, m_rect);
        SendTooltipMessage(GetToolTipCtrl(), TTM_NEWTOOLRECT, &ti);
    }
}

void wxToolTip::SetTip(const std::string& tip)
{
    m_text = tip;

#ifdef TTM_SETMAXTIPWIDTH
    if ( !AdjustMaxWidth() )
#endif // TTM_SETMAXTIPWIDTH
    {
        // replace the '\n's with spaces because otherwise they appear as
        // unprintable characters in the tooltip string
        wx::utils::ReplaceAll(m_text, "\n", " ");
    }

    DoForAllWindows(&wxToolTip::DoSetTip);
}

void wxToolTip::DoSetTip(WXHWND hWnd)
{
    // update the tip text shown by the control
    wxToolInfo ti((WXHWND)hWnd, m_id, m_rect);

    // for some reason, changing the tooltip text directly results in
    // repaint of the controls under it, see #10520 -- but this doesn't
    // happen if we reset it first
    // FIXME: Not sure about the necessity of const_cast here.
    ti.lpszText = nullptr;
    SendTooltipMessage(GetToolTipCtrl(), TTM_UPDATETIPTEXT, &ti);
    boost::nowide::wstackstring stackText(m_text.c_str());
    ti.lpszText = stackText.get();
    SendTooltipMessage(GetToolTipCtrl(), TTM_UPDATETIPTEXT, &ti);
}

bool wxToolTip::AdjustMaxWidth()
{
    // use TTM_SETMAXTIPWIDTH to make tooltip multiline using the
    // extent of its first line as max value
    WXHFONT hfont = (WXHFONT)
        ::SendTooltipMessage(GetToolTipCtrl(), WM_GETFONT, nullptr);

    if ( !hfont )
    {
        hfont = (WXHFONT)::GetStockObject(DEFAULT_GUI_FONT);
        if ( !hfont )
        {
            wxLogLastError("GetStockObject(DEFAULT_GUI_FONT)");
        }
    }

    MemoryHDC hdc;
    if ( !hdc )
    {
        wxLogLastError("CreateCompatibleDC(NULL)");
    }

    if ( !SelectObject(hdc, hfont) )
    {
        wxLogLastError("SelectObject(hfont)");
    }

    // find the width of the widest line
    int maxWidth = 0;
    wxStringTokenizer tokenizer(m_text, "\n");
    while ( tokenizer.HasMoreTokens() )
    {
        const auto token = tokenizer.GetNextToken();

        SIZE sz;
        boost::nowide::wstackstring stackToken{token.c_str()};
        if ( !::GetTextExtentPoint32W(hdc, stackToken.get(),
                                     token.length(), &sz) )
        {
            wxLogLastError("GetTextExtentPoint32");
        }

        if ( sz.cx > maxWidth )
            maxWidth = sz.cx;
    }

    // limit size to ms_maxWidth, if set
    if ( ms_maxWidth == 0 )
    {
        // this is more or less arbitrary but seems to work well
        static constexpr int DEFAULT_MAX_WIDTH = 400;

        ms_maxWidth = wxGetClientDisplayRect().width / 2;

        if ( ms_maxWidth > DEFAULT_MAX_WIDTH )
            ms_maxWidth = DEFAULT_MAX_WIDTH;
    }

    if ( ms_maxWidth != -1 && maxWidth > ms_maxWidth )
        maxWidth = ms_maxWidth;

    // only set a new width if it is bigger than the current setting:
    // otherwise adding a tooltip with shorter line(s) than a previous
    // one would result in breaking the longer lines unnecessarily as
    // all our tooltips share the same maximal width
    if ( maxWidth > SendTooltipMessage(GetToolTipCtrl(),
                                       TTM_GETMAXTIPWIDTH, nullptr) )
    {
        SendTooltipMessage(GetToolTipCtrl(), TTM_SETMAXTIPWIDTH,
                           wxUIntToPtr(maxWidth));
    }

    return true;
}

void wxToolTip::DoForAllWindows(void (wxToolTip::*func)(WXHWND))
{
    if ( m_window )
    {
        (this->*func)(m_window->GetHWND());
    }

    if ( m_others )
    {
        for ( wxToolTipOtherWindows::const_iterator it = m_others->begin();
              it != m_others->end();
              ++it )
        {
            (this->*func)(*it);
        }
    }
}

#endif // wxUSE_TOOLTIPS
