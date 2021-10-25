/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dcscreen.h
// Purpose:     wxScreenDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DCSCREEN_H_
#define _WX_MSW_DCSCREEN_H_

#include "wx/msw/dc.h"

class WXDLLIMPEXP_FWD_CORE wxScreenDC;

struct WXDLLIMPEXP_CORE wxScreenDCImpl : public wxMSWDCImpl
{
    // Create a DC representing the whole virtual screen (all monitors)
    wxScreenDCImpl( wxScreenDC *owner );

    wxScreenDCImpl& operator=(wxScreenDCImpl&&) = delete;

    // Return the size of the whole virtual screen (all monitors)
    wxSize DoGetSize() const override;

    wxDECLARE_CLASS(wxScreenDCImpl);
};

#endif // _WX_MSW_DCSCREEN_H_

