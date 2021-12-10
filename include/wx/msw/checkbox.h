/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/checkbox.h
// Purpose:     wxCheckBox class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CHECKBOX_H_
#define _WX_CHECKBOX_H_

#include "wx/msw/ownerdrawnbutton.h"

import Utils.Geometry;
import WX.WinDef;

import <string>;
import <string_view>;

// Checkbox item (single checkbox)
class wxCheckBox : public wxMSWOwnerDrawnButton<wxCheckBoxBase>
{
public:
    wxCheckBox()  = default;
    wxCheckBox(wxWindow *parent,
               wxWindowID id,
               const std::string& label,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = 0,
               const wxValidator& validator = {},
               std::string_view name = wxCheckBoxNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxCheckBox& operator=(wxCheckBox&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxCheckBoxNameStr);

    void SetValue(bool value) override;
    bool GetValue() const override;

    // override some base class virtuals
    void SetLabel(std::string_view label) override;

    void SetTransparentPartColour(const wxColour& col) override
    {
        SetBackgroundColour(col);
    }

    bool MSWCommand(WXUINT param, WXWORD id) override;
    void Command(wxCommandEvent& event) override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

    // implementation only from now on
    WXDWORD MSWGetStyle(unsigned int flags, WXDWORD *exstyle = nullptr) const override;

protected:
    wxSize DoGetBestClientSize() const override;

    void DoSet3StateValue(wxCheckBoxState value) override;
    wxCheckBoxState DoGet3StateValue() const override;

    // Implement wxMSWOwnerDrawnButtonBase methods.
    int MSWGetButtonStyle() const override;
    void MSWOnButtonResetOwnerDrawn() override;
    unsigned int MSWGetButtonCheckedFlag() const override;
    void
        MSWDrawButtonBitmap(wxDC& dc, const wxRect& rect, unsigned int flags) override;

private:    
    // current state of the checkbox
    wxCheckBoxState m_state{wxCheckBoxState::Unchecked};
};

#endif // _WX_CHECKBOX_H_
