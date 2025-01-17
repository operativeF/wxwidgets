/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_gdctl.h
// Purpose:     XML resource handler for wxGenericDirCtrl
// Author:      Markus Greither
// Created:     2002/01/20
// Copyright:   (c) 2002 Markus Greither
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_GDCTL_H_
#define _WX_XH_GDCTL_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_DIRDLG

class wxGenericDirCtrlXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxGenericDirCtrlXmlHandler);

public:
    wxGenericDirCtrlXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_DIRDLG

#endif // _WX_XH_GDCTL_H_
