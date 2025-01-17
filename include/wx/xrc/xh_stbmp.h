/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_stbmp.h
// Purpose:     XML resource handler for wxStaticBitmap
// Author:      Vaclav Slavik
// Created:     2000/04/22
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_STBMP_H_
#define _WX_XH_STBMP_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_STATBMP

class wxStaticBitmapXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxStaticBitmapXmlHandler);

public:
    wxStaticBitmapXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_STATBMP

#endif // _WX_XH_STBMP_H_
