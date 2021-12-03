/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_grid.cpp
// Purpose:     XML resource handler for the grid control
// Author:      Agron Selimaj
// Created:     2005/08/11
// Copyright:   (c) 2005 Agron Selimaj, Freepour Controls Inc.
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_GRID

#include "wx/xrc/xh_grid.h"

import WX.Grid;

wxIMPLEMENT_DYNAMIC_CLASS(wxGridXmlHandler, wxXmlResourceHandler);

wxGridXmlHandler::wxGridXmlHandler()
                 
{
    AddWindowStyles();
}

wxObject *wxGridXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(grid, wxGrid)

    grid->Create(m_parentAsWindow,
                   GetID(),
                   GetPosition(), GetSize(),
                   GetStyle("style"),
                   GetName());

    SetupWindow( grid);

    return grid;
}

bool wxGridXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxGrid");
}

#endif // wxUSE_XRC && wxUSE_GRID
