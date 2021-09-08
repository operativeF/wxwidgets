/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/printwin.cpp
// Purpose:     wxWindowsPrinter framework
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


// Don't use the Windows printer if we're in wxUniv mode and using
// the PostScript architecture
#if wxUSE_PRINTING_ARCHITECTURE && (!defined(__WXUNIVERSAL__) || !wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW)

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include "wx/window.h"
    #include "wx/msw/private.h"
    #include "wx/utils.h"
    #include "wx/dc.h"
    #include "wx/app.h"
    #include "wx/msgdlg.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/dcprint.h"
    #include "wx/dcmemory.h"
    #include "wx/image.h"
    #include "wx/msw/private.h"
#endif

#include "wx/msw/dib.h"
#include "wx/msw/dcmemory.h"
#include "wx/msw/printwin.h"
#include "wx/msw/printdlg.h"
#include "wx/msw/dcprint.h"
#include "wx/msw/enhmeta.h"

// ---------------------------------------------------------------------------
// private functions
// ---------------------------------------------------------------------------

BOOL CALLBACK wxAbortProc(HDC hdc, int error);

// ---------------------------------------------------------------------------
// Printer
// ---------------------------------------------------------------------------

wxWindowsPrinter::wxWindowsPrinter(wxPrintDialogData *data)
                : wxPrinterBase(data)
{
}

bool wxWindowsPrinter::Print(wxWindow *parent, wxPrintout *printout, bool prompt)
{
    sm_abortIt = false;
    sm_abortWindow = nullptr;

    if (!printout)
    {
        sm_lastError = wxPrinterError::Error;
        return false;
    }

    if (m_printDialogData.GetMinPage() < 1)
        m_printDialogData.SetMinPage(1);
    if (m_printDialogData.GetMaxPage() < 1)
        m_printDialogData.SetMaxPage(9999);

    // Create a suitable device context
    wxPrinterDC *dc wxDUMMY_INITIALIZE(nullptr);
    if (prompt)
    {
        dc = dynamic_cast<wxPrinterDC*>(PrintDialog(parent));
        if (!dc)
            return false;
    }
    else
    {
        dc = new wxPrinterDC(m_printDialogData.GetPrintData());
    }

    // May have pressed cancel.
    if (!dc || !dc->IsOk())
    {
        if (dc) delete dc;
        return false;
    }

    // Set printout parameters
    if (!printout->SetUp(*dc))
    {
        delete dc;
        sm_lastError = wxPrinterError::Error;
        return false;
    }

    // Create an abort window
    wxBusyCursor busyCursor;

    printout->OnPreparePrinting();

    // Get some parameters from the printout, if defined
    int fromPage, toPage;
    int minPage, maxPage;
    printout->GetPageInfo(&minPage, &maxPage, &fromPage, &toPage);

    if (maxPage == 0)
    {
        sm_lastError = wxPrinterError::Error;
        return false;
    }

    // Only set min and max, because from and to have been
    // set by the user
    m_printDialogData.SetMinPage(minPage);
    m_printDialogData.SetMaxPage(maxPage);

    wxPrintAbortDialog *win = CreateAbortWindow(parent, printout);
    wxYield();

    ::SetAbortProc(GetHdcOf(*dc), wxAbortProc);

    if (!win)
    {
        wxLogDebug(wxT("Could not create an abort dialog."));
        sm_lastError = wxPrinterError::Error;

        delete dc;
        return false;
    }
    sm_abortWindow = win;
    sm_abortWindow->Show();
    wxSafeYield();

    printout->OnBeginPrinting();

    sm_lastError = wxPrinterError::NoError;

    int minPageNum = minPage, maxPageNum = maxPage;

    if ( !(m_printDialogData.GetAllPages() || m_printDialogData.GetSelection()) )
    {
        minPageNum = m_printDialogData.GetFromPage();
        maxPageNum = m_printDialogData.GetToPage();
    }

    // The dc we get from the PrintDialog will do multiple copies without help
    // if the device supports it. Loop only if we have created a dc from our
    // own m_printDialogData or the device does not support multiple copies.
    // m_printDialogData.GetPrintData().GetNoCopies() is set from device
    // devMode in printdlg.cpp/wxWindowsPrintDialog::ConvertFromNative()
    const int maxCopyCount = !prompt ||
                             !m_printDialogData.GetPrintData().GetNoCopies()
                             ? m_printDialogData.GetNoCopies() : 1;
    for ( int copyCount = 1; copyCount <= maxCopyCount; copyCount++ )
    {
        if ( !printout->OnBeginDocument(minPageNum, maxPageNum) )
        {
            wxLogError(_("Could not start printing."));
            sm_lastError = wxPrinterError::Error;
            break;
        }
        if (sm_abortIt)
        {
            sm_lastError = wxPrinterError::Cancelled;
            break;
        }

        for ( int pn = minPageNum;
              pn <= maxPageNum && printout->HasPage(pn);
              pn++ )
        {
            win->SetProgress(pn - minPageNum + 1,
                             maxPageNum - minPageNum + 1,
                             copyCount, maxCopyCount);

            if ( sm_abortIt )
            {
                sm_lastError = wxPrinterError::Cancelled;
                break;
            }

            dc->StartPage();
            bool cont = printout->OnPrintPage(pn);
            dc->EndPage();

            if ( !cont )
            {
                sm_lastError = wxPrinterError::Cancelled;
                break;
            }
        }

        printout->OnEndDocument();
    }

    printout->OnEndPrinting();

    if (sm_abortWindow)
    {
        sm_abortWindow->Show(false);
        wxDELETE(sm_abortWindow);
    }

    delete dc;

    return sm_lastError == wxPrinterError::NoError;
}

