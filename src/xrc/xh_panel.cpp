/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_panel.cpp
// Purpose:     XRC resource for panels
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC

#include "wx/xrc/xh_panel.h"

#include "wx/panel.h"
#include "wx/frame.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxPanelXmlHandler, wxXmlResourceHandler);

wxPanelXmlHandler::wxPanelXmlHandler()  
{
    XRC_ADD_STYLE(wxTAB_TRAVERSAL);

    AddWindowStyles();
}

wxObject *wxPanelXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(panel, wxPanel)

    panel->Create(m_parentAsWindow,
                  GetID(),
                  GetPosition(), GetSize(),
                  GetStyle("style", wxTAB_TRAVERSAL),
                  GetName());

    SetupWindow(panel);
    CreateChildren(panel);

    return panel;
}

bool wxPanelXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxPanel");
}

#endif // wxUSE_XRC
