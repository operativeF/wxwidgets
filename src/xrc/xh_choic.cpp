/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_choic.cpp
// Purpose:     XRC resource for wxChoice
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_CHOICE

#include "wx/xrc/xh_choic.h"
#include "wx/intl.h"
#include "wx/choice.h"
#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxChoiceXmlHandler, wxXmlResourceHandler);

wxChoiceXmlHandler::wxChoiceXmlHandler()
  
{
    XRC_ADD_STYLE(wxCB_SORT);
    AddWindowStyles();
}

wxObject *wxChoiceXmlHandler::DoCreateResource()
{
    if( m_class == "wxChoice")
    {
        // find the selection
        long selection = GetLong("selection", -1);

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(nullptr, GetParamNode("content"));

        XRC_MAKE_INSTANCE(control, wxChoice)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetPosition(), GetSize(),
                        strList,
                        GetStyle(),
                        wxValidator{},
                        GetName());

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        strList.clear();    // dump the strings

        return control;
    }
    else
    {
        // on the inside now.
        // handle <item>Label</item>

        // add to the list
        strList.push_back(GetNodeText(m_node, wxXRC_TEXT_NO_ESCAPE));

        return nullptr;
    }
}

bool wxChoiceXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxChoice") ||
           (m_insideBox && node->GetName() == "item"));
}

#endif // wxUSE_XRC && wxUSE_CHOICE