wxDC *wxWindowsPrinter::PrintDialog(wxWindow *parent)
{
    wxDC *dc = nullptr;

    wxWindowsPrintDialog dialog(parent, & m_printDialogData);
    int ret = dialog.ShowModal();

    if (ret == wxID_OK)
    {
        dc = dialog.GetPrintDC();
        m_printDialogData = dialog.GetPrintDialogData();
        if (dc == nullptr)
            sm_lastError = wxPrinterError::Error;
        else
            sm_lastError = wxPrinterError::NoError;
    }
    else
        sm_lastError = wxPrinterError::Cancelled;

    return dc;
}

bool wxWindowsPrinter::Setup(wxWindow *WXUNUSED(parent))
{
#if 0
    // We no longer expose that dialog
    wxPrintDialog dialog(parent, & m_printDialogData);
    dialog.GetPrintDialogData().SetSetupDialog(true);

    int ret = dialog.ShowModal();

    if (ret == wxID_OK)
    {
        m_printDialogData = dialog.GetPrintDialogData();
    }

    return (ret == wxID_OK);
#else
    return false;
#endif
}

/*
* Print preview
*/

wxWindowsPrintPreview::wxWindowsPrintPreview(wxPrintout *printout,
                                             wxPrintout *printoutForPrinting,
                                             wxPrintDialogData *data)
                     : wxPrintPreviewBase(printout, printoutForPrinting, data)
{
    DetermineScaling();
}

wxWindowsPrintPreview::wxWindowsPrintPreview(wxPrintout *printout,
                                             wxPrintout *printoutForPrinting,
                                             wxPrintData *data)
                     : wxPrintPreviewBase(printout, printoutForPrinting, data)
{
    DetermineScaling();
}

bool wxWindowsPrintPreview::Print(bool interactive)
{
    if (!m_printPrintout)
        return false;
    wxWindowsPrinter printer(&m_printDialogData);
    return printer.Print(m_previewFrame, m_printPrintout, interactive);
}

