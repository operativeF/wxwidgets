/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/radiobut.h
// Purpose:     wxRadioButton class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RADIOBUT_H_
#define _WX_RADIOBUT_H_

#include "wx/msw/ownerdrawnbutton.h"

import WX.Cfg.Flags;
import WX.WinDef;

import Utils.Geometry;

import <string>;


class wxRadioButton : public wxMSWOwnerDrawnButton<wxRadioButtonBase>
{
public:
    // ctors and creation functions
    wxRadioButton() = default;

    wxRadioButton(wxWindow *parent,
                  wxWindowID id,
                  const std::string& label,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  unsigned int style = 0,
                  const wxValidator& validator = {},
                  std::string_view name = wxRadioButtonNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxRadioButton& operator=(wxRadioButton&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const std::string& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxRadioButtonNameStr);

    // implement the radio button interface
    void SetValue(bool value) override;
    bool GetValue() const override;

    // implementation only from now on
    bool MSWCommand(WXUINT param, WXWORD id) override;
    void Command(wxCommandEvent& event) override;

    bool HasTransparentBackground() override { return true; }

    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }
    wxSize DoGetBestSize() const override;

    // Implement wxMSWOwnerDrawnButtonBase methods.
    int MSWGetButtonStyle() const override;
    void MSWOnButtonResetOwnerDrawn() override;
    unsigned int MSWGetButtonCheckedFlag() const override;
    void MSWDrawButtonBitmap(wxDC& dc, const wxRect& rect, unsigned int flags) override;


private:    
    // we need to store the state internally as the result of GetValue()
    // sometimes gets out of sync in WM_COMMAND handler
    bool m_isChecked{false};
};

#endif // _WX_RADIOBUT_H_
