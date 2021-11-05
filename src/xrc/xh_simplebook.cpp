/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_simplebook.cpp
// Purpose:     XRC resource handler for wxSimplebook
// Author:      Vaclav Slavik
// Created:     2014-08-05
// Copyright:   (c) 2014 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_BOOKCTRL

#include "wx/xrc/xh_simplebook.h"

#ifndef WX_PRECOMP
#endif

#include "wx/simplebook.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxSimplebookXmlHandler, wxXmlResourceHandler);

wxSimplebookXmlHandler::wxSimplebookXmlHandler()
                      
                        
{
    AddWindowStyles();
}

wxObject *wxSimplebookXmlHandler::DoCreateResource()
{
    if (m_class == "simplebookpage")
    {
        wxXmlNode *n = GetParamNode("object");

        if ( !n )
            n = GetParamNode("object_ref");

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_simplebook, nullptr);
            m_isInside = old_ins;
            wxWindow *wnd = wxDynamicCast(item, wxWindow);

            if (wnd)
            {
                m_simplebook->AddPage(wnd, GetText("label"),
                                      GetBool("selected"));
            }
            else
            {
                ReportError(n, "simplebookpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("simplebookpage must have a window child");
            return nullptr;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(sb, wxSimplebook)

        sb->Create(m_parentAsWindow,
                   GetID(),
                   GetPosition(), GetSize(),
                   GetStyle("style"),
                   GetName());

        SetupWindow(sb);

        wxSimplebook *old_par = m_simplebook;
        m_simplebook = sb;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_simplebook, true/*only this handler*/);
        m_isInside = old_ins;
        m_simplebook = old_par;

        return sb;
    }
}

bool wxSimplebookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxSimplebook")) ||
            (m_isInside && IsOfClass(node, "simplebookpage")));
}

#endif // wxUSE_XRC && wxUSE_BOOKCTRL
