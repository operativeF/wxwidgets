///////////////////////////////////////////////////////////////////////////////
// Name:        wx/popupwin.h
// Purpose:     wxPopupWindow interface declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     06.01.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_POPUPWIN_H_BASE_
#define _WX_POPUPWIN_H_BASE_

#if wxUSE_POPUPWIN

#include "wx/nonownedwnd.h"

// ----------------------------------------------------------------------------
// wxPopupWindow specific flags
// ----------------------------------------------------------------------------

// This flag can be used in MSW if some controls are not working with the
// default popup style.
inline constexpr unsigned int wxPU_CONTAINS_CONTROLS = 0x0001;

// ----------------------------------------------------------------------------
// wxPopupWindow: a special kind of top level window used for popup menus,
// combobox popups and such.
// ----------------------------------------------------------------------------

class wxPopupWindowBase : public wxNonOwnedWindow
{
public:
    wxPopupWindowBase& operator=(wxPopupWindowBase&&) = delete;

    // create the popup window
    //
    // style may only contain border flags
    [[maybe_unused]] bool Create(wxWindow *parent, unsigned int style = wxBORDER_NONE);

    // move the popup window to the right position, i.e. such that it is
    // entirely visible
    //
    // the popup is positioned at ptOrigin + size if it opens below and to the
    // right (default), at ptOrigin - sizePopup if it opens above and to the
    // left &c
    //
    // the point must be given in screen coordinates!
    virtual void Position(const wxPoint& ptOrigin,
                          const wxSize& size);

    bool IsTopLevel() const override { return true; }
};


// include the real class declaration
#if defined(__WXMSW__)
    #include "wx/msw/popupwin.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/popupwin.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/popupwin.h"
#elif defined(__WXX11__)
    #include "wx/x11/popupwin.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/popupwin.h"
#elif defined(__WXDFB__)
    #include "wx/dfb/popupwin.h"
#elif defined(__WXMAC__)
    #include "wx/osx/popupwin.h"
#elif defined(__WXQT__)
    #include "wx/qt/popupwin.h"
#else
    #error "wxPopupWindow is not supported under this platform."
#endif

inline wxPopupWindow* wxCurrentPopupWindow{nullptr};

// ----------------------------------------------------------------------------
// wxPopupTransientWindow: a wxPopupWindow which disappears automatically
// when the user clicks mouse outside it or if it loses focus in any other way
// ----------------------------------------------------------------------------

// Define the public API of wxPopupTransientWindow:
class wxPopupTransientWindowBase : public wxPopupWindow
{
public:
    // popup the window (this will show it too) and keep focus at winFocus
    // (or itself if it's NULL), dismiss the popup if we lose focus
    virtual void Popup(wxWindow *focus = nullptr) = 0;

    // hide the window
    virtual void Dismiss() = 0;

    // can the window be dismissed now?
    //
    // VZ: where is this used??
    virtual bool CanDismiss()
        { return true; }

    // called when a mouse is pressed while the popup is shown: return true
    // from here to prevent its normal processing by the popup (which consists
    // in dismissing it if the mouse is clicked outside it)
    virtual bool ProcessLeftDown([[maybe_unused]] wxMouseEvent& event)
        { return false; }

    // Override to implement delayed destruction of this window.
    bool Destroy() override;

protected:
    // this is called when the popup is disappeared because of anything
    // else but direct call to Dismiss()
    virtual void OnDismiss() { }

    // dismiss and notify the derived class
    void DismissAndNotify()
    {
        Dismiss();
        OnDismiss();
    }
};

#ifdef __WXMSW__

class wxPopupTransientWindow : public wxPopupTransientWindowBase
{
public:
    wxPopupTransientWindow() = default;
    wxPopupTransientWindow(wxWindow *parent, int style = wxBORDER_NONE)
        { Create(parent, style); }

    wxPopupTransientWindow& operator=(wxPopupTransientWindow&&) = delete;

    void Popup(wxWindow *focus = nullptr) override;
    void Dismiss() override;

    // Override to handle WM_NCACTIVATE.
    bool MSWHandleMessage(WXLRESULT *result,
                                  WXUINT message,
                                  WXWPARAM wParam,
                                  WXLPARAM lParam) override;

    // Override to dismiss the popup.
    void MSWDismissUnfocusedPopup() override;

private:
    void DismissOnDeactivate();

    wxDECLARE_DYNAMIC_CLASS(wxPopupTransientWindow);
};

#else // !__WXMSW__

class wxPopupWindowHandler;
class wxPopupFocusHandler;

class wxPopupTransientWindow : public wxPopupTransientWindowBase
{
public:
    wxPopupTransientWindow() { Init(); }
    wxPopupTransientWindow(wxWindow *parent, int style = wxBORDER_NONE);

    ~wxPopupTransientWindow();

    wxPopupTransientWindow& operator=(wxPopupTransientWindow&&) = delete;

    void Popup(wxWindow *focus = NULL) override;
    void Dismiss() override;

    // Overridden to grab the input on some platforms
    bool Show( bool show = true ) override;

protected:
    // common part of all ctors
    void Init();

    // remove our event handlers
    void PopHandlers();

    // get alerted when child gets deleted from under us
    void OnDestroy(wxWindowDestroyEvent& event);

#if defined(__WXMAC__) && wxOSX_USE_COCOA_OR_CARBON
    // Check if the mouse needs to be captured or released: we must release
    // when it's inside our window if we want the embedded controls to work.
    void OnIdle(wxIdleEvent& event);
#endif

    // the child of this popup if any
    wxWindow *m_child;

    // the window which has the focus while we're shown
    wxWindow *m_focus;

    // these classes may call our DismissAndNotify()
    friend class wxPopupWindowHandler;
    friend class wxPopupFocusHandler;

    // the handlers we created, may be NULL (if not, must be deleted)
    wxPopupWindowHandler *m_handlerPopup;
    wxPopupFocusHandler  *m_handlerFocus;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxPopupTransientWindow);
};

#endif // __WXMSW__/!__WXMSW__

#if wxUSE_COMBOBOX && defined(__WXUNIVERSAL__)

// ----------------------------------------------------------------------------
// wxPopupComboWindow: wxPopupTransientWindow used by wxComboBox
// ----------------------------------------------------------------------------

class wxComboBox;
class wxComboCtrl;

class wxPopupComboWindow : public wxPopupTransientWindow
{
public:
    wxPopupComboWindow() { m_combo = NULL; }
    wxPopupComboWindow(wxComboCtrl *parent);

    [[maybe_unused]] bool Create(wxComboCtrl *parent);

    // position the window correctly relatively to the combo
    void PositionNearCombo();

protected:
    // notify the combo here
    virtual void OnDismiss();

    // forward the key presses to the combobox
    void OnKeyDown(wxKeyEvent& event);

    // the parent combobox
    wxComboCtrl *m_combo;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxPopupComboWindow);
};

#endif // wxUSE_COMBOBOX && defined(__WXUNIVERSAL__)

#endif // wxUSE_POPUPWIN

#endif // _WX_POPUPWIN_H_BASE_
