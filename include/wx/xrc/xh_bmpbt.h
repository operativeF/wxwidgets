/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_bmpbt.h
// Purpose:     XML resource handler for bitmap buttons
// Author:      Brian Gavin
// Created:     2000/03/05
// Copyright:   (c) 2000 Brian Gavin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_BMPBT_H_
#define _WX_XH_BMPBT_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_BMPBUTTON

class wxBitmapButtonXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxBitmapButtonXmlHandler);

public:
    wxBitmapButtonXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_BMPBUTTON

#endif // _WX_XH_BMPBT_H_
