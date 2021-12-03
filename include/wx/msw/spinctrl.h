////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/spinctrl.h
// Purpose:     wxSpinCtrl class declaration for Win32
// Author:      Vadim Zeitlin
// Modified by:
// Created:     22.07.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_SPINCTRL_H_
#define _WX_MSW_SPINCTRL_H_

#if wxUSE_SPINCTRL

#include "wx/spinbutt.h"    // the base class

import Utils.Geometry;

import <limits>;
import <string>;
import <vector>;

class wxSpinCtrl;

using wxArraySpins = std::vector<wxSpinCtrl*>;

// ----------------------------------------------------------------------------
// Under Win32, wxSpinCtrl is a wxSpinButton with a buddy (as MSDN docs call
// it) text window whose contents is automatically updated when the spin
// control is clicked.
// ----------------------------------------------------------------------------

class wxSpinCtrl : public wxSpinButton
{
public:
    wxSpinCtrl() = default;

    wxSpinCtrl(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const std::string& value = {},
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = wxSP_ARROW_KEYS,
               int min = 0, int max = 100, int initial = 0,
               std::string_view name = "wxSpinCtrl")
    {
        Create(parent, id, value, pos, size, style, min, max, initial, name);
    }

    wxSpinCtrl& operator=(wxSpinCtrl&&) = delete;

    ~wxSpinCtrl();

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                std::string_view value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                int min = 0, int max = 100, int initial = 0,
                std::string_view name = "wxSpinCtrl");

    // a wxTextCtrl-like method (but we can't have GetValue returning std::string
    // because the base class already has one returning int!)
    void SetValue(std::string_view text);

    // another wxTextCtrl-like method
    void SetSelection(long from, long to);

    // wxSpinCtrlBase methods
    virtual std::string GetTextValue() const;
    virtual int GetBase() const;
    virtual bool SetBase(int base);

    void Refresh( bool eraseBackground = true,
                          const wxRect *rect = (const wxRect *) nullptr ) override;

    // implementation only from now on
    // -------------------------------

    void SetValue(int val) override;
    int  GetValue() const override;
    void SetRange(int minVal, int maxVal) override;
    bool SetFont(const wxFont &font) override;

    bool Enable(bool enable = true) override;
    bool Show(bool show = true) override;

    bool Reparent(wxWindowBase *newParent) override;

    // wxSpinButton doesn't accept focus, but we do
    bool AcceptsFocus() const override { return wxWindow::AcceptsFocus(); }

    // we're like wxTextCtrl and not (default) wxButton
    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal)
    {
        return GetCompositeControlsDefaultAttributes(variant);
    }

    // for internal use only

    // get the subclassed window proc of the buddy text
    WXWNDPROC GetBuddyWndProc() const { return m_wndProcBuddy; }

    // return the spinctrl object whose buddy is the given window or NULL
    static wxSpinCtrl *GetSpinForTextCtrl(WXHWND hwndBuddy);

    // process a WM_COMMAND generated by the buddy text control
    bool ProcessTextCommand(WXWORD cmd, WXWORD id);

    // recognize buddy window as part of this control at wx level
    bool ContainsHWND(WXHWND hWnd) const override { return hWnd == m_hwndBuddy; }

    void SetLayoutDirection(wxLayoutDirection dir) override;

    WXHWND MSWGetFocusHWND() const override;

protected:
    wxPoint DoGetPosition() const override;
    void DoMoveWindow(wxRect boundary) override;
    wxSize DoGetBestSize() const override;
    wxSize DoGetSizeFromTextSize(int xlen, int ylen = -1) const override;
    wxSize DoGetSize() const override;
    wxSize DoGetClientSize() const override;
    void DoClientToScreen(int *x, int *y) const override;
    void DoScreenToClient(int *x, int *y) const override;
#if wxUSE_TOOLTIPS
    void DoSetToolTip( wxToolTip *tip ) override;
#endif // wxUSE_TOOLTIPS

    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;
    bool MSWOnScroll(int orientation, WXWORD wParam,
                             WXWORD pos, WXHWND control) override;
    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    // handle processing of special keys
    void OnChar(wxKeyEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnKillFocus(wxFocusEvent& event);

    // generate spin control update event with the given value
    void SendSpinUpdate(int value);

    // called to ensure that the value is in the correct range
    void NormalizeValue() override;

private:
    // the data for the "buddy" text ctrl
    WXHWND     m_hwndBuddy{nullptr};
    WXWNDPROC  m_wndProcBuddy{nullptr};

    // the value of the control before the latest change (which might not have
    // changed anything in fact -- this is why we need this field)
    int m_oldValue{std::numeric_limits<int>::min()};
    
    // Block text update event after SetValue()
    bool m_blockEvent{false};

    // Adjust the text control style depending on whether we need to enter only
    // digits or may need to enter something else (e.g. "-" sign, "x"
    // hexadecimal prefix, ...) in it.
    void UpdateBuddyStyle();

    // Determine the (horizontal) pixel overlap between the spin button
    // (up-down control) and the text control (buddy window).
    int GetOverlap() const;

    // Calculate the best size for the number with the given number of digits.
    wxSize GetBestSizeFromDigitsCount(int digitsCount) const;

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_SPINCTRL

#endif // _WX_MSW_SPINCTRL_H_