void wxWindowsPrintPreview::DetermineScaling()
{
    using msw::utils::unique_dcwnd;

    unique_dcwnd dc{::GetDC(nullptr)};

    int logPPIScreenX = ::GetDeviceCaps(dc.get(), LOGPIXELSX);
    int logPPIScreenY = ::GetDeviceCaps(dc.get(), LOGPIXELSY);

    m_previewPrintout->SetPPIScreen(logPPIScreenX, logPPIScreenY);

    // Get a device context for the currently selected printer
    wxPrinterDC printerDC(m_printDialogData.GetPrintData());

    int printerWidthMM;
    int printerHeightMM;
    int printerXRes;
    int printerYRes;
    int logPPIPrinterX;
    int logPPIPrinterY;

    wxRect paperRect;

    if ( printerDC.IsOk() )
    {
        wxPrinterDCImpl *impl = (wxPrinterDCImpl*) printerDC.GetImpl();
        HDC hdc = GetHdcOf(*impl);
        printerWidthMM = ::GetDeviceCaps(hdc, HORZSIZE);
        printerHeightMM = ::GetDeviceCaps(hdc, VERTSIZE);
        printerXRes = ::GetDeviceCaps(hdc, HORZRES);
        printerYRes = ::GetDeviceCaps(hdc, VERTRES);
        logPPIPrinterX = ::GetDeviceCaps(hdc, LOGPIXELSX);
        logPPIPrinterY = ::GetDeviceCaps(hdc, LOGPIXELSY);

        paperRect = printerDC.GetPaperRect();

        if ( logPPIPrinterX == 0 ||
                logPPIPrinterY == 0 ||
                    printerWidthMM == 0 ||
                        printerHeightMM == 0 )
        {
            m_isOk = false;
        }
    }
    else
    {
        // FIXME: Why not just default in the constructor?
        // use some defaults
        printerWidthMM = 150;
        printerHeightMM = 250;
        printerXRes = 1500;
        printerYRes = 2500;
        logPPIPrinterX = 600;
        logPPIPrinterY = 600;

        paperRect = wxRect(0, 0, printerXRes, printerYRes);
        m_isOk = false;
    }

    m_pageWidth = printerXRes;
    m_pageHeight = printerYRes;
    m_previewPrintout->SetPageSizePixels(printerXRes, printerYRes);
    m_previewPrintout->SetPageSizeMM(printerWidthMM, printerHeightMM);
    m_previewPrintout->SetPaperRectPixels(paperRect);
    m_previewPrintout->SetPPIPrinter(logPPIPrinterX, logPPIPrinterY);

    // At 100%, the page should look about page-size on the screen.
    m_previewScaleX = float(logPPIScreenX) / logPPIPrinterX;
    m_previewScaleY = float(logPPIScreenY) / logPPIPrinterY;
}

#if wxUSE_ENH_METAFILE
bool wxWindowsPrintPreview::RenderPageIntoBitmap(wxBitmap& bmp, int pageNum)
{
    // The preview, as implemented in wxPrintPreviewBase (and as used prior to
    // wx3) is inexact: it uses screen DC, which has much lower resolution and
    // has other properties different from printer DC, so the preview is not
    // quite right.
    //
    // To make matters worse, if the application depends heavily on
    // GetTextExtent() or does text layout itself, the output in preview and on
    // paper can be very different. In particular, wxHtmlEasyPrinting is
    // affected and the preview can be easily off by several pages.
    //
    // To fix this, we render the preview into high-resolution enhanced
    // metafile with properties identical to the printer DC. This guarantees
    // metrics correctness while still being fast.


    // print the preview into a metafile:
    wxPrinterDC printerDC(m_printDialogData.GetPrintData());
    wxEnhMetaFileDC metaDC(printerDC,
                           "",
                           printerDC.GetSize().x, printerDC.GetSize().y);

    if ( !RenderPageIntoDC(metaDC, pageNum) )
        return false;

    wxEnhMetaFile *metafile = metaDC.Close();
    if ( !metafile )
        return false;

    // now render the metafile:
    wxMemoryDC bmpDC;
    bmpDC.SelectObject(bmp);
    bmpDC.Clear();

    wxRect outRect(0, 0, bmp.GetWidth(), bmp.GetHeight());
    metafile->Play(&bmpDC, &outRect);


    delete metafile;

    // TODO: we should keep the metafile and reuse it when changing zoom level

    return true;
}
#endif // wxUSE_ENH_METAFILE

BOOL CALLBACK wxAbortProc(HDC WXUNUSED(hdc), int WXUNUSED(error))
{
    MSG msg;

    if (!wxPrinterBase::sm_abortWindow)              /* If the abort dialog isn't up yet */
        return(TRUE);

    /* Process messages intended for the abort dialog box */

    while (!wxPrinterBase::sm_abortIt && ::PeekMessage(&msg, nullptr, 0, 0, TRUE))
        if (!::IsDialogMessageW((HWND) wxPrinterBase::sm_abortWindow->GetHWND(), &msg)) {
            TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }

    /* bAbort is TRUE (return is FALSE) if the user has aborted */

    return !wxPrinterBase::sm_abortIt;
}

#endif
    // wxUSE_PRINTING_ARCHITECTURE
