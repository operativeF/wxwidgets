/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/framecmn.cpp
// Purpose:     common (for all platforms) wxFrame functions
// Author:      Julian Smart, Vadim Zeitlin
// Created:     01/02/97
// Copyright:   (c) 1998 Robert Roebling and Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#include "wx/frame.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/menuitem.h"
#include "wx/toolbar.h"
#include "wx/statusbr.h"

// ----------------------------------------------------------------------------
// event table
// ----------------------------------------------------------------------------

#if wxUSE_MENUS

wxBEGIN_EVENT_TABLE(wxFrameBase, wxTopLevelWindow)
    EVT_MENU_OPEN(wxFrameBase::OnMenuOpen)
#if wxUSE_STATUSBAR
    EVT_MENU_CLOSE(wxFrameBase::OnMenuClose)

    EVT_MENU_HIGHLIGHT_ALL(wxFrameBase::OnMenuHighlight)
#endif // wxUSE_STATUSBAR
wxEND_EVENT_TABLE()

/* static */
bool wxFrameBase::ShouldUpdateMenuFromIdle()
{
    // Usually this is determined at compile time and is determined by whether
    // the platform supports wxEVT_MENU_OPEN, however in wxGTK we need to also
    // check if we're using the global menu bar as we don't get EVT_MENU_OPEN
    // for it and need to fall back to idle time updating even if normally
    // wxUSE_IDLEMENUUPDATES is set to 0 for wxGTK.
#ifdef __WXGTK20__
    if ( wxApp::GTKIsUsingGlobalMenu() )
        return true;
#endif // !__WXGTK__

    return wxUSE_IDLEMENUUPDATES != 0;
}

#endif // wxUSE_MENUS

// ============================================================================
// implementation
// ============================================================================

wxFrameBase::~wxFrameBase()
{
    SendDestroyEvent();
}

wxFrame *wxFrameBase::New(wxWindow *parent,
                          wxWindowID id,
                          const std::string& title,
                          const wxPoint& pos,
                          const wxSize& size,
                          unsigned int style,
                          std::string_view name)
{
    return new wxFrame(parent, id, title, pos, size, style, name);
}

bool wxFrameBase::IsOneOfBars(const wxWindow *win) const
{
#if wxUSE_MENUBAR
    if ( win == GetMenuBar() )
        return true;
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    if ( win == GetStatusBar() )
        return true;
#endif // wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
    if ( win == GetToolBar() )
        return true;
#endif // wxUSE_TOOLBAR

    wxUnusedVar(win);

    return false;
}

// ----------------------------------------------------------------------------
// wxFrame size management: we exclude the areas taken by menu/status/toolbars
// from the client area, so the client area is what's really available for the
// frame contents
// ----------------------------------------------------------------------------

