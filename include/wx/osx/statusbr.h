///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/statusbr.h
// Purpose:     native implementation of wxStatusBar.
//              Optional: can use generic version instead.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STATBAR_H_
#define _WX_STATBAR_H_

class wxStatusBarMac : public wxStatusBarGeneric
{
public:
    wxStatusBarMac();
    wxStatusBarMac(wxWindow *parent, wxWindowID id = wxID_ANY,
           long style = wxSTB_DEFAULT_STYLE,
           const wxString& name = wxASCII_STR(wxStatusBarNameStr));

    virtual ~wxStatusBarMac();

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
              long style = wxSTB_DEFAULT_STYLE,
              const wxString& name = wxASCII_STR(wxStatusBarNameStr));

    // Implementation
    void MacHiliteChanged() override;
    void OnPaint(wxPaintEvent& event);

protected:
    int GetEffectiveFieldStyle(int WXUNUSED(i)) const override { return wxSB_NORMAL; }

    void InitColours() override;

private:
    wxColour m_textActive, m_textInactive;

    wxDECLARE_DYNAMIC_CLASS(wxStatusBarMac);
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_STATBAR_H_
