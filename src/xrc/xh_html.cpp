/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_html.cpp
// Purpose:     XRC resource for wxHtmlWindow
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_HTML

#include "wx/xrc/xh_html.h"

#include "wx/html/htmlwin.h"
#include "wx/filesys.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxHtmlWindowXmlHandler, wxXmlResourceHandler);

wxHtmlWindowXmlHandler::wxHtmlWindowXmlHandler()
 
{
    XRC_ADD_STYLE(wxHW_SCROLLBAR_NEVER);
    XRC_ADD_STYLE(wxHW_SCROLLBAR_AUTO);
    XRC_ADD_STYLE(wxHW_NO_SELECTION);
    AddWindowStyles();
}

wxObject *wxHtmlWindowXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxHtmlWindow)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle("style", wxHW_SCROLLBAR_AUTO),
                    GetName());

    if (HasParam("borders"))
    {
        control->SetBorders(GetDimension("borders"));
    }

    if (HasParam("url"))
    {
        wxString url = GetParamValue("url");
        wxFileSystem& fsys = GetCurFileSystem();

        wxFSFile *f = fsys.OpenFile(url);
        if (f)
        {
            control->LoadPage(f->GetLocation());
            delete f;
        }
        else
            control->LoadPage(url);
    }

    else if (HasParam("htmlcode"))
    {
        control->SetPage(GetText("htmlcode"));
    }

    SetupWindow(control);

    return control;
}

bool wxHtmlWindowXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxHtmlWindow");
}

#endif // wxUSE_XRC && wxUSE_HTML
