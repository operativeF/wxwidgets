/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_chckb.cpp
// Purpose:     XRC resource for wxCheckBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_CHECKBOX

#include "wx/xrc/xh_chckb.h"
#include "wx/checkbox.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxCheckBoxXmlHandler, wxXmlResourceHandler);

wxCheckBoxXmlHandler::wxCheckBoxXmlHandler()
 
{
    XRC_ADD_STYLE(wxCHK_2STATE);
    XRC_ADD_STYLE(wxCHK_3STATE);
    XRC_ADD_STYLE(wxCHK_ALLOW_3RD_STATE_FOR_USER);
    XRC_ADD_STYLE(wxALIGN_RIGHT);
    AddWindowStyles();
}

wxObject *wxCheckBoxXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxCheckBox)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText("label"),
                    GetPosition(), GetSize(),
                    GetStyle(),
                    wxDefaultValidator,
                    GetName());

    control->SetValue(GetBool( "checked"));
    SetupWindow(control);

    return control;
}

bool wxCheckBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxCheckBox");
}

#endif // wxUSE_XRC && wxUSE_CHECKBOX
