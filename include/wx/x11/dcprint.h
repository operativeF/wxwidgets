/////////////////////////////////////////////////////////////////////////////
// Name:        wx/x11/dcprint.h
// Purpose:     wxPrinterDC class
// Author:      Julian Smart
// Modified by:
// Created:     17/09/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCPRINT_H_
#define _WX_DCPRINT_H_

#include "wx/dc.h"

class wxPrinterDC: public wxDC
{
public:
    wxDECLARE_CLASS(wxPrinterDC);

    // Create a printer DC
    wxPrinterDC(const wxString& driver, const wxString& device, const wxString& output, bool interactive = TRUE, wxPrintOrientation orientation = wxPrintOrientation::Portrait);

    virtual ~wxPrinterDC();
};

#endif
    // _WX_DCPRINT_H_
