/////////////////////////////////////////////////////////////////////////////
// Name:        wx/frame.h
// Purpose:     wxFrame class interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     15.11.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FRAME_H_BASE_
#define _WX_FRAME_H_BASE_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/toplevel.h"      // the base class
#include "wx/statusbr.h"
#include "wx/toolbar.h"

#include <memory>
import <string>;

// the default names for various classes
inline constexpr std::string_view wxStatusLineNameStr = "status_line";

#if wxUSE_MENUBAR
    #include <wx/menu.h>
    class wxMenuBar;
#endif

class wxFrame;
class wxMenuItem;
class wxStatusBar;
class wxToolBar;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// wxFrame-specific (i.e. not for wxDialog) styles
//
// Also see the bit summary table in wx/toplevel.h.
inline constexpr unsigned int wxFRAME_NO_TASKBAR      = 0x0002;  // No taskbar button (MSW only)
inline constexpr unsigned int wxFRAME_TOOL_WINDOW     = 0x0004;  // No taskbar button, no system menu
inline constexpr unsigned int wxFRAME_FLOAT_ON_PARENT = 0x0008;  // Always above its parent

// ----------------------------------------------------------------------------
// wxFrame is a top-level window with optional menubar, statusbar and toolbar
//
// For each of *bars, a frame may have several of them, but only one is
// managed by the frame, i.e. resized/moved when the frame is and whose size
// is accounted for in client size calculations - all others should be taken
// care of manually. The CreateXXXBar() functions create this, main, XXXBar,
// but the actual creation is done in OnCreateXXXBar() functions which may be
// overridden to create custom objects instead of standard ones when
// CreateXXXBar() is called.
// ----------------------------------------------------------------------------

class wxFrameBase : public wxTopLevelWindow
{
public:
    ~wxFrameBase();

    wxFrame *New(wxWindow *parent,
                 wxWindowID winid,
                 const std::string& title,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxDEFAULT_FRAME_STYLE,
                 std::string_view name = wxFrameNameStr);

    // frame state
    // -----------

    // get the origin of the client area (which may be different from (0, 0)
    // if the frame has a toolbar) in client coordinates
    wxPoint GetClientAreaOrigin() const override;


    // menu bar functions
    // ------------------

#if wxUSE_MENUS
#if wxUSE_MENUBAR
    virtual void SetMenuBar(wxMenuBar *menubar);
    virtual wxMenuBar *GetMenuBar() const { return m_frameMenuBar.get(); }

    // find the item by id in the frame menu bar: this is an internal function
    // and exists mainly in order to be overridden in the MDI parent frame
    // which also looks at its active child menu bar
    virtual wxMenuItem *FindItemInMenuBar(int menuId) const;
#endif
    // generate menu command corresponding to the given menu item
    //
    // returns true if processed
    bool ProcessCommand(wxMenuItem *item);

    // generate menu command corresponding to the given menu command id
    //
    // returns true if processed
    bool ProcessCommand(int winid);
#else
    bool ProcessCommand([[maybe_unused]] int winid) { return false; }
#endif // wxUSE_MENUS

    // status bar functions
    // --------------------
#if wxUSE_STATUSBAR
    // create the main status bar by calling OnCreateStatusBar()
    virtual wxStatusBar* CreateStatusBar(int number = 1,
                                         unsigned int style = wxSTB_DEFAULT_STYLE,
                                         wxWindowID winid = 0,
                                         std::string_view name = wxStatusLineNameStr);
    // return a new status bar
    virtual wxStatusBar *OnCreateStatusBar(int number,
                                           unsigned int style,
                                           wxWindowID winid,
                                           std::string_view name);
    // get the main status bar
    virtual wxStatusBar *GetStatusBar() const { return m_frameStatusBar.get(); }

    // sets the main status bar
    virtual void SetStatusBar(wxStatusBar *statBar);

    // forward these to status bar
    virtual void SetStatusText(std::string_view text, int number = 0);
    virtual void SetStatusWidths(int n, const int widths_field[]);
    void PushStatusText(const std::string& text, int number = 0);
    void PopStatusText(int number = 0);

    // set the status bar pane the help will be shown in
    void SetStatusBarPane(int n) { m_statusBarPane = n; }
    int GetStatusBarPane() const { return m_statusBarPane; }
#endif // wxUSE_STATUSBAR

