/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_choicbk.h
// Purpose:     XML resource handler for wxChoicebook
// Author:      Vaclav Slavik
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_CHOICEBK_H_
#define _WX_XH_CHOICEBK_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_CHOICEBOOK

class wxChoicebook;

class WXDLLIMPEXP_XRC wxChoicebookXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxChoicebookXmlHandler);

public:
    wxChoicebookXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_isInside{false};
    wxChoicebook *m_choicebook{nullptr};
};

#endif // wxUSE_XRC && wxUSE_CHOICEBOOK

#endif // _WX_XH_CHOICEBK_H_
