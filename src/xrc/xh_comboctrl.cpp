/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_comboctrl.cpp
// Purpose:     XRC resource for wxComboCtrl
// Author:      Jaakko Salli
// Created:     2009/01/25
// Copyright:   (c) 2009 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_COMBOCTRL

#include "wx/xrc/xh_comboctrl.h"

#include "wx/intl.h"
#include "wx/textctrl.h"    // for wxTE_PROCESS_ENTER

#include "wx/combo.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxComboCtrlXmlHandler, wxXmlResourceHandler);

wxComboCtrlXmlHandler::wxComboCtrlXmlHandler()
                      
{
    XRC_ADD_STYLE(wxCB_SORT);
    XRC_ADD_STYLE(wxCB_READONLY);
    XRC_ADD_STYLE(wxTE_PROCESS_ENTER);
    XRC_ADD_STYLE(wxCC_SPECIAL_DCLICK);
    XRC_ADD_STYLE(wxCC_STD_BUTTON);
    AddWindowStyles();
}

wxObject *wxComboCtrlXmlHandler::DoCreateResource()
{
    if( m_class == "wxComboCtrl")
    {
        XRC_MAKE_INSTANCE(control, wxComboCtrl)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetText("value"),
                        GetPosition(), GetSize(),
                        GetStyle(),
                        wxValidator{},
                        GetName());

        SetupWindow(control);

        return control;
    }
    return nullptr;
}

bool wxComboCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxComboCtrl");
}

#endif // wxUSE_XRC && wxUSE_COMBOBOX
