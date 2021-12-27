/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mdi.h
// Purpose:     wxMDI base header
// Author:      Julian Smart (original)
//              Vadim Zeitlin (base MDI classes refactoring)
// Copyright:   (c) 1998 Julian Smart
//              (c) 2008 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MDI_H_BASE_
#define _WX_MDI_H_BASE_

#if wxUSE_MDI

#include "wx/frame.h"
#include "wx/menu.h"

import Utils.Geometry;

#include <memory>
import <string>;

// forward declarations
class wxMDIParentFrame;
class wxMDIChildFrame;
class wxMDIClientWindowBase;
class wxMDIClientWindow;

// ----------------------------------------------------------------------------
// wxMDIParentFrameBase: base class for parent frame for MDI children
// ----------------------------------------------------------------------------

class wxMDIParentFrameBase : public wxFrame
{
public:
    /*
        Derived classes should provide ctor and Create() with the following
        declaration:

    bool Create(wxWindow *parent,
                wxWindowID winid,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                std::string_view name = wxFrameNameStr);
     */

    // Get or change the active MDI child window
    virtual wxMDIChildFrame *GetActiveChild() const
        { return m_currentChild; }
    virtual void SetActiveChild(wxMDIChildFrame *child)
        { m_currentChild = child; }


    // Get the client window
    wxMDIClientWindowBase *GetClientWindow() const { return m_clientWindow; }


    // MDI windows menu functions
    // --------------------------

#if wxUSE_MENUS
    // return the pointer to the current window menu or NULL if we don't have
    // because of wxFRAME_NO_WINDOW_MENU style
    wxMenu* GetWindowMenu() const { return m_windowMenu.get(); }

    // use the given menu instead of the default window menu
    //
    // menu can be NULL to disable the window menu completely
    virtual void SetWindowMenu(wxMenu* menu)
    {
        if ( menu != m_windowMenu.get() )
        {
            m_windowMenu.reset(menu);

        }
    }
#endif // wxUSE_MENUS


    // standard MDI window management functions
    // ----------------------------------------

    virtual void Cascade() { }
    virtual void Tile([[maybe_unused]] wxOrientation orient = wxHORIZONTAL) { }
    virtual void ArrangeIcons() { }
    virtual void ActivateNext() = 0;
    virtual void ActivatePrevious() = 0;

    /*
        Derived classes must provide the following function:

    static bool IsTDI();
    */

    // Create the client window class (don't Create() the window here, just
    // return a new object of a wxMDIClientWindow-derived class)
    //
    // Notice that if you override this method you should use the default
    // constructor and Create() and not the constructor creating the window
    // when creating the frame or your overridden version is not going to be
    // called (as the call to a virtual function from ctor will be dispatched
    // to this class version)
    virtual wxMDIClientWindow *OnCreateClient();

protected:
    // Override to pass menu/toolbar events to the active child first.
    bool TryBefore(wxEvent& event) override;


    // This is wxMDIClientWindow for all the native implementations but not for
    // the generic MDI version which has its own wxGenericMDIClientWindow and
    // so we store it as just a base class pointer because we don't need its
    // exact type anyhow
    wxMDIClientWindowBase *m_clientWindow{nullptr};
    wxMDIChildFrame *m_currentChild{nullptr};

#if wxUSE_MENUS
    // the current window menu or NULL if we are not using it
    std::unique_ptr<wxMenu> m_windowMenu;
#endif // wxUSE_MENUS
};

// ----------------------------------------------------------------------------
// wxMDIChildFrameBase: child frame managed by wxMDIParentFrame
// ----------------------------------------------------------------------------

class wxMDIChildFrameBase : public wxFrame
{
public:
    /*
        Derived classes should provide Create() with the following signature:

    bool Create(wxMDIParentFrame *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr);

        And setting m_mdiParent to parent parameter.
     */

    // MDI children specific methods
    virtual void Activate() = 0;

    // Return the MDI parent frame: notice that it may not be the same as
    // GetParent() (our parent may be the client window or even its subwindow
    // in some implementations)
    wxMDIParentFrame *GetMDIParent() const { return m_mdiParent; }

