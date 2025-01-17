/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_stbox.h
// Purpose:     XML resource handler for wxStaticBox
// Author:      Brian Gavin
// Created:     2000/09/00
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_STBOX_H_
#define _WX_XH_STBOX_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_STATBOX

class wxStaticBoxXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxStaticBoxXmlHandler);

public:
    wxStaticBoxXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_STATBOX

#endif // _WX_XH_STBOX_H_
