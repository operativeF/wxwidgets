/////////////////////////////////////////////////////////////////////////
// File:        wx/msw/taskbar.h
// Purpose:     Defines wxTaskBarIcon class for manipulating icons on the
//              Windows task bar.
// Author:      Julian Smart
// Modified by: Vaclav Slavik
// Created:     24/3/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////

#ifndef _WX_TASKBAR_H_
#define _WX_TASKBAR_H_

#include "wx/icon.h"

#include <chrono>
#include <memory>

using namespace std::chrono_literals;

// private helper class:
class wxTaskBarIconWindow;

class wxTaskBarIcon : public wxTaskBarIconBase
{
public:
    wxTaskBarIcon(wxTaskBarIconType iconType = wxTaskBarIconType::DefaultType);
    ~wxTaskBarIcon();

    // Accessors
    bool IsOk() const { return true; }
    bool IsIconInstalled() const { return m_iconAdded; }

    // Operations
    bool SetIcon(const wxIcon& icon, const std::string& tooltip = {}) override;
    bool RemoveIcon() override;
    bool PopupMenu(wxMenu *menu) override;

    // MSW-specific class methods

#if wxUSE_TASKBARICON_BALLOONS
    // show a balloon notification (the icon must have been already initialized
    // using SetIcon)
    //
    // title and text are limited to 63 and 255 characters respectively, msec
    // is the timeout, in milliseconds, before the balloon disappears (will be
    // clamped down to the allowed 10-30s range by Windows if it's outside it)
    // and flags can include wxICON_ERROR/INFO/WARNING to show a corresponding
    // icon
    //
    // return true if balloon was shown, false on error (incorrect parameters
    // or function unsupported by OS)
    bool ShowBalloon(const std::string& title,
                     const std::string& text,
                     std::chrono::milliseconds timeoutDuration = 0ms,
                     unsigned int flags = 0,
                     const wxIcon& icon = wxNullIcon);
#endif // wxUSE_TASKBARICON_BALLOONS

protected:
    friend class wxTaskBarIconWindow;

    long WindowProc(unsigned int msg, unsigned int wParam, long lParam);
    void RegisterWindowMessages();

private:
    std::unique_ptr<wxTaskBarIconWindow> m_win;
    bool                 m_iconAdded{false};
    wxIcon               m_icon;
    std::string          m_strTooltip;

    enum class Operation
    {
        Add,
        Modify,
        TryBoth
    };

    // Implementation of the public SetIcon() which may also be used when we
    // don't know if we should add a new icon or modify the existing one.
    bool DoSetIcon(const wxIcon& icon,
                   const std::string& tooltip,
                   Operation operation);
};

#endif // _WX_TASKBAR_H_
