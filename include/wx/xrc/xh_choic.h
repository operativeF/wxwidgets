/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_choic.h
// Purpose:     XML resource handler for wxChoice
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_CHOIC_H_
#define _WX_XH_CHOIC_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_CHOICE

class wxChoiceXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxChoiceXmlHandler);

public:
    wxChoiceXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_insideBox{false};
    std::vector<std::string> strList;
};

#endif // wxUSE_XRC && wxUSE_CHOICE

#endif // _WX_XH_CHOIC_H_
