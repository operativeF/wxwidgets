///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/tglbtn.h
// Purpose:     wxToggleButton for wxUniversal
// Author:      Vadim Zeitlin
// Modified by: David Bjorkevik
// Created:     16.05.06
// Copyright:   (c) 2000 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_TGLBTN_H_
#define _WX_UNIV_TGLBTN_H_

// ----------------------------------------------------------------------------
// wxToggleButton: a push button
// ----------------------------------------------------------------------------

class wxToggleButton: public wxToggleButtonBase
{
public:
    wxToggleButton();

    wxToggleButton(wxWindow *parent,
             wxWindowID id,
             const wxString& label = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             unsigned int style = 0,
             const wxValidator& validator = wxDefaultValidator,
             const wxString& name = wxASCII_STR(wxToggleButtonNameStr));

    // Create the control
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& lbl = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxToggleButtonNameStr));

    bool IsPressed() const override { return m_isPressed || m_value; }

    // wxToggleButton actions
    void Toggle() override;
    void Click() override;

    // Get/set the value
    void SetValue(bool state);
    bool GetValue() const { return m_value; }

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // the current value
    bool m_value;

private:
    // common part of all ctors
    void Init();

    wxDECLARE_DYNAMIC_CLASS(wxToggleButton);
};

#endif // _WX_UNIV_TGLBTN_H_
