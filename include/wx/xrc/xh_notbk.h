/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_notbk.h
// Purpose:     XML resource handler for wxNotebook
// Author:      Vaclav Slavik
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_NOTBK_H_
#define _WX_XH_NOTBK_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_NOTEBOOK

class wxNotebook;

class wxNotebookXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxNotebookXmlHandler);

public:
    wxNotebookXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

private:
    bool m_isInside{false};
    wxNotebook *m_notebook{nullptr};
};

#endif // wxUSE_XRC && wxUSE_NOTEBOOK

#endif // _WX_XH_NOTBK_H_
