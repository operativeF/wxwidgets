/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/tglbtn.h
// Purpose:     Declaration of the wxToggleButton class, which implements a
//              toggle button under wxMac.
// Author:      Stefan Csomor
// Modified by:
// Created:     08.02.01
// Copyright:   (c) 2004 Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOGGLEBUTTON_H_
#define _WX_TOGGLEBUTTON_H_

class wxToggleButton : public wxToggleButtonBase
{
public:
    wxToggleButton() {}
    wxToggleButton(wxWindow *parent,
                   wxWindowID id,
                   const wxString& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxASCII_STR(wxToggleButtonNameStr))
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

	wxToggleButton(const wxToggleButton&) = delete;
	wxToggleButton& operator=(const wxToggleButton&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxToggleButtonNameStr));

    void SetValue(bool value) override;
    bool GetValue() const override;

    bool OSXHandleClicked( double timestampsec ) override;

    void Command(wxCommandEvent& event) override;

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};


class wxBitmapToggleButton : public wxToggleButton
{
public:
    wxBitmapToggleButton() {}
    wxBitmapToggleButton(wxWindow *parent,
                   wxWindowID id,
                   const wxBitmap& label,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxASCII_STR(wxToggleButtonNameStr))
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

	wxBitmapToggleButton(const wxBitmapToggleButton&) = delete;
	wxBitmapToggleButton& operator=(const wxBitmapToggleButton&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxBitmap& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxToggleButtonNameStr));

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_TOGGLEBUTTON_H_

