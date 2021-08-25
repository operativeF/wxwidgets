///////////////////////////////////////////////////////////////////////////////
// Name:        wx/nativewin.h
// Purpose:     classes allowing to wrap a native window handle
// Author:      Vadim Zeitlin
// Created:     2008-03-05
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_NATIVEWIN_H_
#define _WX_NATIVEWIN_H_

#include "wx/toplevel.h"

// These symbols can be tested in the user code to see if the current wx port
// has support for creating wxNativeContainerWindow and wxNativeWindow from
// native windows.
//
// Be optimistic by default, we undefine them below if necessary.
#define wxHAS_NATIVE_CONTAINER_WINDOW
#define wxHAS_NATIVE_WINDOW

// we define the following typedefs for each of the platform supporting native
// windows wrapping:
//
//  - wxNativeContainerWindowHandle is the toolkit-level handle of the native
//    window, i.e. HWND/GdkWindow*/NSWindow
//
//  - wxNativeContainerWindowId is the lowest level identifier of the native
//    window, i.e. HWND/GdkNativeWindow/NSWindow (so it's the same as above for
//    all platforms except GTK where we also can work with Window/XID)
//
//  - wxNativeWindowHandle for child windows, i.e. HWND/GtkWidget*/NSControl
#if defined(__WXMSW__)
    #include "wx/msw/wrapwin.h"

    using wxNativeContainerWindowId = HWND;
    using wxNativeContainerWindowHandle = HWND;
    using wxNativeWindowHandle = HWND;
#elif defined(__WXGTK__)
    // GdkNativeWindow is guint32 under GDK/X11 and gpointer under GDK/WIN32
    #ifdef __UNIX__
        using wxNativeContainerWindowId = unsigned long;
    #else
        using wxNativeContainerWindowId = void*;
    #endif
    using wxNativeContainerWindowHandle = GdkWindow*;
    using wxNativeWindowHandle = GtkWidget*;
#elif defined(__WXOSX_COCOA__)
    using wxNativeWindowHandle = NSView*;

    // no support for using native TLWs yet
    #undef wxHAS_NATIVE_CONTAINER_WINDOW
#else
    // no support for using native windows under this platform yet
    #undef wxHAS_NATIVE_CONTAINER_WINDOW
    #undef wxHAS_NATIVE_WINDOW
#endif

#ifdef wxHAS_NATIVE_WINDOW

// ----------------------------------------------------------------------------
// wxNativeWindow: for using native windows inside wxWidgets windows
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxNativeWindow : public wxWindow
{
public:
    // Default ctor, Create() must be called later to really create the window.
    wxNativeWindow() = default;

    wxNativeWindow(const wxNativeWindow&) = delete;
    wxNativeWindow& operator=(const wxNativeWindow&) = delete;
    wxNativeWindow(wxNativeWindow&&) = default;
    wxNativeWindow& operator=(wxNativeWindow&&) = default;

    // Create a window from an existing native window handle.
    //
    // Notice that this ctor doesn't take the usual pos and size parameters,
    // they're taken from the window handle itself.
    //
    // Use GetHandle() to check if the creation was successful, it will return
    // 0 if the handle was invalid.
    wxNativeWindow(wxWindow* parent, wxWindowID winid, wxNativeWindowHandle handle)
    {    
        Create(parent, winid, handle);
    }

    // Same as non-default ctor, but with a return code.
    [[maybe_unused]] bool Create(wxWindow* parent, wxWindowID winid, wxNativeWindowHandle handle);

    // By default the native window with which this wxWindow is associated is
    // owned by the user code and needs to be destroyed by it in a platform
    // specific way, however this function can be called to let wxNativeWindow
    // dtor take care of destroying the native window instead of having to do
    // it from the user code.
    void Disown()
    {
        wxCHECK_RET( m_ownedByUser, wxS("Can't disown more than once") );

        m_ownedByUser = false;

        DoDisown();
    }

#ifdef __WXMSW__
    // Prevent the native window, not owned by us, from being destroyed by the
    // base class dtor, unless Disown() had been called.
    ~wxNativeWindow() override;
#endif // __WXMSW__

private:
    // This is implemented in platform-specific code.
    void DoDisown();

    // If the native widget owned by the user code.
    bool m_ownedByUser{true};
};

