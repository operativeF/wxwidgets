/////////////////////////////////////////////////////////////////////////////
// Name:        wx/x11/dcscreen.h
// Purpose:     wxScreenDC class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCSCREEN_H_
#define _WX_DCSCREEN_H_

#include "wx/dcclient.h"
#include "wx/x11/dcclient.h"

//-----------------------------------------------------------------------------
// wxScreenDC
//-----------------------------------------------------------------------------

class wxScreenDCImpl : public wxPaintDCImpl
{
public:
    wxScreenDCImpl( wxDC *owner);
    virtual ~wxScreenDCImpl();

protected:
    virtual wxSize DoGetSize() const;

private:
    wxDECLARE_CLASS(wxScreenDCImpl);
};


#endif
    // _WX_DCSCREEN_H_
