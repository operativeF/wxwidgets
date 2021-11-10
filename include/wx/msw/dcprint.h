/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dcprint.h
// Purpose:     wxPrinterDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_DCPRINT_H_
#define _WX_MSW_DCPRINT_H_

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/dcprint.h"
#include "wx/cmndata.h"
#include "wx/msw/dc.h"

import Utils.Geometry;

import <string>;

class wxPrintData;

// ------------------------------------------------------------------------
//    wxPrinterDCImpl
//

class wxPrinterDCImpl : public wxMSWDCImpl
{
public:
    // Create from print data
    wxPrinterDCImpl( wxPrinterDC *owner, const wxPrintData& data );
    wxPrinterDCImpl( wxPrinterDC *owner, WXHDC theDC );

    wxPrinterDCImpl& operator=(wxPrinterDCImpl&&) = delete;

    // override some base class virtuals
    bool wxStartDoc(const std::string& message) override;
    void EndDoc() override;
    void StartPage() override;
    void EndPage() override;

    wxRect GetPaperRect() const override;

protected:
    void DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                              bool useMask = false) override;
    bool DoBlit(wxCoord xdest, wxCoord ydest,
                        wxCoord width, wxCoord height,
                        wxDC *source, wxPoint src,
                        wxRasterOperationMode rop = wxRasterOperationMode::Copy, bool useMask = false,
                        wxPoint srcMask = wxDefaultPosition) override;
    wxSize DoGetSize() const override
    {
        return GetDeviceSize();
    }

private:
    wxPrintData m_printData;

    wxDECLARE_CLASS(wxPrinterDCImpl);
};

// Gets an HDC for the specified printer configuration
WXHDC wxGetPrinterDC(const wxPrintData& data);

// ------------------------------------------------------------------------
//    wxPrinterDCromHDC
//

struct wxPrinterDCFromHDC: public wxPrinterDC
{
    wxPrinterDCFromHDC( WXHDC theDC )
        : wxPrinterDC(std::make_unique<wxPrinterDCImpl>(this, theDC))
    {
    }
};

#endif // wxUSE_PRINTING_ARCHITECTURE

#endif // _WX_MSW_DCPRINT_H_

