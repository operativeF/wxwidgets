/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_filepicker.h
// Purpose:     XML resource handler for wxFilePickerCtrl
// Author:      Francesco Montorsi
// Created:     2006-04-17
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_FILEPICKERCTRL_H_
#define _WX_XH_FILEPICKERCTRL_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_FILEPICKERCTRL

class wxFilePickerCtrlXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxFilePickerCtrlXmlHandler);

public:
    wxFilePickerCtrlXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_FILEPICKERCTRL

#endif // _WX_XH_FILEPICKERCTRL_H_
