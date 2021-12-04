/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/printwin.h
// Purpose:     wxWindowsPrinter, wxWindowsPrintPreview classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRINTWIN_H_
#define _WX_PRINTWIN_H_

#include "wx/prntbase.h"

// ---------------------------------------------------------------------------
// Represents the printer: manages printing a wxPrintout object
// ---------------------------------------------------------------------------

class wxWindowsPrinter : public wxPrinterBase
{
public:
    wxWindowsPrinter(wxPrintDialogData *data = nullptr);

    wxWindowsPrinter& operator=(wxWindowsPrinter&&) = delete;

    bool Print(wxWindow *parent,
                       wxPrintout *printout,
                       bool prompt = true) override;

    wxDC *PrintDialog(wxWindow *parent) override;
    bool Setup(wxWindow *parent) override;
};

// ---------------------------------------------------------------------------
// wxPrintPreview: programmer creates an object of this class to preview a
// wxPrintout.
// ---------------------------------------------------------------------------

class wxWindowsPrintPreview : public wxPrintPreviewBase
{
public:
    wxWindowsPrintPreview(std::unique_ptr<wxPrintout> printout,
                          std::unique_ptr<wxPrintout> printoutForPrinting = {},
                          wxPrintDialogData *data = nullptr);
    wxWindowsPrintPreview(std::unique_ptr<wxPrintout> printout,
                          std::unique_ptr<wxPrintout> printoutForPrinting,
                          wxPrintData *data);

    wxWindowsPrintPreview& operator=(wxWindowsPrintPreview&&) = delete;

    bool Print(bool interactive) override;
    void DetermineScaling() override;

protected:
#if wxUSE_ENH_METAFILE
    bool RenderPageIntoBitmap(wxBitmap& bmp, int pageNum) override;
#endif
};

#endif
// _WX_PRINTWIN_H_
