///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/nonownedwnd.h
// Purpose:     declares wxNonOwnedWindow class
// Author:      Stefan Csomor
// Modified by:
// Created:     2008-03-24
// Copyright:   (c) 2008 Stefan Csomor
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MAC_NONOWNEDWND_H_
#define _WX_MAC_NONOWNEDWND_H_

#include "wx/window.h"

#include "wx/graphics.h"

#if wxUSE_SYSTEM_OPTIONS
    inline constexpr wxChar wxMAC_WINDOW_PLAIN_TRANSITION[] = wxT("mac.window-plain-transition");
#endif

//-----------------------------------------------------------------------------
// wxNonOwnedWindow
//-----------------------------------------------------------------------------

// This class represents "non-owned" window. A window is owned by another
// window if it has a parent and is positioned within the parent. For example,
// wxFrame is non-owned, because even though it can have a parent, it's
// location is independent of it.  This class is for internal use only, it's
// the base class for wxTopLevelWindow and wxPopupWindow.

class wxNonOwnedWindowImpl;

class wxNonOwnedWindow : public wxNonOwnedWindowBase
{
public:
    // constructors and such
    wxNonOwnedWindow() { Init(); }

    wxNonOwnedWindow(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = 0,
                        const wxString& name = wxASCII_STR(wxPanelNameStr))
    {
        Init();

        (void)Create(parent, id, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxASCII_STR(wxPanelNameStr));

    bool Create(wxWindow *parent, WXWindow nativeWindow);

    virtual ~wxNonOwnedWindow();

    virtual void SubclassWin(WXWindow nativeWindow);
    virtual void UnsubclassWin();

    wxPoint GetClientAreaOrigin() const override;

    

    bool SetTransparent(wxByte alpha) override;
    bool CanSetTransparent() override;

    bool SetBackgroundStyle(wxBackgroundStyle style) override;

    void Update() override;

    WXWindow GetWXWindow() const ;
    static wxNonOwnedWindow* GetFromWXWindow( WXWindow win );

    // implementation from now on
    // --------------------------

    // These accessors are Mac-specific and don't exist in other ports.
    const wxRegion& GetShape() const { return m_shape; }
#if wxUSE_GRAPHICS_CONTEXT
    const wxGraphicsPath& GetShapePath() { return m_shapePath; }
#endif // wxUSE_GRAPHICS_CONTEXT

    // activation hooks only necessary for MDI Implementation
    static void MacDelayedDeactivation(long timestamp);
    virtual void MacActivate( long timestamp , bool inIsActivating ) ;

    void SetWindowStyleFlag(long flags) override;

    void Raise() override;
    void Lower() override;
    bool Show( bool show = true ) override;

    void SetExtraStyle(long exStyle) override;

    bool SetBackgroundColour( const wxColour &colour ) override;

    wxNonOwnedWindowImpl* GetNonOwnedPeer() const { return m_nowpeer; }

#if wxOSX_USE_COCOA_OR_IPHONE
    // override the base class method to return an NSWindow instead of NSView
    void *OSXGetViewOrWindow() const override;
#endif // Cocoa

    // osx specific event handling common for all osx-ports

    virtual void HandleActivated( double timestampsec, bool didActivate );
    virtual void HandleResized( double timestampsec );
    virtual void HandleMoved( double timestampsec );
    virtual void HandleResizing( double timestampsec, wxRect* rect );

    void OSXHandleMiniaturize(double WXUNUSED(timestampsec), bool miniaturized);

    void OSXSetIgnoreResizing(bool value) { m_ignoreResizing = value; }

    void WindowWasPainted();

    bool Destroy() override;

protected:
    // common part of all ctors
    void Init();

    wxPoint DoGetPosition() const override;
    wxSize DoGetSize() const override;
    void DoMoveWindow(int x, int y, int width, int height) override;
    wxSize DoGetClientSize() const override;

    virtual bool OSXShowWithEffect(bool show,
                                   wxShowEffect effect,
                                   unsigned timeout) override;

    bool DoClearShape() override;
    bool DoSetRegionShape(const wxRegion& region) override;
#if wxUSE_GRAPHICS_CONTEXT
    bool DoSetPathShape(const wxGraphicsPath& path) override;
#endif // wxUSE_GRAPHICS_CONTEXT

    virtual void WillBeDestroyed();

    wxNonOwnedWindowImpl* m_nowpeer ;

//    wxWindowMac* m_macFocus ;

    static wxNonOwnedWindow *s_macDeactivateWindow;

private :
    static clock_t s_lastFlush;

    wxRegion m_shape;
#if wxUSE_GRAPHICS_CONTEXT
    wxGraphicsPath m_shapePath;
#endif // wxUSE_GRAPHICS_CONTEXT
    bool m_ignoreResizing;
};

// list of all frames and modeless dialogs
extern WXDLLIMPEXP_DATA_CORE(wxWindowList) wxModelessWindows;


#endif // _WX_MAC_NONOWNEDWND_H_
