/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_timectrl.cpp
// Purpose:     XML resource handler for wxTimePickerCtrl
// Author:      Vadim Zeitlin
// Created:     2011-09-22
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_TIMEPICKCTRL

#include "wx/xrc/xh_timectrl.h"
#include "wx/timectrl.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxTimeCtrlXmlHandler, wxXmlResourceHandler);

wxTimeCtrlXmlHandler::wxTimeCtrlXmlHandler()
{
    XRC_ADD_STYLE(wxTP_DEFAULT);
    AddWindowStyles();
}

wxObject *wxTimeCtrlXmlHandler::DoCreateResource()
{
   XRC_MAKE_INSTANCE(picker, wxTimePickerCtrl)

   picker->Create(m_parentAsWindow,
                  GetID(),
                  wxDefaultDateTime,
                  GetPosition(), GetSize(),
                  GetStyle("style", wxTP_DEFAULT),
                  wxValidator{},
                  GetName());

    SetupWindow(picker);

    return picker;
}

bool wxTimeCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxTimePickerCtrl");
}

#endif // wxUSE_XRC && wxUSE_TIMEPICKCTRL
