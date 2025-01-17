/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_cald.h
// Purpose:     XML resource handler for wxCalendarCtrl
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_CALD_H_
#define _WX_XH_CALD_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_CALENDARCTRL

class wxCalendarCtrlXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxCalendarCtrlXmlHandler);

public:
    wxCalendarCtrlXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_CALENDARCTRL

#endif // _WX_XH_CALD_H_
