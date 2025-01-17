/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_split.h
// Purpose:     XRC resource for wxSplitterWindow
// Author:      panga@freemail.hu, Vaclav Slavik
// Created:     2003/01/26
// Copyright:   (c) 2003 panga@freemail.hu, Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_SPLIT_H_
#define _WX_XH_SPLIT_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_SPLITTER

class wxSplitterWindowXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxSplitterWindowXmlHandler);

public:
    wxSplitterWindowXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_SPLITTER

#endif // _WX_XH_SPLIT_H_
