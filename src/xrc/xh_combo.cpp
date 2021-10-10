/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_combo.cpp
// Purpose:     XRC resource for wxComboBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_COMBOBOX

#include "wx/xrc/xh_combo.h"

#include "wx/intl.h"
#include "wx/combobox.h"
#include "wx/textctrl.h"    // for wxTE_PROCESS_ENTER

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxComboBoxXmlHandler, wxXmlResourceHandler);

wxComboBoxXmlHandler::wxComboBoxXmlHandler()
                     
                     
{
    XRC_ADD_STYLE(ComboStyles::Simple);
    XRC_ADD_STYLE(ComboStyles::Sort);
    XRC_ADD_STYLE(ComboStyles::ReadOnly);
    XRC_ADD_STYLE(ComboStyles::DropDown);
    XRC_ADD_STYLE(wxTE_PROCESS_ENTER);
    AddWindowStyles();
}

wxObject *wxComboBoxXmlHandler::DoCreateResource()
{
    if( m_class == wxT("wxComboBox"))
    {
        // find the selection
        long selection = GetLong( wxT("selection"), -1 );

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(nullptr, GetParamNode(wxT("content")));

        XRC_MAKE_INSTANCE(control, wxComboBox)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetText(wxT("value")),
                        GetPosition(), GetSize(),
                        strList,
                        GetStyle(),
                        wxDefaultValidator,
                        GetName());

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        const wxString hint = GetText(wxS("hint"));
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
    return (IsOfClass(node, wxT("wxComboBox")) ||
           (m_insideBox && node->GetName() == wxT("item")));
}

#endif // wxUSE_XRC && wxUSE_COMBOBOX
