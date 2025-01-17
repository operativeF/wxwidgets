/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_stlin.cpp
// Purpose:     XRC resource for wxStaticLine
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_STATLINE

#include "wx/xrc/xh_stlin.h"
#include "wx/statline.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxStaticLineXmlHandler, wxXmlResourceHandler);

wxStaticLineXmlHandler::wxStaticLineXmlHandler()
 
{
    XRC_ADD_STYLE(wxLI_HORIZONTAL);
    XRC_ADD_STYLE(wxLI_VERTICAL);
    AddWindowStyles();
}

wxObject *wxStaticLineXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(line, wxStaticLine)

    line->Create(m_parentAsWindow,
                GetID(),
                GetPosition(), GetSize(),
                GetStyle("style", wxLI_HORIZONTAL),
                GetName());

    SetupWindow(line);

    return line;
}

bool wxStaticLineXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxStaticLine");
}

#endif // wxUSE_XRC && wxUSE_STATLINE
