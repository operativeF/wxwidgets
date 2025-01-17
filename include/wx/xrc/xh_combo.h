/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_combo.h
// Purpose:     XML resource handler for wxComboBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_COMBO_H_
#define _WX_XH_COMBO_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_COMBOBOX

class wxComboBoxXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxComboBoxXmlHandler);

public:
    wxComboBoxXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_insideBox{false};
    std::vector<std::string> strList;
};

#endif // wxUSE_XRC && wxUSE_COMBOBOX

#endif // _WX_XH_COMBO_H_
