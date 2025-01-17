/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_cald.cpp
// Purpose:     XRC resource for wxCalendarCtrl
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_CALENDARCTRL

#include "wx/xrc/xh_cald.h"

#ifndef WX_PRECOMP
    #include "wx/event.h"
#endif //WX_PRECOMP

#include "wx/calctrl.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxCalendarCtrlXmlHandler, wxXmlResourceHandler);

wxCalendarCtrlXmlHandler::wxCalendarCtrlXmlHandler()
 
{
    XRC_ADD_STYLE(wxCAL_SUNDAY_FIRST);
    XRC_ADD_STYLE(wxCAL_MONDAY_FIRST);
    XRC_ADD_STYLE(wxCAL_SHOW_HOLIDAYS);
    XRC_ADD_STYLE(wxCAL_NO_YEAR_CHANGE);
    XRC_ADD_STYLE(wxCAL_NO_MONTH_CHANGE);
    XRC_ADD_STYLE(wxCAL_SEQUENTIAL_MONTH_SELECTION);
    XRC_ADD_STYLE(wxCAL_SHOW_SURROUNDING_WEEKS);

    AddWindowStyles();
}


wxObject *wxCalendarCtrlXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(calendar, wxCalendarCtrl)

    calendar->Create(m_parentAsWindow,
                     GetID(),
                     wxDefaultDateTime,
                     /*TODO: take it from resource*/
                     GetPosition(), GetSize(),
                     GetStyle(),
                     GetName());

    SetupWindow(calendar);

    return calendar;
}

bool wxCalendarCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxCalendarCtrl");
}

#endif // wxUSE_XRC && wxUSE_CALENDARCTRL
