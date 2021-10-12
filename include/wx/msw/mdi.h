/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/mdi.h
// Purpose:     MDI (Multiple Document Interface) classes
// Author:      Julian Smart
// Modified by: 2008-10-31 Vadim Zeitlin: derive from the base classes
// Created:     01/02/97
// Copyright:   (c) 1997 Julian Smart
//              (c) 2008 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_MDI_H_
#define _WX_MSW_MDI_H_

#include "wx/geometry/rect.h"

#include <memory>
#include <string>

class WXDLLIMPEXP_FWD_CORE wxAcceleratorTable;


// ---------------------------------------------------------------------------
// wxMDIParentFrame
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMDIParentFrame : public wxMDIParentFrameBase
{
public:
    wxMDIParentFrame() = default;

    wxMDIParentFrame(wxWindow *parent,
                     wxWindowID id,
                     const std::string& title,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                     const std::string& name = wxFrameNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    ~wxMDIParentFrame();

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                const std::string& name = wxFrameNameStr);

    // override/implement base class [pure] virtual methods
    // ----------------------------------------------------

    static bool IsTDI() { return false; }

    // we don't store the active child in m_currentChild so override this
    // function to find it dynamically
    wxMDIChildFrame *GetActiveChild() const override;

    void Cascade() override;
    void Tile(wxOrientation orient = wxHORIZONTAL) override;
    void ArrangeIcons() override;
    void ActivateNext() override;
    void ActivatePrevious() override;

#if wxUSE_MENUS
    void SetWindowMenu(wxMenu* menu) override;

    void DoMenuUpdates(wxMenu* menu = nullptr) override;

    // return the active child menu, if any
    WXHMENU MSWGetActiveMenu() const override;
#endif // wxUSE_MENUS


    // implementation only from now on

    // MDI helpers
    // -----------

#if wxUSE_MENUS
    // called by wxMDIChildFrame after it was successfully created
    virtual void AddMDIChild(wxMDIChildFrame *child);

    // called by wxMDIChildFrame just before it is destroyed
    virtual void RemoveMDIChild(wxMDIChildFrame *child);
#endif // wxUSE_MENUS

    // Retrieve the current window menu label: it can be different from
    // "Window" when using non-English translations and can also be different
    // from wxGetTranslation("Window") if the locale has changed since the
    // "Window" menu was added.
    const wxString& MSWGetCurrentWindowMenuLabel() const
        { return m_currentWindowMenuLabel; }

    // handlers
    // --------

    // Responds to colour changes
    void OnSysColourChanged(wxSysColourChangedEvent& event);

    void OnActivate(wxActivateEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnIconized(wxIconizeEvent& event);

    bool HandleActivate(int state, bool minimized, WXHWND activate);

    // override window proc for MDI-specific message processing
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;

    WXLRESULT MSWDefWindowProc(WXUINT, WXWPARAM, WXLPARAM) override;
    bool MSWTranslateMessage(WXMSG* msg) override;

#if wxUSE_MENUS
    // override the menu-relayed methods to also look in the active child menu
    // bar and the "Window" menu
    wxMenuItem *FindItemInMenuBar(int menuId) const override;
    wxMenu* MSWFindMenuFromHMENU(WXHMENU hMenu) override;
#endif // wxUSE_MENUS

protected:
#if wxUSE_MENUS_NATIVE
    void InternalSetMenuBar() override;
#endif // wxUSE_MENUS_NATIVE

    WXHICON GetDefaultIcon() const override;

    // set the size of the MDI client window to match the frame size
    void UpdateClientSize();

private:
    // holds the current translation for the window menu label
    wxString m_currentWindowMenuLabel;

#if wxUSE_MENUS
    // "Window" menu commands event handlers
    void OnMDICommand(wxCommandEvent& event);
    void OnMDIChild(wxCommandEvent& event);


    // add/remove window menu if we have it (i.e. m_windowMenu != NULL)
    void AddWindowMenu();
    void RemoveWindowMenu();

    // update the window menu (if we have it) to enable or disable the commands
    // which only make sense when we have more than one child
    void UpdateWindowMenu(bool enable);

#if wxUSE_ACCEL
    std::unique_ptr<wxAcceleratorTable> m_accelWindowMenu;
#endif // wxUSE_ACCEL
#endif // wxUSE_MENUS

    // return the number of child frames we currently have (maybe 0)
    int GetChildFramesCount() const;

    // if true, indicates whether the event wasn't really processed even though
    // it was "handled", see OnActivate() and HandleActivate()
    bool m_activationNotHandled{false};

    friend class WXDLLIMPEXP_FWD_CORE wxMDIChildFrame;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxMDIParentFrame);
};

// ---------------------------------------------------------------------------
// wxMDIChildFrame
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMDIChildFrame : public wxMDIChildFrameBase
{
public:
    wxMDIChildFrame() = default;
    wxMDIChildFrame(wxMDIParentFrame *parent,
                    wxWindowID id,
                    const std::string& title,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = wxDEFAULT_FRAME_STYLE,
                    const std::string& name = wxFrameNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }

    wxMDIChildFrame(const wxMDIChildFrame&) = delete;
    wxMDIChildFrame& operator=(const wxMDIChildFrame&) = delete;
    wxMDIChildFrame(wxMDIChildFrame&&) = default;
    wxMDIChildFrame& operator=(wxMDIChildFrame&&) = default;

    [[maybe_unused]] bool Create(wxMDIParentFrame *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                const std::string& name = wxFrameNameStr);

    ~wxMDIChildFrame();

    // implement MDI operations
    void Activate() override;

    // Override some frame operations too
    void Maximize(bool maximize = true) override;
    void Restore() override;

    bool Show(bool show = true) override;

    // Implementation only from now on
    // -------------------------------

    // Handlers
    bool HandleMDIActivate(long bActivate, WXHWND, WXHWND);
    bool HandleWindowPosChanging(void *lpPos);
    bool HandleGetMinMaxInfo(void *mmInfo);

    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;
    WXLRESULT MSWDefWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;
    bool MSWTranslateMessage(WXMSG *msg) override;

    void MSWDestroyWindow() override;

    bool ResetWindowStyle(void *vrect);

    void OnIdle(wxIdleEvent& event);

protected:
    wxPoint DoGetScreenPosition() const override;
    wxPoint DoGetPosition() const override;
    void DoSetSize(wxRect boundary, unsigned int sizeFlags) override;
    void DoSetClientSize(int width, int height) override;
    void InternalSetMenuBar() override;
    bool IsMDIChild() const override { return true; }
    void DetachMenuBar() override;

    WXHICON GetDefaultIcon() const override;

private:
    bool m_needsResize{true}; // flag which tells us to artificially resize the frame

    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

// ---------------------------------------------------------------------------
// wxMDIClientWindow
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxMDIClientWindow : public wxMDIClientWindowBase
{
public:
    wxMDIClientWindow() = default;

    wxMDIClientWindow(const wxMDIClientWindow&) = delete;
    wxMDIClientWindow& operator=(const wxMDIClientWindow&) = delete;
    wxMDIClientWindow(wxMDIClientWindow&&) = default;
    wxMDIClientWindow& operator=(wxMDIClientWindow&&) = default;

    // Note: this is virtual, to allow overridden behaviour.
    bool CreateClient(wxMDIParentFrame *parent,
                              unsigned int style = wxVSCROLL | wxHSCROLL) override;

    // Explicitly call default scroll behaviour
    void OnScroll(wxScrollEvent& event);

protected:
    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;

    int m_scrollX{0};
    int m_scrollY{0};

private:
    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_MDI_H_
