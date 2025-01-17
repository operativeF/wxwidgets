/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_frame.h
// Purpose:     XML resource handler for wxFrame
// Author:      Vaclav Slavik & Aleks.
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_FRAME_H_
#define _WX_XH_FRAME_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC

class wxFrameXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxFrameXmlHandler);

public:
    wxFrameXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC

#endif // _WX_XH_FRAME_H_
