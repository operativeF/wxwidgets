/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/dcscreen.h
// Purpose:     wxScreenDC class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCSCREEN_H_
#define _WX_DCSCREEN_H_

#include "wx/dcclient.h"
#include "wx/osx/dcclient.h"

class wxScreenDCImpl: public wxWindowDCImpl
{
public:
    wxScreenDCImpl( wxDC *owner );
    virtual ~wxScreenDCImpl();

    wxBitmap DoGetAsBitmap(const wxRect *subrect) const override;

private:
    wxDECLARE_CLASS(wxScreenDCImpl);
    wxScreenDCImpl(const wxScreenDCImpl&) = delete;
	wxScreenDCImpl& operator=(const wxScreenDCImpl&) = delete;
};

#endif
    // _WX_DCSCREEN_H_