// get the origin of the client area in the client coordinates
wxPoint wxFrameBase::GetClientAreaOrigin() const
{
    wxPoint pt = wxTopLevelWindow::GetClientAreaOrigin();

#if wxUSE_TOOLBAR && !defined(__WXUNIVERSAL__)
    const wxToolBar *toolbar = GetToolBar();
    if ( toolbar && toolbar->IsShown() )
    {
        const wxSize toolbarSize = toolbar->GetSize();

        if ( toolbar->GetWindowStyleFlag() & wxTB_VERTICAL )
        {
            pt.x += toolbarSize.x;
        }
        else
        {
            pt.y += toolbarSize.y;
        }
    }
#endif // wxUSE_TOOLBAR

    return pt;
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------

#if wxUSE_MENUS

bool wxFrameBase::ProcessCommand(int id)
{
#if wxUSE_MENUBAR
    wxMenuItem* const item = FindItemInMenuBar(id);
    if ( !item )
        return false;

    return ProcessCommand(item);
#else
    return false;
#endif
}

bool wxFrameBase::ProcessCommand(wxMenuItem *item)
{
    wxCHECK_MSG( item, false, "Menu item can't be NULL" );

    if (!item->IsEnabled())
        return true;

    if ((item->GetKind() == wxITEM_RADIO) && item->IsChecked() )
        return true;

    int checked;
    if (item->IsCheckable())
    {
        item->Toggle();

        // use the new value
        checked = item->IsChecked();
    }
    else // Uncheckable item.
    {
        checked = -1;
    }

    wxMenu* const menu = item->GetMenu();
    wxCHECK_MSG( menu, false, "Menu item should be attached to a menu" );

    return menu->SendEvent(item->GetId(), checked);
}

#endif // wxUSE_MENUS

// Do the UI update processing for this window. This is
// provided for the application to call if it wants to
// force a UI update, particularly for the menus and toolbar.
void wxFrameBase::UpdateWindowUI(unsigned int flags)
{
    wxWindowBase::UpdateWindowUI(flags);

#if wxUSE_TOOLBAR
    if (GetToolBar())
        GetToolBar()->UpdateWindowUI(flags);
#endif

#if wxUSE_MENUBAR
    if (GetMenuBar())
    {
        // If coming from an idle event, we only want to update the menus if
        // we're in the wxUSE_IDLEMENUUPDATES configuration, otherwise they
        // will be update when the menu is opened later
        if ( !(flags & wxUPDATE_UI_FROMIDLE) || ShouldUpdateMenuFromIdle() )
            DoMenuUpdates();
    }
#endif // wxUSE_MENUS
}

// ----------------------------------------------------------------------------
// event handlers for status bar updates from menus
// ----------------------------------------------------------------------------

#if wxUSE_MENUS

void wxFrameBase::OnMenuOpen(wxMenuEvent& event)
{
    event.Skip();

    if ( !ShouldUpdateMenuFromIdle() )
    {
        // as we didn't update the menus from idle time, do it now
        DoMenuUpdates(event.GetMenu());
    }
}

#if wxUSE_STATUSBAR

void wxFrameBase::OnMenuHighlight(wxMenuEvent& event)
{
    event.Skip();

    std::ignore = ShowMenuHelp(event.GetMenuId());
}

void wxFrameBase::OnMenuClose(wxMenuEvent& event)
{
    event.Skip();

    DoGiveHelp({}, false);
}

#endif // wxUSE_STATUSBAR

#endif // wxUSE_MENUS

// Implement internal behaviour (menu updating on some platforms)
void wxFrameBase::OnInternalIdle()
{
    wxTopLevelWindow::OnInternalIdle();

#if wxUSE_MENUS
    if ( ShouldUpdateMenuFromIdle() && wxUpdateUIEvent::CanUpdate(this) )
        DoMenuUpdates();
#endif
}

// ----------------------------------------------------------------------------
// status bar stuff
// ----------------------------------------------------------------------------

#if wxUSE_STATUSBAR

wxStatusBar* wxFrameBase::CreateStatusBar(int number,
                                          unsigned int style,
                                          wxWindowID id,
                                          std::string_view name)
{
    // the main status bar can only be created once (or else it should be
    // deleted before calling CreateStatusBar() again)
    wxCHECK_MSG( !m_frameStatusBar, nullptr,
                 "recreating status bar in wxFrame" );

    SetStatusBar(OnCreateStatusBar(number, style, id, name));

    return m_frameStatusBar.get();
}

wxStatusBar *wxFrameBase::OnCreateStatusBar(int number,
                                            unsigned int style,
                                            wxWindowID id,
                                            std::string_view name)
{
    wxStatusBar *statusBar = new wxStatusBar(this, id, style, name);

    statusBar->SetFieldsCount(number);

    return statusBar;
}

void wxFrameBase::SetStatusText(std::string_view text, int number)
{
    wxCHECK_RET( m_frameStatusBar != nullptr, "no statusbar to set text for" );

    m_frameStatusBar->SetStatusText(text, number);
}

void wxFrameBase::SetStatusWidths(int n, const int widths_field[] )
{
    wxCHECK_RET( m_frameStatusBar != nullptr, "no statusbar to set widths for" );

    m_frameStatusBar->SetStatusWidths(n, widths_field);

    PositionStatusBar();
}

void wxFrameBase::PushStatusText(const std::string& text, int number)
{
    wxCHECK_RET( m_frameStatusBar != nullptr, "no statusbar to set text for" );

    m_frameStatusBar->PushStatusText(text, number);
}

void wxFrameBase::PopStatusText(int number)
{
    wxCHECK_RET( m_frameStatusBar != nullptr, "no statusbar to set text for" );

    m_frameStatusBar->PopStatusText(number);
}

bool wxFrameBase::ShowMenuHelp(int menuId)
{
#if wxUSE_MENUS
    // if no help string found, we will clear the status bar text
    //
    // NB: wxID_NONE is used for (sub)menus themselves by wxMSW
    std::string helpString;
    if ( menuId != wxID_SEPARATOR && menuId != wxID_NONE )
    {
        const wxMenuItem * const item = FindItemInMenuBar(menuId);
        if ( item && !item->IsSeparator() )
            helpString = item->GetHelp();

        // notice that it's ok if we don't find the item because it might
        // belong to the popup menu, so don't assert here
    }

    DoGiveHelp(helpString, true);

    return !helpString.empty();
#else // !wxUSE_MENUS
    return false;
#endif // wxUSE_MENUS/!wxUSE_MENUS
}

void wxFrameBase::SetStatusBar(wxStatusBar *statBar)
{
    const bool hadBar = m_frameStatusBar != nullptr;
    m_frameStatusBar.reset(statBar);

    if ( (m_frameStatusBar != nullptr) != hadBar )
    {
        PositionStatusBar();

        Layout();
    }
}

#endif // wxUSE_STATUSBAR

#if wxUSE_MENUS || wxUSE_TOOLBAR
void wxFrameBase::DoGiveHelp(const std::string& help, bool show)
{
#if wxUSE_STATUSBAR
    if ( m_statusBarPane < 0 )
    {
        // status bar messages disabled
        return;
    }

    wxStatusBar *statbar = GetStatusBar();
    if ( !statbar )
        return;

    std::string text;

    if ( show )
    {
        // remember the old status bar text if this is the first time we're
        // called since the menu has been opened as we're going to overwrite it
        // in our DoGiveHelp() and we want to restore it when the menu is
        // closed
        //
        // note that it would be logical to do this in OnMenuOpen() but under
        // MSW we get an EVT_MENU_HIGHLIGHT before EVT_MENU_OPEN, strangely
        // enough, and so this doesn't work and instead we use the ugly trick
        // with using special m_oldStatusText value as "menu opened" (but it is
        // arguably better than adding yet another member variable to wxFrame
        // on all platforms)
        if ( m_oldStatusText.empty() )
        {
            m_oldStatusText = statbar->GetStatusText(m_statusBarPane);
            if ( m_oldStatusText.empty() )
            {
                // use special value to prevent us from doing this the next time
                m_oldStatusText += '\0';
            }
        }

        m_lastHelpShown = help;
        text = help;
    }
    else // hide help, restore the original text
    {
        // clear the last shown help string but remember its value
        std::string lastHelpShown;
        lastHelpShown.swap(m_lastHelpShown);

        // also clear the old status text but remember it too to restore it
        // below
        text.swap(m_oldStatusText);

        if ( statbar->GetStatusText(m_statusBarPane) != lastHelpShown )
        {
            // if the text was changed with an explicit SetStatusText() call
            // from the user code in the meanwhile, do not overwrite it with
            // the old status bar contents -- this is almost certainly not what
            // the user expects and would be very hard to avoid from user code
            return;
        }
    }

    statbar->SetStatusText(text, m_statusBarPane);
#else
    wxUnusedVar(help);
    wxUnusedVar(show);
#endif // wxUSE_STATUSBAR
}
#endif // wxUSE_MENUS || wxUSE_TOOLBAR


// ----------------------------------------------------------------------------
// toolbar stuff
// ----------------------------------------------------------------------------

#if wxUSE_TOOLBAR

wxToolBar* wxFrameBase::CreateToolBar(unsigned int style,
                                      wxWindowID id,
                                      std::string_view name)
{
    // the main toolbar can't be recreated (unless it was explicitly deleted
    // before)
    wxCHECK_MSG( !m_frameToolBar, nullptr,
                 "recreating toolbar in wxFrame" );

    if ( style == -1 )
    {
        // use default style
        //
        // NB: we don't specify the default value in the method declaration
        //     because
        //      a) this allows us to have different defaults for different
        //         platforms (even if we don't have them right now)
        //      b) we don't need to include wx/toolbar.h in the header then
        style = wxTB_DEFAULT_STYLE;
    }

    SetToolBar(OnCreateToolBar(style, id, name));

    return m_frameToolBar.get();
}

wxToolBar* wxFrameBase::OnCreateToolBar(unsigned int style,
                                        wxWindowID id,
                                        std::string_view name)
{
    return new wxToolBar(this, id,
                         wxDefaultPosition, wxDefaultSize,
                         style, name);
}

void wxFrameBase::SetToolBar(wxToolBar *toolbar)
{
    if ( (toolbar != nullptr) != (m_frameToolBar != nullptr) )
    {
        // the toolbar visibility must have changed so we need to both position
        // the toolbar itself (if it appeared) and to relayout the frame
        // contents in any case

        if ( toolbar )
        {
            // we need to assign it to m_frameToolBar for PositionToolBar() to
            // do anything
            m_frameToolBar.reset(toolbar);
            PositionToolBar();
        }
        //else: tricky: do not reset m_frameToolBar yet as otherwise Layout()
        //      wouldn't recognize the (still existing) toolbar as one of our
        //      bars and wouldn't layout the single child of the frame correctly


        // and this is even more tricky: we want Layout() to recognize the
        // old toolbar for the purpose of not counting it among our non-bar
        // children but we don't want to reserve any more space for it so we
        // temporarily hide it
        if ( m_frameToolBar )
            m_frameToolBar->Hide();

        Layout();

        if ( m_frameToolBar )
            m_frameToolBar->Show();
    }

    // this might have been already done above but it's simpler to just always
    // do it unconditionally instead of testing for whether we already did it
    m_frameToolBar.reset(toolbar);
}

#endif // wxUSE_TOOLBAR

// ----------------------------------------------------------------------------
// menus
// ----------------------------------------------------------------------------

#if wxUSE_MENUS

// update all menus
void wxFrameBase::DoMenuUpdates(wxMenu* menu)
{
    if (menu)
    {
        menu->UpdateUI();
    }
#if wxUSE_MENUBAR
    else
    {
        wxMenuBar* bar = GetMenuBar();
        if (bar != nullptr)
            bar->UpdateMenus();
    }
#endif
}

#if wxUSE_MENUBAR

void wxFrameBase::DetachMenuBar()
{
    if ( m_frameMenuBar )
    {
        m_frameMenuBar->Detach();
        m_frameMenuBar = nullptr;
    }
}

void wxFrameBase::AttachMenuBar(wxMenuBar *menubar)
{
    if ( menubar )
    {
        menubar->Attach((wxFrame *)this);
        m_frameMenuBar.reset(menubar);
    }
}

void wxFrameBase::SetMenuBar(wxMenuBar *menubar)
{
    if ( menubar == GetMenuBar() )
    {
        // nothing to do
        return;
    }

    DetachMenuBar();

    this->AttachMenuBar(menubar);
}

wxMenuItem *wxFrameBase::FindItemInMenuBar(int menuId) const
{
    const wxMenuBar * const menuBar = GetMenuBar();

    return menuBar ? menuBar->FindItem(menuId) : nullptr;
}

#endif // wxUSE_MENUBAR

#endif // wxUSE_MENUS
