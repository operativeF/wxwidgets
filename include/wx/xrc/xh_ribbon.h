/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_ribbon.h
// Purpose:     XML resource handler for wxRibbon related classes
// Author:      Armel Asselin
// Created:     2010-04-23
// Copyright:   (c) 2010 Armel Asselin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XRC_XH_RIBBON_H_
#define _WX_XRC_XH_RIBBON_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_RIBBON

class wxRibbonControl;

class wxRibbonXmlHandler : public wxXmlResourceHandler
{
public:
    wxRibbonXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    const wxClassInfo *m_isInside{nullptr};

    bool IsRibbonControl (wxXmlNode *node);

    wxObject* Handle_buttonbar();
    wxObject* Handle_button();
    wxObject* Handle_control();
    wxObject* Handle_page();
    wxObject* Handle_gallery();
    wxObject* Handle_galleryitem();
    wxObject* Handle_panel();
    wxObject* Handle_bar();

    void Handle_RibbonArtProvider(wxRibbonControl *control);

    wxDECLARE_DYNAMIC_CLASS(wxRibbonXmlHandler);
};

#endif // wxUSE_XRC && wxUSE_RIBBON

#endif // _WX_XRC_XH_RIBBON_H_
