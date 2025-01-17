/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_richtext.h
// Purpose:     XML resource handler for wxRichTextCtrl
// Author:      Julian Smart
// Created:     2006-11-08
// Copyright:   (c) 2006 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_RICHTEXT_H_
#define _WX_XH_RICHTEXT_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_RICHTEXT

class wxRichTextCtrlXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextCtrlXmlHandler);

public:
    wxRichTextCtrlXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_RICHTEXT

#endif // _WX_XH_RICHTEXT_H_
