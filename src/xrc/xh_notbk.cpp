/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_notbk.cpp
// Purpose:     XRC resource for wxNotebook
// Author:      Vaclav Slavik
// Created:     2000/03/21
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_NOTEBOOK

#include "wx/xrc/xh_notbk.h"

#include "wx/log.h"

#include "wx/notebook.h"
#include "wx/imaglist.h"

import WX.Core.Sizer;

wxIMPLEMENT_DYNAMIC_CLASS(wxNotebookXmlHandler, wxXmlResourceHandler);

wxNotebookXmlHandler::wxNotebookXmlHandler()
                     
                      
{
    XRC_ADD_STYLE(wxBK_DEFAULT);
    XRC_ADD_STYLE(wxBK_LEFT);
    XRC_ADD_STYLE(wxBK_RIGHT);
    XRC_ADD_STYLE(wxBK_TOP);
    XRC_ADD_STYLE(wxBK_BOTTOM);

    // provide the old synonyms for these fields as well
    XRC_ADD_STYLE(wxNB_DEFAULT);
    XRC_ADD_STYLE(wxNB_LEFT);
    XRC_ADD_STYLE(wxNB_RIGHT);
    XRC_ADD_STYLE(wxNB_TOP);
    XRC_ADD_STYLE(wxNB_BOTTOM);

    XRC_ADD_STYLE(wxNB_FIXEDWIDTH);
    XRC_ADD_STYLE(wxNB_MULTILINE);
    XRC_ADD_STYLE(wxNB_NOPAGETHEME);

    AddWindowStyles();
}

wxObject *wxNotebookXmlHandler::DoCreateResource()
{
    if (m_class == "notebookpage")
    {
        wxXmlNode *n = GetParamNode("object");

        if ( !n )
            n = GetParamNode("object_ref");

        if (n)
        {
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, m_notebook, nullptr);
            m_isInside = old_ins;
            wxWindow *wnd = dynamic_cast<wxWindow*>(item);

            if (wnd)
            {
                m_notebook->AddPage(wnd, GetText("label"),
                                         GetBool("selected"));
                if ( HasParam("bitmap") )
                {
                    wxBitmap bmp = GetBitmap("bitmap", wxART_OTHER);
                    wxImageList *imgList = m_notebook->GetImageList();
                    if ( imgList == nullptr )
                    {
                        imgList = new wxImageList( bmp.GetWidth(), bmp.GetHeight() );
                        m_notebook->AssignImageList( imgList );
                    }
                    int imgIndex = imgList->Add(bmp);
                    m_notebook->SetPageImage(m_notebook->GetPageCount()-1, imgIndex );
                }
                else if ( HasParam("image") )
                {
                    if ( m_notebook->GetImageList() )
                    {
                        m_notebook->SetPageImage(m_notebook->GetPageCount()-1,
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
                ReportError(n, "notebookpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("notebookpage must have a window child");
            return nullptr;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(nb, wxNotebook)

        nb->Create(m_parentAsWindow,
                   GetID(),
                   GetPosition(), GetSize(),
                   GetStyle("style"),
                   GetName());

        wxImageList *imagelist = GetImageList();
        if ( imagelist )
            nb->AssignImageList(imagelist);

        SetupWindow(nb);

        wxNotebook *old_par = m_notebook;
        m_notebook = nb;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_notebook, true/*only this handler*/);
        m_isInside = old_ins;
        m_notebook = old_par;

        return nb;
    }
}

bool wxNotebookXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxNotebook")) ||
            (m_isInside && IsOfClass(node, "notebookpage")));
}

#endif // wxUSE_XRC && wxUSE_NOTEBOOK
