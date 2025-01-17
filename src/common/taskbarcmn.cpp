/////////////////////////////////////////////////////////////////////////
// File:        src/common/taskbarcmn.cpp
// Purpose:     Common parts of wxTaskBarIcon class
// Author:      Julian Smart
// Modified by:
// Created:     04/04/2003
// Copyright:   (c) Julian Smart, 2003
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////

#if wxUSE_TASKBARICON

#include "wx/taskbar.h"
#include "wx/app.h"
#include "wx/list.h"
#include "wx/menu.h"

// DLL options compatibility check:
WX_CHECK_BUILD_OPTIONS("wxAdvanced")

wxDEFINE_EVENT( wxEVT_TASKBAR_MOVE, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_LEFT_DOWN, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_LEFT_UP, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_RIGHT_DOWN, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_RIGHT_UP, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_LEFT_DCLICK, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_RIGHT_DCLICK, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_BALLOON_TIMEOUT, wxTaskBarIconEvent );
wxDEFINE_EVENT( wxEVT_TASKBAR_BALLOON_CLICK, wxTaskBarIconEvent );


wxBEGIN_EVENT_TABLE(wxTaskBarIconBase, wxEvtHandler)
    EVT_TASKBAR_CLICK(wxTaskBarIconBase::OnRightButtonDown)
wxEND_EVENT_TABLE()

void wxTaskBarIconBase::OnRightButtonDown([[maybe_unused]] wxTaskBarIconEvent& event)
{
    std::unique_ptr<wxMenu> menuDeleter;
    wxMenu *menu = GetPopupMenu();
    if ( !menu )
    {
        menu = CreatePopupMenu();
        if ( !menu )
            return;

        menuDeleter.reset(menu);
    }

    PopupMenu(menu);
}

void wxTaskBarIconBase::Destroy()
{
    wxPendingDelete.Append(this);
}

#endif // wxUSE_TASKBARICON
