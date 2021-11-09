/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_toolb.h
// Purpose:     XML resource handler for wxAuiToolBar
// Author:      Rodolphe Suescun
// Created:     2013-11-23
// Copyright:   (c) 2013 Rodolphe Suescun
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_AUITOOLB_H_
#define _WX_XH_AUITOOLB_H_

#include "wx/aui/auibar.h"
#include "wx/menu.h"
import <vector>;
#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_AUI

class wxAuiToolBar;

class wxAuiToolBarXmlHandler : public wxXmlResourceHandler
{
public:
    wxAuiToolBarXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_isInside{false};
    wxAuiToolBar *m_toolbar{nullptr};
    wxSize m_toolSize;

    class MenuHandler : public wxEvtHandler
    {
    public:
        void OnDropDown(wxAuiToolBarEvent& event);
        unsigned RegisterMenu(wxAuiToolBar *toobar, int id, wxMenu *menu);

    private:
        std::vector<wxMenu*> m_menus;
    };

    MenuHandler m_menuHandler;

    wxDECLARE_DYNAMIC_CLASS(wxAuiToolBarXmlHandler);
};

#endif // wxUSE_XRC && wxUSE_AUI

#endif // _WX_XH_AUITOOLB_H_
