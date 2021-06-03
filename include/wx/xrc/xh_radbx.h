/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_radbx.h
// Purpose:     XML resource handler for wxRadioBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_RADBX_H_
#define _WX_XH_RADBX_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_RADIOBOX

class WXDLLIMPEXP_XRC wxRadioBoxXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxRadioBoxXmlHandler);

public:
    wxRadioBoxXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_insideBox{false};

    // the items labels
    wxArrayString m_labels;

#if wxUSE_TOOLTIPS
    // the items tooltips
    wxArrayString m_tooltips;
#endif // wxUSE_TOOLTIPS

    // the item help text
    wxArrayString m_helptexts;
    std::vector<int>    m_helptextSpecified;

    // if the corresponding array element is 1, the radiobox item is
    // disabled/hidden
    std::vector<int> m_isEnabled;
    std::vector<int> m_isShown;
};

#endif // wxUSE_XRC && wxUSE_RADIOBOX

#endif // _WX_XH_RADBX_H_
