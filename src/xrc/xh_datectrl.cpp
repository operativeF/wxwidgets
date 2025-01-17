/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_datectrl.cpp
// Purpose:     XML resource handler for wxDatePickerCtrl
// Author:      Vaclav Slavik
// Created:     2005-02-07
// Copyright:   (c) 2005 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_DATEPICKCTRL

#include "wx/xrc/xh_datectrl.h"
#include "wx/datectrl.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxDateCtrlXmlHandler, wxXmlResourceHandler);

wxDateCtrlXmlHandler::wxDateCtrlXmlHandler()  
{
    XRC_ADD_STYLE(wxDP_DEFAULT);
    XRC_ADD_STYLE(wxDP_SPIN);
    XRC_ADD_STYLE(wxDP_DROPDOWN);
    XRC_ADD_STYLE(wxDP_ALLOWNONE);
    XRC_ADD_STYLE(wxDP_SHOWCENTURY);
    AddWindowStyles();
}

wxObject *wxDateCtrlXmlHandler::DoCreateResource()
{
   XRC_MAKE_INSTANCE(picker, wxDatePickerCtrl)

   picker->Create(m_parentAsWindow,
                  GetID(),
                  wxDefaultDateTime,
                  GetPosition(), GetSize(),
                  GetStyle("style", wxDP_DEFAULT | wxDP_SHOWCENTURY),
                  wxValidator{},
                  GetName());

    SetupWindow(picker);

    // Note that we want to set this one even if it's empty.
    if ( HasParam("null-text") )
        picker->SetNullText(GetText("null-text"));

    return picker;
}

bool wxDateCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxDatePickerCtrl");
}

#endif // wxUSE_XRC && wxUSE_DATEPICKCTRL
