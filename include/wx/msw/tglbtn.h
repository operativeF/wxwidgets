/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/tglbtn.h
// Purpose:     Declaration of the wxToggleButton class, which implements a
//              toggle button under wxMSW.
// Author:      John Norris, minor changes by Axel Schlueter
// Modified by:
// Created:     08.02.01
// Copyright:   (c) 2000 Johnny C. Norris II
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOGGLEBUTTON_H_
#define _WX_TOGGLEBUTTON_H_

import WX.Cfg.Flags;
import WX.WinDef;

import <string>;

class wxBitmap;

// Checkbox item (single checkbox)
class wxToggleButton : public wxToggleButtonBase
{
public:
    wxToggleButton() = default;
    wxToggleButton(wxWindow *parent,
                   wxWindowID id,
                   const std::string& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   unsigned int style = 0,
                   const wxValidator& validator = {},
                   std::string_view name = wxToggleButtonNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxToggleButton& operator=(wxToggleButton&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxToggleButtonNameStr);

    void SetValue(bool value) override;
    bool GetValue() const override;

    bool MSWCommand(WXUINT param, WXWORD id) override;
    void Command(wxCommandEvent& event) override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    WXDWORD MSWGetStyle(unsigned int flags, WXDWORD *exstyle = nullptr) const override;

    bool MSWIsPushed() const override;

private:
    // current state of the button (when owner-drawn)
    bool m_state{false};
};

//-----------------------------------------------------------------------------
// wxBitmapToggleButton
//-----------------------------------------------------------------------------


class wxBitmapToggleButton: public wxToggleButton
{
public:
    // construction/destruction
    wxBitmapToggleButton() = default;
    wxBitmapToggleButton(wxWindow *parent,
                   wxWindowID id,
                   const wxBitmap& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   unsigned int style = 0,
                   const wxValidator& validator = {},
                   std::string_view name = wxToggleButtonNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    // Create the control
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxBitmap& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxToggleButtonNameStr);

    // prevent virtual function hiding
    void SetLabel(std::string_view label) override { wxToggleButton::SetLabel(label); }
};

#endif // _WX_TOGGLEBUTTON_H_

