/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/graphics.cpp
// Purpose:     wxGCDC class
// Author:      Stefan Csomor
// Modified by:
// Created:     2006-09-30
// Copyright:   (c) 2006 Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/wrapcdlg.h"
#include "wx/msw/private.h" // needs to be before #include <commdlg.h>

#include "wx/dc.h"
#include "wx/image.h"
#include "wx/window.h"
#include "wx/utils.h"
#include "wx/bitmap.h"
#include "wx/log.h"
#include "wx/icon.h"
#include "wx/module.h"
// include all dc types that are used as a param
#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/dcprint.h"

#include "wx/private/graphics.h"
#include "wx/msw/wrapgdip.h"
#include "wx/msw/dc.h"
#include "wx/msw/enhmeta.h"
#include "wx/dcgraph.h"
#include "wx/rawbmp.h"

// FIXME: Already defined?
#if wxUSE_COMMON_DIALOGS
#include <commdlg.h>
#endif

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

// ----------------------------------------------------------------------------
// wxMSW-specific parts of wxGCDC
// ----------------------------------------------------------------------------

wxGraphicsRenderer* wxGraphicsRenderer::GetDefaultRenderer()
{
    return wxGraphicsRenderer::GetDirect2DRenderer();
}

WXHDC wxGCDC::AcquireHDC()
{
    wxGraphicsContext * const gc = GetGraphicsContext();
    if ( !gc )
        return nullptr;

#if wxUSE_CAIRO
    // we can't get the WXHDC if it is not a GDI+ context
    wxGraphicsRenderer* r1 = gc->GetRenderer();
    wxGraphicsRenderer* r2 = wxGraphicsRenderer::GetCairoRenderer();
    if (r1 == r2)
        return NULL;
#endif

    Graphics * const g = static_cast<Graphics *>(gc->GetNativeContext());
    return g ? g->GetHDC() : nullptr;
}

void wxGCDC::ReleaseHDC(WXHDC hdc)
{
    if ( !hdc )
        return;

    wxGraphicsContext * const gc = GetGraphicsContext();
    wxCHECK_RET( gc, "can't release WXHDC because there is no wxGraphicsContext" );

#if wxUSE_CAIRO
    // we can't get the WXHDC if it is not a GDI+ context
    wxGraphicsRenderer* r1 = gc->GetRenderer();
    wxGraphicsRenderer* r2 = wxGraphicsRenderer::GetCairoRenderer();
    if (r1 == r2)
        return;
#endif

    Graphics * const g = static_cast<Graphics *>(gc->GetNativeContext());
    wxCHECK_RET( g, "can't release WXHDC because there is no Graphics" );

    g->ReleaseHDC((WXHDC)hdc);
}
