/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_fontpicker.h
// Purpose:     XML resource handler for wxFontPickerCtrl
// Author:      Francesco Montorsi
// Created:     2006-04-17
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_FONTPICKERCTRL_H_
#define _WX_XH_FONTPICKERCTRL_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_FONTPICKERCTRL

class wxFontPickerCtrlXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxFontPickerCtrlXmlHandler);

public:
    wxFontPickerCtrlXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_FONTPICKERCTRL

#endif // _WX_XH_FONTPICKERCTRL_H_
