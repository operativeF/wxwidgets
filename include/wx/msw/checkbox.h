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

// Checkbox item (single checkbox)
class WXDLLIMPEXP_CORE wxCheckBox : public wxMSWOwnerDrawnButton<wxCheckBoxBase>
{
public:
    wxCheckBox()  = default;
    wxCheckBox(wxWindow *parent,
               wxWindowID id,
               const wxString& label,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxASCII_STR(wxCheckBoxNameStr))
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

wxCheckBox(const wxCheckBox&) = delete;
   wxCheckBox& operator=(const wxCheckBox&) = delete;
   wxCheckBox(wxCheckBox&&) = default;
   wxCheckBox& operator=(wxCheckBox&&) = default;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxCheckBoxNameStr));

    void SetValue(bool value) override;
    bool GetValue() const override;

    // override some base class virtuals
    void SetLabel(const wxString& label) override;

    void SetTransparentPartColour(const wxColour& col) override
    {
        SetBackgroundColour(col);
    }

    bool MSWCommand(WXUINT param, WXWORD id) override;
    void Command(wxCommandEvent& event) override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

    // implementation only from now on
    WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle = nullptr) const override;

protected:
    wxSize DoGetBestClientSize() const override;

    void DoSet3StateValue(wxCheckBoxState value) override;
    wxCheckBoxState DoGet3StateValue() const override;

    // Implement wxMSWOwnerDrawnButtonBase methods.
    int MSWGetButtonStyle() const override;
    void MSWOnButtonResetOwnerDrawn() override;
    int MSWGetButtonCheckedFlag() const override;
    void
        MSWDrawButtonBitmap(wxDC& dc, const wxRect& rect, int flags) override;

private:    
    // current state of the checkbox
    wxCheckBoxState m_state{wxCHK_UNCHECKED};

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_CHECKBOX_H_
