/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_dirpicker.cpp
// Purpose:     XML resource handler for wxDirPickerCtrl
// Author:      Francesco Montorsi
// Created:     2006-04-17
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_DIRPICKERCTRL

#include "wx/xrc/xh_dirpicker.h"
#include "wx/filepicker.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxDirPickerCtrlXmlHandler, wxXmlResourceHandler);

wxDirPickerCtrlXmlHandler::wxDirPickerCtrlXmlHandler()  
{
    XRC_ADD_STYLE(wxDIRP_USE_TEXTCTRL);
    XRC_ADD_STYLE(wxDIRP_DIR_MUST_EXIST);
    XRC_ADD_STYLE(wxDIRP_CHANGE_DIR);
    XRC_ADD_STYLE(wxDIRP_SMALL);
    XRC_ADD_STYLE(wxDIRP_DEFAULT_STYLE);
    AddWindowStyles();
}

wxObject *wxDirPickerCtrlXmlHandler::DoCreateResource()
{
   XRC_MAKE_INSTANCE(picker, wxDirPickerCtrl)

   picker->Create(m_parentAsWindow,
                  GetID(),
                  GetParamValue("value"),
                  GetText("message"),
                  GetPosition(), GetSize(),
                  GetStyle("style", wxDIRP_DEFAULT_STYLE),
                  wxValidator{},
                  GetName());

    SetupWindow(picker);

    return picker;
}

bool wxDirPickerCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxDirPickerCtrl");
}

#endif // wxUSE_XRC && wxUSE_DIRPICKERCTRL
