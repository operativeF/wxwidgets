/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_combo.cpp
// Purpose:     XRC resource for wxComboBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_COMBOBOX

#include "wx/xrc/xh_combo.h"

#include "wx/intl.h"
#include "wx/combobox.h"
#include "wx/textctrl.h"    // for wxTE_PROCESS_ENTER

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxComboBoxXmlHandler, wxXmlResourceHandler);

wxComboBoxXmlHandler::wxComboBoxXmlHandler()
                     
                     
{
    XRC_ADD_STYLE(wxCB_SIMPLE);
    XRC_ADD_STYLE(wxCB_SORT);
    XRC_ADD_STYLE(wxCB_READONLY);
    XRC_ADD_STYLE(wxCB_DROPDOWN);
    XRC_ADD_STYLE(wxTE_PROCESS_ENTER);
    AddWindowStyles();
}

wxObject *wxComboBoxXmlHandler::DoCreateResource()
{
    if( m_class == "wxComboBox")
    {
        // find the selection
        long selection = GetLong( "selection", -1 );

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(nullptr, GetParamNode("content"));

        XRC_MAKE_INSTANCE(control, wxComboBox)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetText("value"),
                        GetPosition(), GetSize(),
                        strList,
                        GetStyle(),
                        wxValidator{},
                        GetName());

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        const wxString hint = GetText("hint");
        if ( !hint.empty() )
            control->SetHint(hint);

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

bool wxComboBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxComboBox") ||
           (m_insideBox && node->GetName() == "item"));
}

#endif // wxUSE_XRC && wxUSE_COMBOBOX
