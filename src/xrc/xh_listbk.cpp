/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_listbk.cpp
// Purpose:     XRC resource for wxListbook
// Author:      Vaclav Slavik
// Created:     2000/03/21
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_LISTBOOK

#include "wx/xrc/xh_listbk.h"

#include "wx/log.h"

#include "wx/listbook.h"
#include "wx/imaglist.h"

import WX.Core.Sizer;

wxIMPLEMENT_DYNAMIC_CLASS(wxListbookXmlHandler, wxXmlResourceHandler);

wxListbookXmlHandler::wxListbookXmlHandler()
                     
                      
{
    XRC_ADD_STYLE(wxBK_DEFAULT);
    XRC_ADD_STYLE(wxBK_LEFT);
    XRC_ADD_STYLE(wxBK_RIGHT);
    XRC_ADD_STYLE(wxBK_TOP);
    XRC_ADD_STYLE(wxBK_BOTTOM);

    XRC_ADD_STYLE(wxLB_DEFAULT);
    XRC_ADD_STYLE(wxLB_LEFT);
    XRC_ADD_STYLE(wxLB_RIGHT);
    XRC_ADD_STYLE(wxLB_TOP);
    XRC_ADD_STYLE(wxLB_BOTTOM);

    AddWindowStyles();
}

wxObject *wxListbookXmlHandler::DoCreateResource()
{
    if (m_class == "listbookpage")
    {
        wxXmlNode *n = GetParamNode("object");

        if ( !n )
            n = GetParamNode("object_ref");

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_listbook, nullptr);
            m_isInside = old_ins;
            wxWindow *wnd = dynamic_cast<wxWindow*>(item);

            if (wnd)
            {
                m_listbook->AddPage(wnd, GetText("label"),
                                         GetBool("selected"));
                if ( HasParam("bitmap") )
                {
                    wxBitmap bmp = GetBitmap("bitmap", wxART_OTHER);
                    wxImageList *imgList = m_listbook->GetImageList();
                    if ( imgList == nullptr )
                    {
                        imgList = new wxImageList( bmp.GetWidth(), bmp.GetHeight() );
                        m_listbook->AssignImageList( imgList );
                    }
                    int imgIndex = imgList->Add(bmp);
                    m_listbook->SetPageImage(m_listbook->GetPageCount()-1, imgIndex );
                }
                else if ( HasParam("image") )
                {
                    if ( m_listbook->GetImageList() )
                    {
                        m_listbook->SetPageImage(m_listbook->GetPageCount()-1,
                                                 GetLong("image") );
                    }
                    else // image without image list?
                    {
                        ReportError(n, "image can only be used in conjunction "
                                       "with imagelist");
                    }
                }
            }
            else
            {
                ReportError(n, "listbookpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("listbookpage must have a window child");
            return nullptr;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(nb, wxListbook)

        nb->Create(m_parentAsWindow,
                   GetID(),
                   GetPosition(), GetSize(),
                   GetStyle("style"),
                   GetName());

        wxImageList *imagelist = GetImageList();
        if ( imagelist )
            nb->AssignImageList(imagelist);

        wxListbook *old_par = m_listbook;
        m_listbook = nb;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_listbook, true/*only this handler*/);
        m_isInside = old_ins;
        m_listbook = old_par;

        return nb;
    }
}

bool wxListbookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxListbook")) ||
            (m_isInside && IsOfClass(node, "listbookpage")));
}

#endif // wxUSE_XRC && wxUSE_LISTBOOK
