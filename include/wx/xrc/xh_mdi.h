/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_mdi.h
// Purpose:     XML resource handler for wxMDI
// Author:      David M. Falkinder & Vaclav Slavik
// Created:     14/02/2005
// Copyright:   (c) 2005 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_MDI_H_
#define _WX_XH_MDI_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_MDI

class wxWindow;

class wxMdiXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxMdiXmlHandler);

public:
    wxMdiXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    wxWindow *CreateFrame();
};

#endif // wxUSE_XRC && wxUSE_MDI

#endif // _WX_XH_MDI_H_
