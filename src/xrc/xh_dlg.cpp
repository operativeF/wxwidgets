/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_dlg.cpp
// Purpose:     XRC resource for dialogs
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC

#include "wx/xrc/xh_dlg.h"

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/frame.h"
#include "wx/dialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxDialogXmlHandler, wxXmlResourceHandler);

wxDialogXmlHandler::wxDialogXmlHandler()  
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

wxObject *wxDialogXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(dlg, wxDialog)

    dlg->Create(m_parentAsWindow,
                GetID(),
                GetText("title"),
                wxDefaultPosition, wxDefaultSize,
                GetStyle("style", wxDEFAULT_DIALOG_STYLE),
                GetName());

    if (HasParam("size"))
        dlg->SetClientSize(GetSize("size", dlg));
    if (HasParam("pos"))
        dlg->Move(GetPosition());
    if (HasParam("icon"))
        dlg->SetIcons(GetIconBundle("icon", wxART_FRAME_ICON));

    SetupWindow(dlg);

    CreateChildren(dlg);

    if (GetBool("centered", false))
        dlg->Centre();

    return dlg;
}

bool wxDialogXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxDialog");
}

#endif // wxUSE_XRC
