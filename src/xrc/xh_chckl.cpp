/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_chckl.cpp
// Purpose:     XRC resource for wxCheckListBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_CHECKLISTBOX

#include "wx/xrc/xh_chckl.h"

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/checklst.h"

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxCheckListBoxXmlHandler, wxXmlResourceHandler);

wxCheckListBoxXmlHandler::wxCheckListBoxXmlHandler()
  
{
    // wxListBox styles:
    XRC_ADD_STYLE(wxLB_SINGLE);
    XRC_ADD_STYLE(wxLB_MULTIPLE);
    XRC_ADD_STYLE(wxLB_EXTENDED);
    XRC_ADD_STYLE(wxLB_HSCROLL);
    XRC_ADD_STYLE(wxLB_ALWAYS_SB);
    XRC_ADD_STYLE(wxLB_NEEDED_SB);
    XRC_ADD_STYLE(wxLB_SORT);

    AddWindowStyles();
}

wxObject *wxCheckListBoxXmlHandler::DoCreateResource()
{
    if (m_class == "wxCheckListBox")
    {
        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(nullptr, GetParamNode("content"));

        XRC_MAKE_INSTANCE(control, wxCheckListBox)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetPosition(), GetSize(),
                        strList,
                        GetStyle(),
                        wxValidator{},
                        GetName());

        // step through children myself (again.)
        wxXmlNode *n = GetParamNode("content");
        if (n)
            n = n->GetChildren();
        int i = 0;
        while (n)
        {
            if (n->GetType() != wxXML_ELEMENT_NODE ||
                n->GetName() != "item")
               { n = n->GetNext(); continue; }

            // FIXME: bad.
            // checking boolean is a bit ugly here (see GetBool() )
            std::string v = n->GetAttribute("checked", "");
            wx::utils::ToLower(v);
            if (v == "1")
                control->Check( i, true );

            i++;
            n = n->GetNext();
        }

        SetupWindow(control);

        strList.clear();    // dump the strings

        return control;
    }
    else
    {
        // on the inside now.
        // handle <item checked="boolean">Label</item>

        // add to the list
        strList.push_back(GetNodeText(m_node, wxXRC_TEXT_NO_ESCAPE));
        return nullptr;
    }
}

bool wxCheckListBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxCheckListBox") ||
           (m_insideBox && node->GetName() == "item"));
}

#endif // wxUSE_XRC && wxUSE_CHECKLISTBOX
