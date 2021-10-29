/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_stbox.cpp
// Purpose:     XRC resource for wxStaticBox
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_STATBOX

#include "wx/xrc/xh_stbox.h"
#include "wx/statbox.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxStaticBoxXmlHandler, wxXmlResourceHandler);

wxStaticBoxXmlHandler::wxStaticBoxXmlHandler()
                      
{
    AddWindowStyles();
}

wxObject *wxStaticBoxXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(box, wxStaticBox)

    box->Create(m_parentAsWindow,
                GetID(),
                GetText(wxT("label")),
                GetPosition(), GetSize(),
                GetStyle(),
                GetName());

    SetupWindow(box);

    return box;
}

bool wxStaticBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxStaticBox"));
}

#endif // wxUSE_XRC && wxUSE_STATBOX
