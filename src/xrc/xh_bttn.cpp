/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_bttn.cpp
// Purpose:     XRC resource for buttons
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_BUTTON

#include "wx/xrc/xh_bttn.h"
#include "wx/button.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxButtonXmlHandler, wxXmlResourceHandler);

wxButtonXmlHandler::wxButtonXmlHandler()
 
{
    XRC_ADD_STYLE(wxBU_LEFT);
    XRC_ADD_STYLE(wxBU_RIGHT);
    XRC_ADD_STYLE(wxBU_TOP);
    XRC_ADD_STYLE(wxBU_BOTTOM);
    XRC_ADD_STYLE(wxBU_EXACTFIT);
    AddWindowStyles();
}

wxObject *wxButtonXmlHandler::DoCreateResource()
{
   XRC_MAKE_INSTANCE(button, wxButton)

   button->Create(m_parentAsWindow,
                    GetID(),
                    GetText("label"),
                    GetPosition(), GetSize(),
                    GetStyle(),
                    wxDefaultValidator,
                    GetName());

    if (GetBool("default", false))
        button->SetDefault();

    if ( GetParamNode("bitmap") )
    {
        button->SetBitmap(GetBitmap("bitmap", wxART_BUTTON),
                          GetDirection("bitmapposition"));
    }

    SetupWindow(button);

    return button;
}

bool wxButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxButton");
}

#endif // wxUSE_XRC && wxUSE_BUTTON
