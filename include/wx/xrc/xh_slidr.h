/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_slidr.h
// Purpose:     XML resource handler for wxSlider
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_SLIDR_H_
#define _WX_XH_SLIDR_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_SLIDER

class wxSliderXmlHandler : public wxXmlResourceHandler
{
public:
    wxSliderXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

    wxDECLARE_DYNAMIC_CLASS(wxSliderXmlHandler);
};

#endif // wxUSE_XRC && wxUSE_SLIDER

#endif // _WX_XH_SLIDR_H_
