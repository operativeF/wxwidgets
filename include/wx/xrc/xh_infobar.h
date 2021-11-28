/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_infobar.h
// Purpose:     XML resource handler for wxInfoBar
// Author:      Ilya Sinitsyn
// Created:     2019-09-25
// Copyright:   (c) 2019 TT-Solutions SARL
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_INFOBAR_H_
#define _WX_XH_INFOBAR_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_INFOBAR

class wxInfoBarXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxInfoBarXmlHandler);

public:
    wxInfoBarXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    wxShowEffect GetShowEffect(wxString const& param);

    bool m_insideBar{false};

    // FIXME: Don't do this.
    wxString m_effectNames[static_cast<int>(wxShowEffect::Max)];
};

#endif // wxUSE_XRC && wxUSE_INFOBAR

#endif // _WX_XH_INFOBAR_H_
