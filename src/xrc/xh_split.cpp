/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_split.cpp
// Purpose:     XRC resource for wxSplitterWindow
// Author:      panga@freemail.hu, Vaclav Slavik
// Created:     2003/01/26
// Copyright:   (c) 2003 panga@freemail.hu, Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_SPLITTER

#include "wx/xrc/xh_split.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/splitter.h"

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxSplitterWindowXmlHandler, wxXmlResourceHandler);

wxSplitterWindowXmlHandler::wxSplitterWindowXmlHandler()  
{
    XRC_ADD_STYLE(wxSP_3D);
    XRC_ADD_STYLE(wxSP_3DSASH);
    XRC_ADD_STYLE(wxSP_3DBORDER);
    XRC_ADD_STYLE(wxSP_BORDER);
    XRC_ADD_STYLE(wxSP_NOBORDER);
    XRC_ADD_STYLE(wxSP_PERMIT_UNSPLIT);
    XRC_ADD_STYLE(wxSP_LIVE_UPDATE);
    XRC_ADD_STYLE(wxSP_NO_XP_THEME);
    AddWindowStyles();
}

wxObject *wxSplitterWindowXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(splitter, wxSplitterWindow)

    splitter->Create(m_parentAsWindow,
                     GetID(),
                     GetPosition(), GetSize(),
                     GetStyle("style", wxSP_3D),
                     GetName());

    SetupWindow(splitter);

    long sashpos = GetDimension("sashpos", 0);
    long minpanesize = GetDimension("minsize", -1);
    float gravity = GetFloat("gravity");
    if (minpanesize != -1)
        splitter->SetMinimumPaneSize(minpanesize);
    if (gravity != 0)
        splitter->SetSashGravity(double(gravity));

    wxWindow *win1 = nullptr, *win2 = nullptr;
    wxXmlNode *n = m_node->GetChildren();
    while (n)
    {
        if ((n->GetType() == wxXML_ELEMENT_NODE) &&
            (n->GetName() == "object" ||
             n->GetName() == "object_ref"))
        {
            wxObject *created = CreateResFromNode(n, splitter, nullptr);
            wxWindow *win = wxDynamicCast(created, wxWindow);
            if (win1 == nullptr)
            {
                win1 = win;
            }
            else
            {
                win2 = win;
                break;
            }
        }
        n = n->GetNext();
    }

    if (win1 == nullptr)
        ReportError("wxSplitterWindow node must contain at least one window");

    bool horizontal = (GetParamValue("orientation")) != wxT("vertical");
    if (win1 && win2)
    {
        if (horizontal)
            splitter->SplitHorizontally(win1, win2, sashpos);
        else
            splitter->SplitVertically(win1, win2, sashpos);
    }
    else
    {
        splitter->Initialize(win1);
    }

    return splitter;
}

bool wxSplitterWindowXmlHandler::CanHandle(wxXmlNode *node)
{
     return IsOfClass(node, "wxSplitterWindow");
}

#endif // wxUSE_XRC && wxUSE_SPLITTER
