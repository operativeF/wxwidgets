/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dfb/dcscreen.h
// Purpose:     wxScreenDCImpl declaration
// Author:      Vaclav Slavik
// Created:     2006-08-10
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DFB_DCSCREEN_H_
#define _WX_DFB_DCSCREEN_H_

#include "wx/dfb/dc.h"

class wxScreenDCImpl : public wxDFBDCImpl
{
public:
    wxScreenDCImpl(wxScreenDC *owner);

    wxDECLARE_DYNAMIC_CLASS(wxScreenDCImpl);
};

#endif // _WX_DFB_DCSCREEN_H_
