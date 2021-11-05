/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_scrol.cpp
// Purpose:     XRC resource for wxScrollBar
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_SCROLLBAR

#include "wx/xrc/xh_scrol.h"
#include "wx/scrolbar.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxScrollBarXmlHandler, wxXmlResourceHandler);

wxScrollBarXmlHandler::wxScrollBarXmlHandler()
 
{
    XRC_ADD_STYLE(wxSB_HORIZONTAL);
    XRC_ADD_STYLE(wxSB_VERTICAL);
    AddWindowStyles();
}

wxObject *wxScrollBarXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxScrollBar)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle(),
                    wxDefaultValidator,
                    GetName());

    control->SetScrollbar(GetLong( "value", 0),
                          GetLong( "thumbsize",1),
                          GetLong( "range", 10),
                          GetLong( "pagesize",1));

    SetupWindow(control);
    CreateChildren(control);

    return control;
}

bool wxScrollBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxScrollBar");
}

#endif // wxUSE_XRC && wxUSE_SCROLLBAR
