/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_choicbk.cpp
// Purpose:     XRC resource for wxChoicebook
// Author:      Vaclav Slavik
// Created:     2000/03/21
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_CHOICEBOOK

#include "wx/xrc/xh_choicbk.h"

#include "wx/log.h"

#include "wx/choicebk.h"
#include "wx/imaglist.h"

import WX.Core.Sizer;

wxIMPLEMENT_DYNAMIC_CLASS(wxChoicebookXmlHandler, wxXmlResourceHandler);

wxChoicebookXmlHandler::wxChoicebookXmlHandler()
                       
                        
{
    XRC_ADD_STYLE(wxBK_DEFAULT);
    XRC_ADD_STYLE(wxBK_LEFT);
    XRC_ADD_STYLE(wxBK_RIGHT);
    XRC_ADD_STYLE(wxBK_TOP);
    XRC_ADD_STYLE(wxBK_BOTTOM);

    XRC_ADD_STYLE(wxCHB_DEFAULT);
    XRC_ADD_STYLE(wxCHB_LEFT);
    XRC_ADD_STYLE(wxCHB_RIGHT);
    XRC_ADD_STYLE(wxCHB_TOP);
    XRC_ADD_STYLE(wxCHB_BOTTOM);

    AddWindowStyles();
}

wxObject *wxChoicebookXmlHandler::DoCreateResource()
{
    if (m_class == "choicebookpage")
    {
        wxXmlNode *n = GetParamNode("object");

        if ( !n )
            n = GetParamNode("object_ref");

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_choicebook, nullptr);
            m_isInside = old_ins;
            wxWindow *wnd = dynamic_cast<wxWindow*>(item);

            if (wnd)
            {
                m_choicebook->AddPage(wnd, GetText("label"),
                                           GetBool("selected"));
                if ( HasParam("bitmap") )
                {
                    wxBitmap bmp = GetBitmap("bitmap", wxART_OTHER);
                    wxImageList *imgList = m_choicebook->GetImageList();
                    if ( imgList == nullptr )
                    {
                        imgList = new wxImageList( bmp.GetWidth(), bmp.GetHeight() );
                        m_choicebook->AssignImageList( imgList );
                    }
                    int imgIndex = imgList->Add(bmp);
                    m_choicebook->SetPageImage(m_choicebook->GetPageCount()-1, imgIndex );
                }
                else if ( HasParam("image") )
                {
                    if ( m_choicebook->GetImageList() )
                    {
                        m_choicebook->SetPageImage(m_choicebook->GetPageCount()-1,
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
                ReportError(n, "choicebookpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("choicebookpage must have a window child");
            return nullptr;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(nb, wxChoicebook)

        nb->Create(m_parentAsWindow,
                   GetID(),
                   GetPosition(), GetSize(),
                   GetStyle("style"),
                   GetName());

        wxImageList *imagelist = GetImageList();
        if ( imagelist )
            nb->AssignImageList(imagelist);

        wxChoicebook *old_par = m_choicebook;
        m_choicebook = nb;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_choicebook, true/*only this handler*/);
        m_isInside = old_ins;
        m_choicebook = old_par;

        return nb;
    }
}

bool wxChoicebookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxChoicebook")) ||
            (m_isInside && IsOfClass(node, "choicebookpage")));
}

#endif // wxUSE_XRC && wxUSE_CHOICEBOOK
