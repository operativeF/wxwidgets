/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_sttxt.h
// Purpose:     XML resource handler for wxStaticText
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_STTXT_H_
#define _WX_XH_STTXT_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_STATTEXT

class wxStaticTextXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxStaticTextXmlHandler);

public:
    wxStaticTextXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_STATTEXT

#endif // _WX_XH_STTXT_H_
