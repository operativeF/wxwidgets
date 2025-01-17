/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_filepicker.cpp
// Purpose:     XML resource handler for wxFilePickerCtrl
// Author:      Francesco Montorsi
// Created:     2006-04-17
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_FILEPICKERCTRL

#include "wx/xrc/xh_filepicker.h"
#include "wx/filepicker.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxFilePickerCtrlXmlHandler, wxXmlResourceHandler);

wxFilePickerCtrlXmlHandler::wxFilePickerCtrlXmlHandler()  
{
    XRC_ADD_STYLE(wxFLP_OPEN);
    XRC_ADD_STYLE(wxFLP_SAVE);
    XRC_ADD_STYLE(wxFLP_OVERWRITE_PROMPT);
    XRC_ADD_STYLE(wxFLP_FILE_MUST_EXIST);
    XRC_ADD_STYLE(wxFLP_CHANGE_DIR);
    XRC_ADD_STYLE(wxFLP_SMALL);
    XRC_ADD_STYLE(wxFLP_DEFAULT_STYLE);
    XRC_ADD_STYLE(wxFLP_USE_TEXTCTRL);
    AddWindowStyles();
}

wxObject *wxFilePickerCtrlXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(picker, wxFilePickerCtrl)

    picker->Create(m_parentAsWindow,
                  GetID(),
                  GetParamValue("value"),
                  GetText("message"),
                  GetParamValue("wildcard"),
                  GetPosition(), GetSize(),
                  GetStyle("style", wxFLP_DEFAULT_STYLE),
                  wxValidator{},
                  GetName());

    SetupWindow(picker);
    return picker;
}

bool wxFilePickerCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxFilePickerCtrl");
}

#endif // wxUSE_XRC && wxUSE_FILEPICKERCTRL
