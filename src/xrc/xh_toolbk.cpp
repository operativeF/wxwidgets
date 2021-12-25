/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_toolbk.cpp
// Purpose:     XRC resource for wxToolbook
// Author:      Andrea Zanellato
// Created:     2009/12/12
// Copyright:   (c) 2010 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_XRC && wxUSE_TOOLBOOK

#include "wx/xrc/xh_toolbk.h"

#include "wx/log.h"

#include "wx/toolbook.h"
#include "wx/imaglist.h"

#include "wx/xml/xml.h"

import WX.Core.Sizer;

wxIMPLEMENT_DYNAMIC_CLASS(wxToolbookXmlHandler, wxXmlResourceHandler);

wxToolbookXmlHandler::wxToolbookXmlHandler()
                     
                      
{
    XRC_ADD_STYLE(wxBK_DEFAULT);
    XRC_ADD_STYLE(wxBK_TOP);
    XRC_ADD_STYLE(wxBK_BOTTOM);
    XRC_ADD_STYLE(wxBK_LEFT);
    XRC_ADD_STYLE(wxBK_RIGHT);

    XRC_ADD_STYLE(wxTBK_BUTTONBAR);
    XRC_ADD_STYLE(wxTBK_HORZ_LAYOUT);

    AddWindowStyles();
}

wxObject *wxToolbookXmlHandler::DoCreateResource()
{
    if (m_class == "toolbookpage")
    {
        wxXmlNode *n = GetParamNode("object");

        if ( !n )
            n = GetParamNode("object_ref");

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_toolbook, nullptr);
            m_isInside = old_ins;
            wxWindow *wnd = dynamic_cast<wxWindow*>(item);

            if (wnd)
            {
                int imgId = -1;

                if ( HasParam("bitmap") )
                {
                    wxBitmap bmp = GetBitmap("bitmap", wxART_OTHER);
                    wxImageList *imgList = m_toolbook->GetImageList();
                    if ( imgList == nullptr )
                    {
                        imgList = new wxImageList( bmp.GetWidth(), bmp.GetHeight() );
                        m_toolbook->AssignImageList( imgList );
                    }
                    imgId = imgList->Add(bmp);
                }
                else if ( HasParam("image") )
                {
                    if ( m_toolbook->GetImageList() )
                    {
                        imgId = (int)GetLong("image");
                    }
                    else // image without image list?
                    {
                        ReportError(n, "image can only be used in conjunction "
                                       "with imagelist");
                    }
                }

                m_toolbook->AddPage(wnd, GetText("label"),
                        GetBool("selected"), imgId );
            }
            else
            {
                ReportError(n, "toolbookpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("toolbookpage must have a window child");
            return nullptr;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(nb, wxToolbook)

        nb->Create( m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle("style"),
                    GetName() );

        wxImageList *imagelist = GetImageList();
        if ( imagelist )
            nb->AssignImageList(imagelist);

        wxToolbook *old_par = m_toolbook;
        m_toolbook = nb;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_toolbook, true/*only this handler*/);
        m_isInside = old_ins;
        m_toolbook = old_par;

        return nb;
    }
}

bool wxToolbookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxToolbook")) ||
            (m_isInside && IsOfClass(node, "toolbookpage")));
}

#endif // wxUSE_XRC && wxUSE_TOOLBOOK
