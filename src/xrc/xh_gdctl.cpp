/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_gdctl.cpp
// Purpose:     XRC resource for wxGenericDirCtrl
// Author:      Markus Greither
// Created:     2002/01/20
// Copyright:   (c) 2002 Markus Greither
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_DIRDLG

#include "wx/xrc/xh_gdctl.h"

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
#endif

#include "wx/dirctrl.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxGenericDirCtrlXmlHandler, wxXmlResourceHandler);

wxGenericDirCtrlXmlHandler::wxGenericDirCtrlXmlHandler()
 
{
    XRC_ADD_STYLE(wxDIRCTRL_DIR_ONLY);
    XRC_ADD_STYLE(wxDIRCTRL_3D_INTERNAL);
    XRC_ADD_STYLE(wxDIRCTRL_SELECT_FIRST);
    XRC_ADD_STYLE(wxDIRCTRL_SHOW_FILTERS);
    XRC_ADD_STYLE(wxDIRCTRL_EDIT_LABELS);
    XRC_ADD_STYLE(wxDIRCTRL_MULTIPLE);
    AddWindowStyles();
}

wxObject *wxGenericDirCtrlXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(ctrl, wxGenericDirCtrl)

    ctrl->Create(m_parentAsWindow,
                 GetID(),
                 GetText("defaultfolder"),
                 GetPosition(), GetSize(),
                 GetStyle(),
                 GetText("filter"),
                 (int)GetLong("defaultfilter"),
                 GetName());

    SetupWindow(ctrl);

    return ctrl;
}

bool wxGenericDirCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxGenericDirCtrl");
}

#endif // wxUSE_XRC && wxUSE_DIRDLG
