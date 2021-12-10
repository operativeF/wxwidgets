/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/radiobox.h
// Purpose:     wxRadioBox class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RADIOBOX_H_
#define _WX_RADIOBOX_H_

#include "wx/statbox.h"

import WX.Cfg.Flags;

import Utils.Geometry;

import <string>;
import <vector>;

class wxSubwindows;

// ----------------------------------------------------------------------------
// wxRadioBox
// ----------------------------------------------------------------------------

class wxRadioBox : public wxStaticBox, public wxRadioBoxBase
{
public:
    wxRadioBox() = default;

    wxRadioBox(wxWindow *parent,
               wxWindowID id,
               const std::string& title,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               const std::vector<std::string>& choices = {},
               int majorDim = 0,
               unsigned int style = wxRA_SPECIFY_COLS,
               const wxValidator& val = {},
               std::string_view name = wxRadioBoxNameStr)
    {
        Create(parent, id, title, pos, size, choices, majorDim,
                     style, val, name);
    }

    ~wxRadioBox();

    wxRadioBox& operator=(wxRadioBox&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& title,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<std::string>& choices,
                int majorDim = 0,
                unsigned int style = wxRA_SPECIFY_COLS,
                const wxValidator& val = {},
                std::string_view name = wxRadioBoxNameStr);

    // implement the radiobox interface
    void SetSelection(int n) override;
    int GetSelection() const override { return m_selectedButton; }
    size_t GetCount() const override;
    std::string GetString(unsigned int n) const override;
    void SetString(unsigned int n, const std::string& label) override;
    bool Enable(unsigned int n, bool enable = true) override;
    bool Show(unsigned int n, bool show = true) override;
    bool IsItemEnabled(unsigned int n) const override;
    bool IsItemShown(unsigned int n) const override;
    int GetItemFromPoint(const wxPoint& pt) const override;

    // override some base class methods
    bool Show(bool show = true) override;
    bool Enable(bool enable = true) override;
    bool CanBeFocused() const override;
    void SetFocus() override;
    bool SetFont(const wxFont& font) override;
    bool ContainsHWND(WXHWND hWnd) const override;
    bool SetForegroundColour(const wxColour& colour) override;
    bool SetBackgroundColour(const wxColour& colour) override;
#if wxUSE_TOOLTIPS
    bool HasToolTips() const override;
#endif // wxUSE_TOOLTIPS
#if wxUSE_HELP
    // override virtual function with a platform-independent implementation
    std::string GetHelpTextAtPoint(const wxPoint & pt, wxHelpEvent::Origin origin) const override
    {
        return wxRadioBoxBase::DoGetHelpTextAtPoint( this, pt, origin );
    }
#endif // wxUSE_HELP

    bool Reparent(wxWindowBase *newParent) override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

    void SetLabelFont([[maybe_unused]] const wxFont& font) {}
    void SetButtonFont(const wxFont& font) { SetFont(font); }


    // implementation only from now on
    // -------------------------------

    // This function can be used to check if the given radio button WXHWND
    // belongs to one of our radio boxes. If it doesn't, NULL is returned.
    static wxRadioBox *GetFromRadioButtonHWND(WXHWND hwnd);

    bool MSWCommand(WXUINT param, WXWORD id) override;
    void Command(wxCommandEvent& event) override;

    void SendNotificationEvent();

protected:
    // subclass one radio button
    void SubclassRadioButton(WXHWND hWndBtn);

    // get the max size of radio buttons
    wxSize GetMaxButtonSize() const;

    // get the total size occupied by the radio box buttons
    wxSize GetTotalButtonSize(const wxSize& sizeBtn) const;

    // Adjust all the buttons to the new window size.
    void PositionAllButtons(wxRect boundary);

    void DoSetSize(wxRect boundary, unsigned int sizeFlags = wxSIZE_AUTO) override;
    void DoMoveWindow(wxRect boundary) override;
    wxSize DoGetBestSize() const override;

#if wxUSE_TOOLTIPS
    void DoSetItemToolTip(unsigned int n, wxToolTip * tooltip) override;
#endif

    WXHRGN MSWGetRegionWithoutChildren() override;

    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    // resolve ambiguity in base classes
    wxBorder GetDefaultBorder() const override { return wxRadioBoxBase::GetDefaultBorder(); }

private:
    // the buttons we contain
    wxSubwindows *m_radioButtons {nullptr};

    // and the special dummy button used only as a tab group boundary
    WXHWND m_dummyHwnd {nullptr};
    wxWindowIDRef m_dummyId;

    // currently selected button or wxNOT_FOUND if none
    int m_selectedButton {wxNOT_FOUND};
};

#endif
    // _WX_RADIOBOX_H_
