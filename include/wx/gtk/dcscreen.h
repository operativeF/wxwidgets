/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/dcscreen.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTKDCSCREEN_H_
#define _WX_GTKDCSCREEN_H_

#include "wx/dcscreen.h"
#include "wx/gtk/dcclient.h"

//-----------------------------------------------------------------------------
// wxScreenDCImpl
//-----------------------------------------------------------------------------

class wxScreenDCImpl : public wxWindowDCImpl
{
public:
    wxScreenDCImpl( wxScreenDC *owner );
    ~wxScreenDCImpl();

    wxSize DoGetSize() const override;

private:
    void Init();

    wxDECLARE_ABSTRACT_CLASS(wxScreenDCImpl);
};

#endif // _WX_GTKDCSCREEN_H_
