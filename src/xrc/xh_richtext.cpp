/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_richtext.cpp
// Purpose:     XRC resource for wxRichTextCtrl
// Author:      Julian Smart
// Created:     2006-11-08
// Copyright:   (c) 2006 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_XRC && wxUSE_RICHTEXT

#include "wx/xrc/xh_richtext.h"
#include "wx/richtext/richtextctrl.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextCtrlXmlHandler, wxXmlResourceHandler);

wxRichTextCtrlXmlHandler::wxRichTextCtrlXmlHandler()  
{
    XRC_ADD_STYLE(wxTE_PROCESS_ENTER);
    XRC_ADD_STYLE(wxTE_PROCESS_TAB);
    XRC_ADD_STYLE(wxTE_MULTILINE);
    XRC_ADD_STYLE(wxTE_READONLY);
    XRC_ADD_STYLE(wxTE_AUTO_URL);

    AddWindowStyles();
}

wxObject *wxRichTextCtrlXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(text, wxRichTextCtrl)

    text->Create(m_parentAsWindow,
                 GetID(),
                 GetText("value"),
                 GetPosition(), GetSize(),
                 GetStyle(),
                 wxValidator{},
                 GetName());

    SetupWindow(text);

    if (HasParam("maxlength"))
        text->SetMaxLength(GetLong("maxlength"));

    return text;
}

bool wxRichTextCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxRichTextCtrl");
}

#endif // wxUSE_XRC && wxUSE_RICHTEXT

