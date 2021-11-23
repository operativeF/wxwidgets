/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/dcscreen.cpp
// Purpose:     wxScreenDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"
#include "wx/dcscreen.h"
#include "wx/msw/dcscreen.h"
#include "wx/window.h"

wxIMPLEMENT_ABSTRACT_CLASS(wxScreenDCImpl, wxMSWDCImpl);

// Create a DC representing the whole virtual screen (all monitors)
wxScreenDCImpl::wxScreenDCImpl( wxScreenDC *owner ) :
    wxMSWDCImpl( owner )
{
    m_hDC = (WXHDC) ::GetDC((WXHWND) nullptr);

    // the background mode is only used for text background and is set in
    // DrawText() to OPAQUE as required, otherwise always TRANSPARENT
    ::SetBkMode( GetHdc(), TRANSPARENT );
}

// Return the size of the whole virtual screen (all monitors)
wxSize wxScreenDCImpl::DoGetSize() const
{
    return {::GetSystemMetrics(SM_CXVIRTUALSCREEN), ::GetSystemMetrics(SM_CYVIRTUALSCREEN)};
}
