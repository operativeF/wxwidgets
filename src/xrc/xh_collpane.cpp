/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_collpane.cpp
// Purpose:     XML resource handler for wxCollapsiblePane
// Author:      Francesco Montorsi
// Created:     2006-10-27
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_COLLPANE

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/collpane.h"
#include "wx/xrc/xh_collpane.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxCollapsiblePaneXmlHandler, wxXmlResourceHandler);

wxCollapsiblePaneXmlHandler::wxCollapsiblePaneXmlHandler()
  
{
    XRC_ADD_STYLE(wxCP_NO_TLW_RESIZE);
    XRC_ADD_STYLE(wxCP_DEFAULT_STYLE);
    AddWindowStyles();
}

wxObject *wxCollapsiblePaneXmlHandler::DoCreateResource()
{
    if (m_class == "panewindow")   // read the XRC for the pane window
    {
        wxXmlNode *n = GetParamNode("object");

        if ( !n )
            n = GetParamNode("object_ref");

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_collpane->GetPane(), nullptr);
            m_isInside = old_ins;

            return item;
        }
        else
        {
            ReportError("no control within panewindow");
            return nullptr;
        }
    }
    else
    {
        XRC_MAKE_INSTANCE(ctrl, wxCollapsiblePane)

        wxString label = GetText("label");
        if (label.empty())
        {
            ReportParamError("label", "label cannot be empty");
            return nullptr;
        }

        ctrl->Create(m_parentAsWindow,
                    GetID(),
                    label,
                    GetPosition(), GetSize(),
                    GetStyle("style", wxCP_DEFAULT_STYLE),
                    wxValidator{}
                    GetName());

        ctrl->Collapse(GetBool("collapsed"));
        SetupWindow(ctrl);

        wxCollapsiblePane *old_par = m_collpane;
        m_collpane = ctrl;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_collpane, true/*only this handler*/);
        m_isInside = old_ins;
        m_collpane = old_par;

        return ctrl;
    }
}

bool wxCollapsiblePaneXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxCollapsiblePane") ||
            (m_isInside && IsOfClass(node, "panewindow"));
}

#endif // wxUSE_XRC && wxUSE_COLLPANE
