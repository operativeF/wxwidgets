/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_bmpbt.cpp
// Purpose:     XRC resource for bitmap buttons
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_BMPBUTTON

#include "wx/xrc/xh_bmpbt.h"
#include "wx/bmpbuttn.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapButtonXmlHandler, wxXmlResourceHandler);

wxBitmapButtonXmlHandler::wxBitmapButtonXmlHandler()
 
{
    XRC_ADD_STYLE(wxBU_AUTODRAW);
    XRC_ADD_STYLE(wxBU_LEFT);
    XRC_ADD_STYLE(wxBU_RIGHT);
    XRC_ADD_STYLE(wxBU_TOP);
    XRC_ADD_STYLE(wxBU_BOTTOM);
    XRC_ADD_STYLE(wxBU_EXACTFIT);
    AddWindowStyles();
}

wxObject *wxBitmapButtonXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(button, wxBitmapButton)

    if ( GetBool("close", false) )
    {
        button->CreateCloseButton(m_parentAsWindow,
                                  GetID(),
                                  GetName());
    }
    else
    {
        button->Create(m_parentAsWindow,
                       GetID(),
                       GetBitmap("bitmap", wxART_BUTTON),
                       GetPosition(), GetSize(),
                       GetStyle("style"),
                       wxDefaultValidator,
                       GetName());
    }

    if (GetBool("default", false))
        button->SetDefault();
    SetupWindow(button);

    if (GetParamNode("selected"))
        button->SetBitmapSelected(GetBitmap("selected"));
    if (GetParamNode("focus"))
        button->SetBitmapFocus(GetBitmap("focus"));
    if (GetParamNode("disabled"))
        button->SetBitmapDisabled(GetBitmap("disabled"));
    if (GetParamNode("hover"))
        button->SetBitmapHover(GetBitmap("hover"));

    return button;
}

bool wxBitmapButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxBitmapButton");
}

#endif // wxUSE_XRC && wxUSE_BMPBUTTON
