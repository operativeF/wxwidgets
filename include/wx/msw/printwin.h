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

class WXDLLIMPEXP_CORE wxWindowsPrinter : public wxPrinterBase
{
    wxDECLARE_DYNAMIC_CLASS(wxWindowsPrinter);

public:
    wxWindowsPrinter(wxPrintDialogData *data = nullptr) noexcept;

    wxWindowsPrinter(const wxWindowsPrinter&) = delete;
    wxWindowsPrinter& operator=(const wxWindowsPrinter&) = delete;
    wxWindowsPrinter(wxWindowsPrinter&&) = default;
    wxWindowsPrinter& operator=(wxWindowsPrinter&&) = default;

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

class WXDLLIMPEXP_CORE wxWindowsPrintPreview : public wxPrintPreviewBase
{
public:
    wxWindowsPrintPreview(wxPrintout *printout,
                          wxPrintout *printoutForPrinting = nullptr,
                          wxPrintDialogData *data = nullptr);
    wxWindowsPrintPreview(wxPrintout *printout,
                          wxPrintout *printoutForPrinting,
                          wxPrintData *data);
    ~wxWindowsPrintPreview() override = default;

    wxWindowsPrintPreview(const wxWindowsPrintPreview&) = delete;
    wxWindowsPrintPreview& operator=(const wxWindowsPrintPreview&) = delete;
    wxWindowsPrintPreview(wxWindowsPrintPreview&&) = default;
    wxWindowsPrintPreview& operator=(wxWindowsPrintPreview&&) = default;

    bool Print(bool interactive) override;
    void DetermineScaling() override;

protected:
#if wxUSE_ENH_METAFILE
    bool RenderPageIntoBitmap(wxBitmap& bmp, int pageNum) override;
#endif
};

#endif
// _WX_PRINTWIN_H_
