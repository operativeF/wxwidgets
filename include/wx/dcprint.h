/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcprint.h
// Purpose:     wxPrinterDC base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCPRINT_H_BASE_
#define _WX_DCPRINT_H_BASE_

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/defs.h"

#include "wx/dc.h"

//-----------------------------------------------------------------------------
// wxPrinterDC
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrinterDC : public wxDC
{
public:
    wxPrinterDC();
    wxPrinterDC(const wxPrintData& data);

    wxRect GetPaperRect() const;
    int GetResolution() const override;

protected:
    wxPrinterDC(wxDCImpl *impl) : wxDC(impl) { }

private:
    wxDECLARE_DYNAMIC_CLASS(wxPrinterDC);
};

#endif // wxUSE_PRINTING_ARCHITECTURE

#endif // _WX_DCPRINT_H_BASE_
