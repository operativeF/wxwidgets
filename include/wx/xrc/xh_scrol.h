/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_scrol.h
// Purpose:     XML resource handler for wxScrollBar
// Author:      Brian Gavin
// Created:     2000/09/09
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_SCROL_H_
#define _WX_XH_SCROL_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_SCROLLBAR

class wxScrollBarXmlHandler : public wxXmlResourceHandler
{
public:
    wxScrollBarXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

    wxDECLARE_DYNAMIC_CLASS(wxScrollBarXmlHandler);
};

#endif // wxUSE_XRC && wxUSE_SCROLLBAR

#endif // _WX_XH_SCROL_H_
