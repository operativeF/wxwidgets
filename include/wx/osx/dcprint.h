/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/dcprint.h
// Purpose:     wxPrinterDC class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCPRINT_H_
#define _WX_DCPRINT_H_

#include "wx/dc.h"
#include "wx/dcgraph.h"
#include "wx/cmndata.h"

class wxNativePrinterDC ;

class wxPrinterDCImpl: public wxGCDCImpl
{
public:
#if wxUSE_PRINTING_ARCHITECTURE

    wxPrinterDCImpl( wxPrinterDC *owner, const wxPrintData& printdata );
    virtual ~wxPrinterDCImpl();

    bool StartDoc( const wxString& WXUNUSED(message) ) override;
    void EndDoc() override;
    void StartPage() override;
    void EndPage() override;

    wxRect GetPaperRect() const override;

    wxPrintData& GetPrintData() { return m_printData; }
    wxSize GetPPI() const override;

protected:
    wxSize DoGetSize() const override;

    wxPrintData        m_printData ;
    wxNativePrinterDC* m_nativePrinterDC ;

private:
    wxDECLARE_CLASS(wxPrinterDC);
#endif // wxUSE_PRINTING_ARCHITECTURE
};

#endif
    // _WX_DCPRINT_H_

