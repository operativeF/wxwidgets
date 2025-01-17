/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_sttxt.cpp
// Purpose:     XRC resource for wxStaticText
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_STATTEXT

#include "wx/xrc/xh_sttxt.h"
#include "wx/stattext.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxStaticTextXmlHandler, wxXmlResourceHandler);

wxStaticTextXmlHandler::wxStaticTextXmlHandler()
 
{
    XRC_ADD_STYLE(wxST_NO_AUTORESIZE);
    XRC_ADD_STYLE(wxALIGN_LEFT);
    XRC_ADD_STYLE(wxALIGN_RIGHT);
    XRC_ADD_STYLE(wxALIGN_CENTER);
    XRC_ADD_STYLE(wxALIGN_CENTRE);
    XRC_ADD_STYLE(wxALIGN_CENTER_HORIZONTAL);
    XRC_ADD_STYLE(wxALIGN_CENTRE_HORIZONTAL);
    XRC_ADD_STYLE(wxST_ELLIPSIZE_START);
    XRC_ADD_STYLE(wxST_ELLIPSIZE_MIDDLE);
    XRC_ADD_STYLE(wxST_ELLIPSIZE_END);
    AddWindowStyles();
}

wxObject *wxStaticTextXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(text, wxStaticText)

    text->Create(m_parentAsWindow,
                 GetID(),
                 GetText("label"),
                 GetPosition(), GetSize(),
                 GetStyle(),
                 GetName());

    SetupWindow(text);

    long wrap = GetDimension("wrap", -1);
    if (wrap != -1)
        text->Wrap(wrap);

    return text;
}

bool wxStaticTextXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxStaticText");
}

#endif // wxUSE_XRC && wxUSE_STATTEXT