    // Synonym for GetMDIParent(), was used in some other ports
    wxMDIParentFrame *GetMDIParentFrame() const { return GetMDIParent(); }


    // in most ports MDI children frames are not really top-level, the only
    // exception are the Mac ports in which MDI children are just normal top
    // level windows too
    bool IsTopLevel() const override { return false; }

    // In all ports keyboard navigation must stop at MDI child frame level and
    // can't cross its boundary. Indicate this by overriding this function to
    // return true.
    bool IsTopNavigationDomain(NavigationKind kind) const override
    {
        switch ( kind )
        {
            case NavigationKind::Tab:
                return true;

            case NavigationKind::Accel:
                // Parent frame accelerators should work inside MDI child, so
                // don't block their processing by returning true for them.
                break;
        }

        return false;
    }

    // Raising any frame is supposed to show it but wxFrame Raise()
    // implementation doesn't work for MDI child frames in most forms so
    // forward this to Activate() which serves the same purpose by default.
    void Raise() override { Activate(); }

protected:
    wxMDIParentFrame *m_mdiParent{nullptr};
};

// ----------------------------------------------------------------------------
// wxTDIChildFrame: child frame used by TDI implementations
// ----------------------------------------------------------------------------

class wxTDIChildFrame : public wxMDIChildFrameBase
{
public:
    // override wxFrame methods for this non top-level window

#if wxUSE_STATUSBAR
    // no status bars
    //
    // TODO: MDI children should have their own status bars, why not?
    wxStatusBar* CreateStatusBar([[maybe_unused]] int number = 1,
                                 [[maybe_unused]] unsigned int style = 1,
                                 [[maybe_unused]] wxWindowID id = 1,
                                 [[maybe_unused]] std::string_view name = {}) override
      { return nullptr; }

    wxStatusBar *GetStatusBar() const override
        { return nullptr; }
    void SetStatusText([[maybe_unused]] std::string_view text,
                       [[maybe_unused]] int number=0) override
        { }
    void SetStatusWidths([[maybe_unused]] int n,
                         [[maybe_unused]] const int widths[]) override
        { }
#endif // wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
    // no toolbar
    //
    // TODO: again, it should be possible to have tool bars
    wxToolBar *CreateToolBar([[maybe_unused]] unsigned int style,
                             [[maybe_unused]] wxWindowID id,
                             [[maybe_unused]] std::string_view name) override
        { return nullptr; }
    wxToolBar *GetToolBar() const override { return nullptr; }
#endif // wxUSE_TOOLBAR

    // no icon
    void SetIcons([[maybe_unused]] const wxIconBundle& icons) override { }

    // title is used as the tab label
    std::string GetTitle() const override { return m_title; }
    void SetTitle(const std::string& title) override = 0;

    // no maximize etc
    void Maximize([[maybe_unused]] bool maximize = true) override { }
    bool wxIsMaximized() const override { return true; }
    bool IsAlwaysMaximized() const override { return true; }
    void Iconize([[maybe_unused]] bool iconize = true) override { }
    bool IsIconized() const override { return false; }
    void Restore() override { }

    bool ShowFullScreen([[maybe_unused]] bool show,
                        [[maybe_unused]] unsigned int style) override { return false; }
    bool IsFullScreen() const override { return false; }


    // we need to override these functions to ensure that a child window is
    // created even though we derive from wxFrame -- basically we make it
    // behave as just a wxWindow by short-circuiting wxTLW changes to the base
    // class behaviour

    void AddChild(wxWindowBase *child) override { wxWindow::AddChild(child); }

    bool Destroy() override { return wxWindow::Destroy(); }

    // extra platform-specific hacks
#ifdef __WXMSW__
    DWORD MSWGetStyle(unsigned int flags, DWORD *exstyle = nullptr) const override
    {
        return wxWindow::MSWGetStyle(flags, exstyle);
    }

    WXHWND MSWGetParent() const override
    {
        return wxWindow::MSWGetParent();
    }

    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override
    {
        return wxWindow::MSWWindowProc(message, wParam, lParam);
    }
#endif // __WXMSW__

protected:
    wxSize DoGetSize() const override
    {
        return wxWindow::DoGetSize();
    }

