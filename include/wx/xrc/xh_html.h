/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_html.h
// Purpose:     XML resource handler for wxHtmlWindow
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_HTML_H_
#define _WX_XH_HTML_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_HTML

class wxHtmlWindowXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlWindowXmlHandler);

public:
    wxHtmlWindowXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_HTML

#endif // _WX_XH_HTML_H_