    // toolbar functions
    // -----------------

#if wxUSE_TOOLBAR
    // create main toolbar bycalling OnCreateToolBar()
    virtual wxToolBar* CreateToolBar(unsigned int style = -1,
                                     wxWindowID winid = wxID_ANY,
                                     std::string_view name = std::string_view{"toolbar"});
    // return a new toolbar
    virtual wxToolBar *OnCreateToolBar(unsigned int style,
                                       wxWindowID winid,
                                       std::string_view name );

    // get/set the main toolbar
    virtual wxToolBar *GetToolBar() const { return m_frameToolBar.get(); }
    virtual void SetToolBar(wxToolBar *toolbar);
#endif // wxUSE_TOOLBAR

    // implementation only from now on
    // -------------------------------

    // event handlers
#if wxUSE_MENUS
    void OnMenuOpen(wxMenuEvent& event);
#if wxUSE_STATUSBAR
    void OnMenuClose(wxMenuEvent& event);
    void OnMenuHighlight(wxMenuEvent& event);
#endif // wxUSE_STATUSBAR

    // send wxUpdateUIEvents for all menu items in the menubar,
    // or just for menu if non-NULL
    virtual void DoMenuUpdates(wxMenu* menu = nullptr);
#endif // wxUSE_MENUS

    // do the UI update processing for this window
    void UpdateWindowUI(unsigned int flags = wxUPDATE_UI_NONE) override;

    // Implement internal behaviour (menu updating on some platforms)
    void OnInternalIdle() override;

#if wxUSE_MENUS || wxUSE_TOOLBAR
    // show help text for the currently selected menu or toolbar item
    // (typically in the status bar) or hide it and restore the status bar text
    // originally shown before the menu was opened if show == false
    virtual void DoGiveHelp(const std::string& text, bool show);
#endif

    bool IsClientAreaChild(const wxWindow *child) const override
    {
        return !IsOneOfBars(child) && wxTopLevelWindow::IsClientAreaChild(child);
    }

protected:
    // test whether this window makes part of the frame
    bool IsOneOfBars(const wxWindow *win) const override;

#if wxUSE_MENUBAR
    // override to update menu bar position when the frame size changes
    virtual void PositionMenuBar() { }

    // override to do something special when the menu bar is being removed
    // from the frame
    virtual void DetachMenuBar();

    // override to do something special when the menu bar is attached to the
    // frame
    virtual void AttachMenuBar(wxMenuBar *menubar);
#endif // wxUSE_MENUBAR

    // Return true if we should update the menu item state from idle event
    // handler or false if we should delay it until the menu is opened.
    static bool ShouldUpdateMenuFromIdle();

#if wxUSE_STATUSBAR && (wxUSE_MENUS || wxUSE_TOOLBAR)
    // the saved status bar text overwritten by DoGiveHelp()
    std::string m_oldStatusText;

    // the last help string we have shown in the status bar
    std::string m_lastHelpShown;
#endif

#if wxUSE_MENUBAR
    std::unique_ptr<wxMenuBar> m_frameMenuBar;
#endif // wxUSE_MENUBAR

#if wxUSE_TOOLBAR
    // override to update status bar position (or anything else) when
    // something changes
    virtual void PositionToolBar() { }

    std::unique_ptr<wxToolBar> m_frameToolBar;
#endif // wxUSE_TOOLBAR

#if wxUSE_STATUSBAR
    std::unique_ptr<wxStatusBar> m_frameStatusBar;

    // override to update status bar position (or anything else) when
    // something changes
    virtual void PositionStatusBar() { }

    // show the help string for the given menu item using DoGiveHelp() if the
    // given item does have a help string (as determined by FindInMenuBar()),
    // return false if there is no help for such item
    bool ShowMenuHelp(int helpid);
#endif // wxUSE_STATUSBAR

    int m_statusBarPane{0};

#if wxUSE_MENUS
    wxDECLARE_EVENT_TABLE();
#endif // wxUSE_MENUS
};

// include the real class declaration
#if defined(__WXUNIVERSAL__)
    #include "wx/univ/frame.h"
#else // !__WXUNIVERSAL__
    #if defined(__WXMSW__)
        #include "wx/msw/frame.h"
    #elif defined(__WXGTK20__)
        #include "wx/gtk/frame.h"
    #elif defined(__WXGTK__)
        #include "wx/gtk1/frame.h"
    #elif defined(__WXMOTIF__)
        #include "wx/motif/frame.h"
    #elif defined(__WXMAC__)
        #include "wx/osx/frame.h"
    #elif defined(__WXQT__)
        #include "wx/qt/frame.h"
    #endif
#endif

#endif
    // _WX_FRAME_H_BASE_
