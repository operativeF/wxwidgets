/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_stbmp.cpp
// Purpose:     XRC resource for wxStaticBitmap
// Author:      Vaclav Slavik
// Created:     2000/04/22
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_STATBMP

#include "wx/xrc/xh_stbmp.h"
#include "wx/statbmp.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxStaticBitmapXmlHandler, wxXmlResourceHandler);

wxStaticBitmapXmlHandler::wxStaticBitmapXmlHandler()
                         
{
    AddWindowStyles();
}

wxObject *wxStaticBitmapXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(bmp, wxStaticBitmap)

    bmp->Create(m_parentAsWindow,
                GetID(),
                GetBitmap("bitmap", wxART_OTHER, GetSize()),
                GetPosition(), GetSize(),
                GetStyle(),
                GetName());

    SetupWindow(bmp);

    return bmp;
}

bool wxStaticBitmapXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxStaticBitmap");
}

#endif // wxUSE_XRC && wxUSE_STATBMP
