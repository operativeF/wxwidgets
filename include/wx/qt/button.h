/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/button.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_BUTTON_H_
#define _WX_QT_BUTTON_H_

#include "wx/control.h"
#include "wx/button.h"

class wxButton : public wxButtonBase
{
public:
    wxButton();
    wxButton(wxWindow *parent, wxWindowID id,
           const wxString& label = wxEmptyString,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, unsigned int style = 0,
           const wxValidator& validator = wxDefaultValidator,
           const wxString& name = wxASCII_STR(wxButtonNameStr));

    wxButton(const wxButton&) = delete;
    wxButton& operator=(const wxButton&) = delete;

    bool Create(wxWindow *parent, wxWindowID id,
           const wxString& label = wxEmptyString,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, unsigned int style = 0,
           const wxValidator& validator = wxDefaultValidator,
           const wxString& name = wxASCII_STR(wxButtonNameStr));

   wxWindow *SetDefault() override;

    // implementation only
    int QtGetEventType() const override { return wxEVT_BUTTON; }

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};


#endif // _WX_QT_BUTTON_H_
