/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_listb.cpp
// Purpose:     XRC resource for wxListBox
// Author:      Bob Mitchell & Vaclav Slavik
// Created:     2000/07/29
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_LISTBOX

#include "wx/xrc/xh_listb.h"
#include "wx/intl.h"
#include "wx/listbox.h"
#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxListBoxXmlHandler, wxXmlResourceHandler);

wxListBoxXmlHandler::wxListBoxXmlHandler()
                    
                     
{
    XRC_ADD_STYLE(wxLB_SINGLE);
    XRC_ADD_STYLE(wxLB_MULTIPLE);
    XRC_ADD_STYLE(wxLB_EXTENDED);
    XRC_ADD_STYLE(wxLB_HSCROLL);
    XRC_ADD_STYLE(wxLB_ALWAYS_SB);
    XRC_ADD_STYLE(wxLB_NEEDED_SB);
    XRC_ADD_STYLE(wxLB_SORT);
    AddWindowStyles();
}

wxObject *wxListBoxXmlHandler::DoCreateResource()
{
    if ( m_class == "wxListBox")
    {
        // find the selection
        long selection = GetLong("selection", -1);

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(nullptr, GetParamNode("content"));
        m_insideBox = false;

        XRC_MAKE_INSTANCE(control, wxListBox)

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

bool wxListBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxListBox") ||
           (m_insideBox && node->GetName() == "item"));
}

#endif // wxUSE_XRC && wxUSE_LISTBOX
