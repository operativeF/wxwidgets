/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_radbt.cpp
// Purpose:     XRC resource for wxRadioButton
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_RADIOBTN

#include "wx/xrc/xh_radbt.h"
#include "wx/radiobut.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxRadioButtonXmlHandler, wxXmlResourceHandler);

wxRadioButtonXmlHandler::wxRadioButtonXmlHandler()
 
{
    XRC_ADD_STYLE(wxRB_GROUP);
    XRC_ADD_STYLE(wxRB_SINGLE);
    AddWindowStyles();
}

wxObject *wxRadioButtonXmlHandler::DoCreateResource()
{
    /* BOBM - implementation note.
     * once the wxBitmapRadioButton is implemented.
     * look for a bitmap property. If not null,
     * make it a wxBitmapRadioButton instead of the
     * normal radio button.
     */

    XRC_MAKE_INSTANCE(control, wxRadioButton)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText("label"),
                    GetPosition(), GetSize(),
                    GetStyle(),
                    wxDefaultValidator,
                    GetName());

    control->SetValue(GetBool("value", false));
    SetupWindow(control);

    return control;
}

bool wxRadioButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxRadioButton");
}

#endif // wxUSE_XRC && wxUSE_RADIOBTN
