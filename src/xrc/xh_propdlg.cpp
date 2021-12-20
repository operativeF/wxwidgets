/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_propdlg.cpp
// Purpose:     XRC resource handler for wxPropertySheetDialog
// Author:      Sander Berents
// Created:     2007/07/12
// Copyright:   (c) 2007 Sander Berents
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_BOOKCTRL

#include "wx/xrc/xh_propdlg.h"

#include "wx/log.h"
#include "wx/frame.h"

#include "wx/bookctrl.h"
#include "wx/propdlg.h"
#include "wx/imaglist.h"

import WX.Core.Sizer;

wxIMPLEMENT_DYNAMIC_CLASS(wxPropertySheetDialogXmlHandler, wxXmlResourceHandler);

wxPropertySheetDialogXmlHandler::wxPropertySheetDialogXmlHandler()
                     
                      
{
    XRC_ADD_STYLE(wxSTAY_ON_TOP);
    XRC_ADD_STYLE(wxCAPTION);
    XRC_ADD_STYLE(wxDEFAULT_DIALOG_STYLE);
    XRC_ADD_STYLE(wxSYSTEM_MENU);
    XRC_ADD_STYLE(wxRESIZE_BORDER);
    XRC_ADD_STYLE(wxCLOSE_BOX);
    XRC_ADD_STYLE(wxDIALOG_NO_PARENT);

    XRC_ADD_STYLE(wxTAB_TRAVERSAL);
    XRC_ADD_STYLE(wxWS_EX_VALIDATE_RECURSIVELY);
#ifdef __WXMAC__
    XRC_ADD_STYLE(wxDIALOG_EX_METAL);
#endif
    XRC_ADD_STYLE(wxMAXIMIZE_BOX);
    XRC_ADD_STYLE(wxMINIMIZE_BOX);
    XRC_ADD_STYLE(wxFRAME_SHAPED);
    XRC_ADD_STYLE(wxDIALOG_EX_CONTEXTHELP);

    AddWindowStyles();
}

wxObject *wxPropertySheetDialogXmlHandler::DoCreateResource()
{
    if (m_class == "propertysheetpage")
    {
        wxXmlNode *n = GetParamNode("object");

        if (!n) n = GetParamNode("object_ref");

        if (n)
        {
            wxBookCtrlBase *bookctrl = m_dialog->GetBookCtrl();
            bool old_ins = m_isInside;
            m_isInside = false;
            wxObject *item = CreateResFromNode(n, bookctrl, nullptr);
            m_isInside = old_ins;
            wxWindow *wnd = wxDynamicCast(item, wxWindow);

            if (wnd)
            {
                bookctrl->AddPage(wnd, GetText("label")), GetBool(wxT("selected"));
                if (HasParam("bitmap"))
                {
                    wxBitmap bmp = GetBitmap("bitmap", wxART_OTHER);
                    wxImageList *imgList = bookctrl->GetImageList();
                    if (imgList == nullptr)
                    {
                        imgList = new wxImageList(bmp.GetWidth(), bmp.GetHeight());
                        bookctrl->AssignImageList(imgList);
                    }
                    int imgIndex = imgList->Add(bmp);
                    bookctrl->SetPageImage(bookctrl->GetPageCount()-1, imgIndex);
                }
            }
            else
            {
                ReportError(n, "propertysheetpage child must be a window");
            }
            return wnd;
        }
        else
        {
            ReportError("propertysheetpage must have a window child");
            return nullptr;
        }
    }

    else
    {
        XRC_MAKE_INSTANCE(dlg, wxPropertySheetDialog)

        dlg->Create(m_parentAsWindow,
                   GetID(),
                   GetText("title"),
                   GetPosition(),
                   GetSize(),
                   GetStyle(),
                   GetName());

        if (HasParam("icon"))
            dlg->SetIcons(GetIconBundle("icon", wxART_FRAME_ICON));

        SetupWindow(dlg);

        wxPropertySheetDialog *old_par = m_dialog;
        m_dialog = dlg;
        bool old_ins = m_isInside;
        m_isInside = true;
        CreateChildren(m_dialog, true/*only this handler*/);
        m_isInside = old_ins;
        m_dialog = old_par;

        if (GetBool("centered", false)) dlg->Centre();
        wxString buttons = GetText("buttons");
        if (!buttons.IsEmpty())
        {
            int flags = 0;
            if (buttons.Find("wxOK")         != wxNOT_FOUND) flags |= wxOK;
            if (buttons.Find("wxCANCEL")     != wxNOT_FOUND) flags |= wxCANCEL;
            if (buttons.Find("wxYES")        != wxNOT_FOUND) flags |= wxYES;
            if (buttons.Find("wxNO")         != wxNOT_FOUND) flags |= wxNO;
            if (buttons.Find("wxHELP")       != wxNOT_FOUND) flags |= wxHELP;
            if (buttons.Find("wxNO_DEFAULT") != wxNOT_FOUND) flags |= wxNO_DEFAULT;
            dlg->CreateButtons(flags);
        }

        return dlg;
    }
}

bool wxPropertySheetDialogXmlHandler::CanHandle(wxXmlNode *node)
{
    return ((!m_isInside && IsOfClass(node, "wxPropertySheetDialog")) ||
            (m_isInside && IsOfClass(node, "propertysheetpage")));
}

#endif // wxUSE_XRC && wxUSE_BOOKCTRL
