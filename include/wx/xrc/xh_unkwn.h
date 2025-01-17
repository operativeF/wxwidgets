/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_unkwn.h
// Purpose:     XML resource handler for unknown widget
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_UNKWN_H_
#define _WX_XH_UNKWN_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC

class wxUnknownWidgetXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxUnknownWidgetXmlHandler);

public:
    wxUnknownWidgetXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC

#endif // _WX_XH_UNKWN_H_
