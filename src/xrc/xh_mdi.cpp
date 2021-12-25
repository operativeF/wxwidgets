/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_mdi.cpp
// Purpose:     XRC resource for wxMDI
// Author:      David M. Falkinder & Vaclav Slavik
// Created:     14/02/2005
// Copyright:   (c) 2005 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_MDI

#include "wx/xrc/xh_mdi.h"
#include "wx/mdi.h"

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/dialog.h" // to get wxDEFAULT_DIALOG_STYLE

wxIMPLEMENT_DYNAMIC_CLASS(wxMdiXmlHandler, wxXmlResourceHandler);

wxMdiXmlHandler::wxMdiXmlHandler()  
{
    XRC_ADD_STYLE(wxSTAY_ON_TOP);
    XRC_ADD_STYLE(wxCAPTION);
    XRC_ADD_STYLE(wxDEFAULT_DIALOG_STYLE);
    XRC_ADD_STYLE(wxDEFAULT_FRAME_STYLE);
    XRC_ADD_STYLE(wxSYSTEM_MENU);
    XRC_ADD_STYLE(wxRESIZE_BORDER);
    XRC_ADD_STYLE(wxCLOSE_BOX);
    XRC_ADD_STYLE(wxFRAME_NO_TASKBAR);
    XRC_ADD_STYLE(wxFRAME_SHAPED);
    XRC_ADD_STYLE(wxFRAME_TOOL_WINDOW);
    XRC_ADD_STYLE(wxFRAME_FLOAT_ON_PARENT);
    XRC_ADD_STYLE(wxMAXIMIZE_BOX);
    XRC_ADD_STYLE(wxMINIMIZE_BOX);
    XRC_ADD_STYLE(wxSTAY_ON_TOP);
    XRC_ADD_STYLE(wxTAB_TRAVERSAL);
    XRC_ADD_STYLE(wxWS_EX_VALIDATE_RECURSIVELY);
#ifdef __WXMAC__
    XRC_ADD_STYLE(wxFRAME_EX_METAL);
#endif

    XRC_ADD_STYLE(wxHSCROLL);
    XRC_ADD_STYLE(wxVSCROLL);
    XRC_ADD_STYLE(wxMAXIMIZE);
    XRC_ADD_STYLE(wxFRAME_NO_WINDOW_MENU);

    AddWindowStyles();
}

wxWindow *wxMdiXmlHandler::CreateFrame()
{
    if (m_class == "wxMDIParentFrame")
    {
        XRC_MAKE_INSTANCE(frame, wxMDIParentFrame)

        frame->Create(m_parentAsWindow,
                      GetID(),
                      GetText("title"),
                      wxDefaultPosition, wxDefaultSize,
                      GetStyle("style",
                               wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL),
                      GetName());
        return frame;
    }
    else // wxMDIChildFrame
    {
        wxMDIParentFrame *mdiParent = dynamic_cast<wxMDIParentFrame*>(m_parent);

        if ( !mdiParent )
        {
            ReportError("parent of wxMDIChildFrame must be wxMDIParentFrame");
            return nullptr;
        }

        XRC_MAKE_INSTANCE(frame, wxMDIChildFrame)

        frame->Create(mdiParent,
                      GetID(),
                      GetText("title"),
                      wxDefaultPosition, wxDefaultSize,
                      GetStyle("style", wxDEFAULT_FRAME_STYLE),
                      GetName());

        return frame;
    }
}

wxObject *wxMdiXmlHandler::DoCreateResource()
{
    wxWindow *frame = CreateFrame();

    if (HasParam("size"))
        frame->SetClientSize(GetSize());
    if (HasParam("pos"))
        frame->Move(GetPosition());
    if (HasParam("icon"))
    {
        wxFrame* f = dynamic_cast<wxFrame*>(frame);
        if (f)
            f->SetIcons(GetIconBundle("icon", wxART_FRAME_ICON));
    }

    SetupWindow(frame);

    CreateChildren(frame);

    if (GetBool("centered", false))
        frame->Centre();

    return frame;
}

bool wxMdiXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxMDIParentFrame") ||
            IsOfClass(node, "wxMDIChildFrame"));
}

#endif // wxUSE_XRC && wxUSE_MDI
