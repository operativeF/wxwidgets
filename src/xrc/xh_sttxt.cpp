/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_sttxt.cpp
// Purpose:     XRC resource for wxStaticText
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_STATTEXT

#include "wx/xrc/xh_sttxt.h"
#include "wx/stattext.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxStaticTextXmlHandler, wxXmlResourceHandler);

wxStaticTextXmlHandler::wxStaticTextXmlHandler()
 
{
    // FIXME: Flags not compatible in this construct.
    XRC_ADD_STYLE(wxST_NO_AUTORESIZE);
    //XRC_ADD_STYLE(wxAlignment::Left);
    //XRC_ADD_STYLE(wxAlignment::Right);
    //XRC_ADD_STYLE(wxAlignment::Center);
    //XRC_ADD_STYLE(wxAlignment::Center);
    //XRC_ADD_STYLE(wxAlignment::CenterHorizontal);
    //XRC_ADD_STYLE(wxAlignment::CenterHorizontal);
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
                 GetText(wxT("label")),
                 GetPosition(), GetSize(),
                 GetStyle(),
                 GetName());

    SetupWindow(text);

    long wrap = GetDimension(wxT("wrap"), -1);
    if (wrap != -1)
        text->Wrap(wrap);

    return text;
}

bool wxStaticTextXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxStaticText"));
}

#endif // wxUSE_XRC && wxUSE_STATTEXT