    void DoSetSize(wxRect boundary, unsigned int sizeFlags) override
    {
        wxWindow::DoSetSize(boundary, sizeFlags);
    }

    wxSize DoGetClientSize() const override
    {
        return wxWindow::DoGetClientSize();
    }

    void DoSetClientSize(int width, int height) override
    {
        wxWindow::DoSetClientSize(width, height);
    }

    void DoMoveWindow(wxRect boundary) override
    {
        wxWindow::DoMoveWindow(boundary);
    }

    wxPoint DoGetScreenPosition() const override
    {
        return wxWindow::DoGetScreenPosition();
    }

    // no size hints
    void DoSetSizeHints([[maybe_unused]] int minW, [[maybe_unused]] int minH,
                        [[maybe_unused]] int maxW, [[maybe_unused]] int maxH,
                        [[maybe_unused]] int incW, [[maybe_unused]] int incH) override { }

    std::string m_title;
};

// ----------------------------------------------------------------------------
// wxMDIClientWindowBase: child of parent frame, parent of children frames
// ----------------------------------------------------------------------------

class wxMDIClientWindowBase : public wxWindow
{
public:
    /*
        The derived class must provide the default ctor only (CreateClient()
        will be called later).
    */

    // Can be overridden in the derived classes but the base class version must
    // be usually called first to really create the client window.
    virtual bool CreateClient(wxMDIParentFrame *parent,
                              unsigned int style = wxVSCROLL | wxHSCROLL) = 0;
};

// ----------------------------------------------------------------------------
// Include the port-specific implementation of the base classes defined above
// ----------------------------------------------------------------------------

// wxUSE_GENERIC_MDI_AS_NATIVE may be predefined to force the generic MDI
// implementation use even on the platforms which usually don't use it
//
// notice that generic MDI can still be used without this, but you would need
// to explicitly use wxGenericMDIXXX classes in your code (and currently also
// add src/generic/mdig.cpp to your build as it's not compiled in if generic
// MDI is not used by default -- but this may change later...)
#ifndef wxUSE_GENERIC_MDI_AS_NATIVE
    // wxUniv always uses the generic MDI implementation and so do the ports
    // without native version (although wxCocoa seems to have one -- but it's
    // probably not functional?)
    #if defined(__WXMOTIF__) || \
        defined(__WXUNIVERSAL__)
        #define wxUSE_GENERIC_MDI_AS_NATIVE   1
    #else
        #define wxUSE_GENERIC_MDI_AS_NATIVE   0
    #endif
#endif // wxUSE_GENERIC_MDI_AS_NATIVE

#if wxUSE_GENERIC_MDI_AS_NATIVE
    #include "wx/generic/mdig.h"
#elif defined(__WXMSW__)
    #include "wx/msw/mdi.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/mdi.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/mdi.h"
#elif defined(__WXMAC__)
    #include "wx/osx/mdi.h"
#elif defined(__WXQT__)
    #include "wx/qt/mdi.h"
#endif

inline wxMDIClientWindow *wxMDIParentFrameBase::OnCreateClient()
{
    return new wxMDIClientWindow;
}

inline bool wxMDIParentFrameBase::TryBefore(wxEvent& event)
{
    // Menu (and toolbar) events should be sent to the active child frame
    // first, if any.
    if ( event.GetEventType() == wxEVT_MENU ||
            event.GetEventType() == wxEVT_UPDATE_UI )
    {
        wxMDIChildFrame * const child = GetActiveChild();
        if ( child )
        {
            // However avoid sending the event back to the child if it's
            // currently being propagated to us from it.
            auto* const
                from = dynamic_cast<wxWindow*>(event.GetPropagatedFrom());
            if ( !from || !from->IsDescendant(child) )
            {
                if ( child->ProcessWindowEventLocally(event) )
                    return true;
            }
        }
    }

    return wxFrame::TryBefore(event);
}

#endif // wxUSE_MDI

#endif // _WX_MDI_H_BASE_
