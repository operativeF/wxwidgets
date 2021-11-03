/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/control.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_CONTROL_H_
#define _WX_QT_CONTROL_H_

class wxControl : public wxControlBase
{
public:
    wxControl();
    wxControl(wxWindow *parent, wxWindowID id,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize, unsigned int style = 0,
             const wxValidator& validator = wxDefaultValidator,
             const wxString& name = wxASCII_STR(wxControlNameStr));

    bool Create(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, unsigned int style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxControlNameStr));

    wxSize DoGetBestSize() const override;

protected:
    bool QtCreateControl( wxWindow *parent, wxWindowID id, const wxPoint &pos,
        const wxSize &size, unsigned int style, const wxValidator &validator,
        const wxString &name );

private:
    wxDECLARE_DYNAMIC_CLASS(wxControl);
};

#endif // _WX_QT_CONTROL_H_
