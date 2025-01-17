/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_odcombo.h
// Purpose:     XML resource handler for wxOwnerDrawnComboBox
// Author:      Alex Bligh - based on wx/xrc/xh_combo.h
// Created:     2006/06/19
// Copyright:   (c) 2006 Alex Bligh
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_ODCOMBO_H_
#define _WX_XH_ODCOMBO_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_ODCOMBOBOX

class wxOwnerDrawnComboBoxXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxOwnerDrawnComboBoxXmlHandler);

public:
    wxOwnerDrawnComboBoxXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_insideBox{false};
    std::vector<std::string> strList;
};

#endif // wxUSE_XRC && wxUSE_ODCOMBOBOX

#endif // _WX_XH_ODCOMBO_H_

