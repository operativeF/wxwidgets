/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk1/dcscreen.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __GTKDCSCREENH__
#define __GTKDCSCREENH__

#include "wx/gtk1/dcclient.h"

//-----------------------------------------------------------------------------
// wxScreenDCImpl
//-----------------------------------------------------------------------------

class wxScreenDCImpl : public wxPaintDCImpl
{
public:
    wxScreenDCImpl(wxScreenDC *owner);
    virtual ~wxScreenDCImpl();

    // implementation

    static GdkWindow  *sm_overlayWindow;
    static int         sm_overlayWindowX;
    static int         sm_overlayWindowY;

protected:
    virtual wxSize DoGetSize() const;

private:
    wxDECLARE_DYNAMIC_CLASS(wxScreenDCImpl);
};

#endif // __GTKDCSCREENH__

