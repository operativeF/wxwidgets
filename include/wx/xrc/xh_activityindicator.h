/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_activityindicator.h
// Purpose:     Declaration of wxActivityIndicator XRC handler.
// Author:      Vadim Zeitlin
// Created:     2015-03-18
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_ACTIVITYINDICATOR_H_
#define _WX_XH_ACTIVITYINDICATOR_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_ACTIVITYINDICATOR

class wxActivityIndicatorXmlHandler : public wxXmlResourceHandler
{
public:
    wxActivityIndicatorXmlHandler();

    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxActivityIndicatorXmlHandler);
};

#endif // wxUSE_XRC && wxUSE_ACTIVITYINDICATOR

#endif // _WX_XH_ACTIVITYINDICATOR_H_