#endif // wxHAS_NATIVE_WINDOW

#ifdef wxHAS_NATIVE_CONTAINER_WINDOW

// ----------------------------------------------------------------------------
// wxNativeContainerWindow: can be used for creating other wxWindows inside it
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxNativeContainerWindow : public wxTopLevelWindow
{
public:
    // default ctor, call Create() later
    wxNativeContainerWindow() = default;

    // create a window from an existing native window handle
    //
    // use GetHandle() to check if the creation was successful, it will return
    // 0 if the handle was invalid
    wxNativeContainerWindow(wxNativeContainerWindowHandle handle)
    {
        Create(handle);
    }

    // same as ctor above but with a return code
    [[maybe_unused]] bool Create(wxNativeContainerWindowHandle handle);

#if defined(__WXGTK__)
    // this is a convenient ctor for wxGTK applications which can also create
    // the objects of this class from the really native window handles and not
    // only the GdkWindow objects
    //
    // wxNativeContainerWindowId is Window (i.e. an XID, i.e. an int) under X11
    // (when GDK_WINDOWING_X11 is defined) or HWND under Win32
    wxNativeContainerWindow(wxNativeContainerWindowId winid) { Create(winid); }

    [[maybe_unused]] bool Create(wxNativeContainerWindowId winid);
#endif // wxGTK

    // unlike for the normal windows, dtor will not destroy the native window
    // as it normally doesn't belong to us
    ~wxNativeContainerWindow() override;

   wxNativeContainerWindow(const wxNativeContainerWindow&) = delete;
   wxNativeContainerWindow& operator=(const wxNativeContainerWindow&) = delete;
   wxNativeContainerWindow(wxNativeContainerWindow&&) = default;
   wxNativeContainerWindow& operator=(wxNativeContainerWindow&&) = default;

    // provide (trivial) implementation of the base class pure virtuals
    void SetTitle(const std::string& WXUNUSED(title)) override
    {
        wxFAIL_MSG( "not implemented for native windows" );
    }

    std::string GetTitle() const override
    {
        wxFAIL_MSG( "not implemented for native windows" );

        return {};
    }

    void Maximize(bool WXUNUSED(maximize) = true) override
    {
        wxFAIL_MSG( "not implemented for native windows" );
    }

    bool IsMaximized() const override
    {
        wxFAIL_MSG( "not implemented for native windows" );

        return false;
    }

    void Iconize(bool WXUNUSED(iconize) = true) override
    {
        wxFAIL_MSG( "not implemented for native windows" );
    }

    bool IsIconized() const override
    {
        // this is called by wxGTK implementation so don't assert
        return false;
    }

    void Restore() override
    {
        wxFAIL_MSG( "not implemented for native windows" );
    }

    bool ShowFullScreen(bool WXUNUSED(show),
                                long WXUNUSED(style) = wxFULLSCREEN_ALL) override
    {
        wxFAIL_MSG( "not implemented for native windows" );

        return false;
    }

    bool IsFullScreen() const override
    {
        wxFAIL_MSG( "not implemented for native windows" );

        return false;
    }

#ifdef __WXMSW__
    bool IsShown() const override;
#endif // __WXMSW__

    // this is an implementation detail: called when the native window is
    // destroyed by an outside agency; deletes the C++ object too but can in
    // principle be overridden to something else (knowing that the window
    // handle of this object and all of its children is invalid any more)
    virtual void OnNativeDestroyed();

protected:
#ifdef __WXMSW__
    WXLRESULT
    MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;
#endif // __WXMSW__
};

#endif // wxHAS_NATIVE_CONTAINER_WINDOW

#endif // _WX_NATIVEWIN_H_

