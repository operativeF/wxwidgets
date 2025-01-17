/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_grid.h
// Purpose:     XML resource handler for wxGrid
// Author:      Agron Selimaj
// Created:     2005/08/11
// Copyright:   (c) 2005 Agron Selimaj, Freepour Controls Inc.
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_GRD_H_
#define _WX_XH_GRD_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_GRID

class wxGridXmlHandler : public wxXmlResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxGridXmlHandler);

public:
    wxGridXmlHandler();

    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;
};

#endif // wxUSE_XRC && wxUSE_GRID

#endif // _WX_XH_GRD_H_
