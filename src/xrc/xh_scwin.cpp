/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_scwin.cpp
// Purpose:     XRC resource for wxScrolledWindow
// Author:      Vaclav Slavik
// Created:     2002/10/18
// Copyright:   (c) 2002 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC

#include "wx/xrc/xh_scwin.h"
#include "wx/frame.h"
#include "wx/scrolwin.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxScrolledWindowXmlHandler, wxXmlResourceHandler);

wxScrolledWindowXmlHandler::wxScrolledWindowXmlHandler()
 
{
    XRC_ADD_STYLE(wxHSCROLL);
    XRC_ADD_STYLE(wxVSCROLL);

    // wxPanel styles
    XRC_ADD_STYLE(wxTAB_TRAVERSAL);

    AddWindowStyles();
}

wxObject *wxScrolledWindowXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxScrolledWindow)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle("style", wxHSCROLL | wxVSCROLL),
                    GetName());

    SetupWindow(control);
    CreateChildren(control);

    if ( HasParam("scrollrate") )
    {
        wxSize rate = GetSize("scrollrate");
        control->SetScrollRate(rate.x, rate.y);
    }

    return control;
}

bool wxScrolledWindowXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxScrolledWindow");
}

#endif // wxUSE_XRC
