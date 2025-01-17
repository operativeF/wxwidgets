/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_statbar.h
// Purpose:     XML resource handler for wxStatusBar
// Author:      Brian Ravnsgaard Riis
// Created:     2004/01/21
// Copyright:   (c) 2004 Brian Ravnsgaard Riis
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_STATBAR_H_
#define _WX_XH_STATBAR_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_STATUSBAR

class wxStatusBarXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxStatusBarXmlHandler);

public:
    wxStatusBarXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_STATUSBAR

#endif // _WX_XH_STATBAR_H_
