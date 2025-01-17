/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/mimetype.h
// Purpose:     classes and functions to manage MIME types
// Author:      Hans Mackowiak
// Created:     2016-06-05
// Copyright:   (c) 2016 Hans Mackowiak <hanmac@gmx.de>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_MIMETYPE_IMPL_H
#define _WX_GTK_MIMETYPE_IMPL_H

#include "wx/defs.h"

#if defined(__UNIX__)
#include "wx/unix/mimetype.h"
#elif defined(WX_WINDOWS)
#include "wx/msw/mimetype.h"
#endif

#if wxUSE_MIMETYPE

class wxGTKMimeTypesManagerImpl : public wxMimeTypesManagerImpl
{
protected:
#if defined(__UNIX__)
    wxString GetIconFromMimeType(const wxString& mime) override;
#endif
};


class wxGTKMimeTypesManagerFactory : public wxMimeTypesManagerFactory
{
public:
    wxMimeTypesManagerImpl *CreateMimeTypesManagerImpl() override;
};

#endif // wxUSE_MIMETYPE

#endif // _WX_GTK_MIMETYPE_IMPL_H
