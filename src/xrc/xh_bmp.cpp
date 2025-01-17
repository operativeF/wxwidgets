/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_bmp.cpp
// Purpose:     XRC resource for wxBitmap and wxIcon
// Author:      Vaclav Slavik
// Created:     2000/09/09
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC

#include "wx/xrc/xh_bmp.h"

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
#endif

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapXmlHandler, wxXmlResourceHandler);

wxObject *wxBitmapXmlHandler::DoCreateResource()
{
    return new wxBitmap(GetBitmap(m_node));
}

bool wxBitmapXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxBitmap");
}

wxIMPLEMENT_DYNAMIC_CLASS(wxIconXmlHandler, wxXmlResourceHandler);

wxObject *wxIconXmlHandler::DoCreateResource()
{
    return new wxIcon(GetIcon(m_node));
}

bool wxIconXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxIcon");
}

#endif // wxUSE_XRC
