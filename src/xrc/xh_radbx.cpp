/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_radbx.cpp
// Purpose:     XRC resource for wxRadioBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_XRC && wxUSE_RADIOBOX

#include "wx/xrc/xh_radbx.h"
#include "wx/intl.h"
#include "wx/radiobox.h"
#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxRadioBoxXmlHandler, wxXmlResourceHandler);

wxRadioBoxXmlHandler::wxRadioBoxXmlHandler()
  
{
    XRC_ADD_STYLE(wxRA_SPECIFY_COLS);
    XRC_ADD_STYLE(wxRA_HORIZONTAL);
    XRC_ADD_STYLE(wxRA_SPECIFY_ROWS);
    XRC_ADD_STYLE(wxRA_VERTICAL);
    AddWindowStyles();
}

wxObject *wxRadioBoxXmlHandler::DoCreateResource()
{
    if ( m_class == "wxRadioBox")
    {
        // find the selection
        long selection = GetLong( "selection", -1 );

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately( nullptr, GetParamNode("content"));

        XRC_MAKE_INSTANCE(control, wxRadioBox)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetText("label"),
                        GetPosition(), GetSize(),
                        m_labels,
                        GetLong("dimension", 1),
                        GetStyle(),
                        wxValidator{},
                        GetName());

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        const unsigned count = m_labels.size();
        for( unsigned i = 0; i < count; i++ )
        {
#if wxUSE_TOOLTIPS
            if ( !m_tooltips[i].empty() )
                control->SetItemToolTip(i, m_tooltips[i]);
#endif // wxUSE_TOOLTIPS
#if wxUSE_HELP
            if ( m_helptextSpecified[i] )
                control->SetItemHelpText(i, m_helptexts[i]);
#endif // wxUSE_HELP

            if ( !m_isShown[i] )
                control->Show(i, false);
            if ( !m_isEnabled[i] )
                control->Enable(i, false);
        }


        // forget information about the items of this radiobox, we should start
        // afresh for the next one
        m_labels.clear();

#if wxUSE_TOOLTIPS
        m_tooltips.clear();
#endif // wxUSE_TOOLTIPS

#if wxUSE_HELP
        m_helptexts.clear();
        m_helptextSpecified.clear();
#endif // wxUSE_HELP

        m_isShown.clear();
        m_isEnabled.clear();

        return control;
    }
    else // inside the radiobox element
    {
        // we handle handle <item>Label</item> constructs here, and the item
        // tag can have tooltip, helptext, enabled and hidden attributes

        // For compatibility, labels are not escaped in XRC by default and
        // label="1" attribute needs to be explicitly specified to handle them
        // consistently with the other labels.
        m_labels.push_back(GetNodeText(m_node,
                                       GetBoolAttr("label", false)
                                        ? 0
                                        : wxXRC_TEXT_NO_ESCAPE));
#if wxUSE_TOOLTIPS
        m_tooltips.push_back(GetNodeText(GetParamNode("tooltip"),
                                         wxXRC_TEXT_NO_ESCAPE));
#endif // wxUSE_TOOLTIPS
#if wxUSE_HELP
        const wxXmlNode* const nodeHelp = GetParamNode("helptext");
        m_helptexts.push_back(GetNodeText(nodeHelp, wxXRC_TEXT_NO_ESCAPE));
        m_helptextSpecified.push_back(nodeHelp != nullptr);
#endif // wxUSE_HELP
        m_isEnabled.push_back(GetBoolAttr("enabled", true));
        m_isShown.push_back(!GetBoolAttr("hidden", false));

        return nullptr;
    }

}

bool wxRadioBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxRadioBox") ||
           (m_insideBox && node->GetName() == "item"));
}

#endif // wxUSE_XRC && wxUSE_RADIOBOX
