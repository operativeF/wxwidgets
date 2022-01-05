/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_frame.cpp
// Purpose:     XRC resource for dialogs
// Author:      Vaclav Slavik & Aleks.
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC

#include "wx/xrc/xh_frame.h"

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/frame.h"
#include "wx/dialog.h" // to get wxDEFAULT_DIALOG_STYLE

wxIMPLEMENT_DYNAMIC_CLASS(wxFrameXmlHandler, wxXmlResourceHandler);

wxFrameXmlHandler::wxFrameXmlHandler()  
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
#ifdef __WXMAC__
    XRC_ADD_STYLE(wxFRAME_EX_METAL);
#endif
    XRC_ADD_STYLE(wxFRAME_EX_CONTEXTHELP);

    AddWindowStyles();
}

wxObject *wxFrameXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(frame, wxFrame)

    frame->Create(m_parentAsWindow,
                  GetID(),
                  GetText("title"),
                  wxDefaultPosition, wxDefaultSize,
                  GetStyle("style", wxDEFAULT_FRAME_STYLE),
                  GetName());

    if (HasParam("size"))
        frame->SetClientSize(GetSize("size", frame));
    if (HasParam("pos"))
        frame->Move(GetPosition());
    if (HasParam("icon"))
        frame->SetIcons(GetIconBundle("icon", wxART_FRAME_ICON));

    SetupWindow(frame);

    CreateChildren(frame);

    if (GetBool("centered", false))
        frame->Centre();

    return frame;
}

bool wxFrameXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxFrame");
}

#endif // wxUSE_XRC
