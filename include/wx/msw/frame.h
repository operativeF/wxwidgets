/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/frame.h
// Purpose:     wxFrame class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FRAME_H_
#define _WX_FRAME_H_

import WX.WinDef;

import <string>;

#if wxUSE_TASKBARBUTTON
#include <wx/taskbarbutton.h>
#endif

class wxFrame : public wxFrameBase
{
public:
    wxFrame() = default;
    wxFrame(wxWindow *parent,
            wxWindowID id,
            const std::string& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxDEFAULT_FRAME_STYLE,
            std::string_view name = wxFrameNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                std::string_view name = wxFrameNameStr);
    
    bool ShowFullScreen(bool show, unsigned int style = wxFULLSCREEN_ALL) override;

    // event handlers
    void OnSysColourChanged(wxSysColourChangedEvent& event);

    // Toolbar
#if wxUSE_TOOLBAR
    wxToolBar* CreateToolBar(unsigned int style = -1,
                             wxWindowID id = wxID_ANY,
                             std::string_view name = std::string_view{"toolbar"}) override;
#endif // wxUSE_TOOLBAR

    // Status bar
#if wxUSE_STATUSBAR
    wxStatusBar* OnCreateStatusBar(int number = 1,
                                   unsigned int style = wxSTB_DEFAULT_STYLE,
                                   wxWindowID id = 0,
                                   std::string_view name = wxStatusLineNameStr) override;

    // Hint to tell framework which status bar to use: the default is to use
    // native one for the platforms which support it (Win32), the generic one
    // otherwise

    // TODO: should this go into a wxFrameworkSettings class perhaps?
    static void UseNativeStatusBar(bool useNative)
        { m_useNativeStatusBar = useNative; }
    static bool UsesNativeStatusBar()
        { return m_useNativeStatusBar; }
#endif // wxUSE_STATUSBAR

    // event handlers
    bool HandleSize(int x, int y, WXUINT flag);
    bool HandleCommand(WXWORD id, WXWORD cmd, WXHWND control);

    // tooltip management
#if wxUSE_TOOLTIPS
    WXHWND GetToolTipCtrl() const { return m_hwndToolTip; }
    void SetToolTipCtrl(WXHWND hwndTT) { m_hwndToolTip = hwndTT; }
#endif // tooltips

    // override the base class function to handle iconized/maximized frames
    void SendSizeEvent(unsigned int flags = 0) override;

    wxPoint GetClientAreaOrigin() const override;

    // override base class version to add menu bar accel processing
    bool MSWTranslateMessage(WXMSG *msg) override
    {
        return MSWDoTranslateMessage(this, msg);
    }

    // window proc for the frames
    WXLRESULT MSWWindowProc(WXUINT message,
                                    WXWPARAM wParam,
                                    WXLPARAM lParam) override;

#if wxUSE_MENUS
    // get the currently active menu: this is the same as the frame menu for
    // normal frames but is overridden by wxMDIParentFrame
    virtual WXHMENU MSWGetActiveMenu() const { return m_hMenu; }

    bool HandleMenuSelect(WXWORD nItem, WXWORD nFlags, WXHMENU hMenu) override;
    bool DoSendMenuOpenCloseEvent(wxEventType evtType, wxMenu* menu) override;

    // Look up the menu in the menu bar.
    wxMenu* MSWFindMenuFromHMENU(WXHMENU hMenu) override;
#endif // wxUSE_MENUS

#if wxUSE_TASKBARBUTTON
    std::unique_ptr<wxTaskBarButton> m_taskBarButton;
    
    // Return the taskbar button of the window.
    //
    // The pointer returned by this method belongs to the window and will be
    // deleted when the window itself is, do not delete it yourself. May return
    // NULL if the initialization of taskbar button failed.
    wxTaskBarButton* MSWGetTaskBarButton();
#endif // wxUSE_TASKBARBUTTON

protected:
    // override base class virtuals
    wxSize DoGetClientSize() const override;
    void DoSetClientSize(int width, int height) override;

#if wxUSE_MENUS_NATIVE
    // perform MSW-specific action when menubar is changed
    void AttachMenuBar(wxMenuBar *menubar) override;

    // a plug in for MDI frame classes which need to do something special when
    // the menubar is set
    virtual void InternalSetMenuBar();
#endif // wxUSE_MENUS_NATIVE

    // propagate our state change to all child frames
    void IconizeChildFrames(bool bIconize);

    // the real implementation of MSWTranslateMessage(), also used by
    // wxMDIChildFrame
    bool MSWDoTranslateMessage(wxFrame *frame, WXMSG *msg);

    virtual bool IsMDIChild() const { return false; }

    // get default (wxWidgets) icon for the frame
    virtual WXHICON GetDefaultIcon() const;

#if wxUSE_TOOLBAR
    void PositionToolBar() override;
#endif // wxUSE_TOOLBAR

#if wxUSE_TOOLTIPS
    WXHWND                m_hwndToolTip{nullptr};
#endif // tooltips

#if wxUSE_STATUSBAR
    void PositionStatusBar() override;

#if wxUSE_MENUS
    // frame menu, NULL if none
    WXHMENU m_hMenu{nullptr};

    // The number of currently opened menus: 0 initially, 1 when a top level
    // menu is opened, 2 when its submenu is opened and so on.
    int m_menuDepth{0};
#endif // wxUSE_MENUS

private:
#if wxUSE_NATIVE_STATUSBAR
    inline static bool m_useNativeStatusBar = true;
#else
    inline static bool m_useNativeStatusBar = false;
#endif

#endif // wxUSE_STATUSBAR

    // used by IconizeChildFrames(), see comments there
    bool m_wasMinimized{false};

    wxDECLARE_EVENT_TABLE();
};

#endif
    // _WX_FRAME_H_
