/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_bmpcbox.cpp
// Purpose:     XRC resource for wxBitmapComboBox
// Author:      Jaakko Salli
// Created:     Sep-10-2006
// Copyright:   (c) 2006 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_BITMAPCOMBOBOX

#include "wx/xrc/xh_bmpcbox.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#include "wx/bmpcbox.h"

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapComboBoxXmlHandler, wxXmlResourceHandler);

wxBitmapComboBoxXmlHandler::wxBitmapComboBoxXmlHandler()
                     
{
    XRC_ADD_STYLE(wxCB_SORT);
    XRC_ADD_STYLE(wxCB_READONLY);
    AddWindowStyles();
}

wxObject *wxBitmapComboBoxXmlHandler::DoCreateResource()
{
    if (m_class == "ownerdrawnitem")
    {
        if ( !m_combobox )
        {
            ReportError("ownerdrawnitem only allowed within a wxBitmapComboBox");
            return nullptr;
        }

        m_combobox->Append(GetText("text"),
                           GetBitmap("bitmap"));

        return m_combobox;
    }
    else /*if( m_class == "wxBitmapComboBox")*/
    {
        // find the selection
        long selection = GetLong( "selection", -1 );

        XRC_MAKE_INSTANCE(control, wxBitmapComboBox)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetText("value"),
                        GetPosition(), GetSize(),
                        {},
                        GetStyle(),
                        wxValidator{},
                        GetName());

        m_isInside = true;
        m_combobox = control;

        wxXmlNode *children_node = GetParamNode("object");

        wxXmlNode *n = children_node;

        while (n)
        {
            if ((n->GetType() == wxXML_ELEMENT_NODE) &&
                (n->GetName() == "object"))
            {
                CreateResFromNode(n, control, nullptr);
            }
            n = n->GetNext();
        }

        m_isInside = false;
        m_combobox = nullptr;

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        return control;
    }
}

bool wxBitmapComboBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxBitmapComboBox")) ||
            (m_isInside && IsOfClass(node, "ownerdrawnitem")));
}

#endif // wxUSE_XRC && wxUSE_BITMAPCOMBOBOX
