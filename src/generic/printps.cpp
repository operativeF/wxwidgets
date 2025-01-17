/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/printps.cpp
// Purpose:     Postscript print/preview framework
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_PRINTING_ARCHITECTURE && wxUSE_POSTSCRIPT && (!defined(__WXMSW__) || wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW)

#include "wx/utils.h"
#include "wx/dc.h"
#include "wx/app.h"
#include "wx/msgdlg.h"
#include "wx/intl.h"
#include "wx/progdlg.h"
#include "wx/log.h"
#include "wx/dcprint.h"

#include "wx/generic/printps.h"
#include "wx/printdlg.h"
#include "wx/generic/prntdlgg.h"
#include "wx/progdlg.h"
#include "wx/paper.h"

bool wxPostScriptPrinter::Print(wxWindow *parent, wxPrintout *printout, bool prompt)
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
    wxDC *dc;
    if (prompt)
    {
        dc = PrintDialog(parent);
        if (!dc)
            return false;
    }
    else
    {
        dc = new wxPostScriptDC(GetPrintDialogData().GetPrintData());
    }

    // May have pressed cancel.
    if (!dc || !dc->IsOk())
    {
        delete dc;
        sm_lastError = wxPrinterError::Error;
        return false;
    }

    // Set printout parameters
    printout->SetUp(*dc);

    // Create an abort window
    wxBeginBusyCursor();

    printout->OnPreparePrinting();

    // Get some parameters from the printout, if defined
    int fromPage, toPage;
    int minPage, maxPage;
    printout->GetPageInfo(&minPage, &maxPage, &fromPage, &toPage);

    if (maxPage == 0)
    {
        sm_lastError = wxPrinterError::Error;
        wxEndBusyCursor();
        return false;
    }

    // Only set min and max, because from and to have been
    // set by the user
    m_printDialogData.SetMinPage(minPage);
    m_printDialogData.SetMaxPage(maxPage);

    if (m_printDialogData.GetFromPage() < minPage)
        m_printDialogData.SetFromPage( minPage );
    if (m_printDialogData.GetToPage() > maxPage)
        m_printDialogData.SetToPage( maxPage );

    int
       pagesPerCopy = m_printDialogData.GetToPage()-m_printDialogData.GetFromPage()+1,
       totalPages = pagesPerCopy * m_printDialogData.GetNoCopies(),
       printedPages = 0;
    // Open the progress bar dialog
    wxProgressDialog *progressDialog = new wxProgressDialog (
       printout->GetTitle(),
       _("Printing..."),
       totalPages,
       parent,
       wxPD_CAN_ABORT|wxPD_AUTO_HIDE|wxPD_APP_MODAL);

    printout->OnBeginPrinting();

    sm_lastError = wxPrinterError::NoError;

    bool keepGoing = true;

    int copyCount;
    for (copyCount = 1; copyCount <= m_printDialogData.GetNoCopies(); copyCount ++)
    {
        if (!printout->OnBeginDocument(m_printDialogData.GetFromPage(), m_printDialogData.GetToPage()))
        {
            wxEndBusyCursor();
            wxLogError(_("Could not start printing."));
            sm_lastError = wxPrinterError::Error;
            break;
        }
        if (sm_abortIt)
        {
            sm_lastError = wxPrinterError::Cancelled;
            break;
        }

        int pn;
        for (pn = m_printDialogData.GetFromPage(); keepGoing && (pn <= m_printDialogData.GetToPage()) && printout->HasPage(pn);
        pn++)
        {
            if (sm_abortIt)
            {
                keepGoing = false;
                sm_lastError = wxPrinterError::Cancelled;
                break;
            }
            else
            {
               wxString msg;
               msg.Printf(_("Printing page %d..."), printedPages+1);
               if(progressDialog->Update(printedPages++, msg))
               {
                  dc->StartPage();
                  printout->OnPrintPage(pn);
                  dc->EndPage();
               }
               else
               {
                  sm_abortIt = true;
                  sm_lastError = wxPrinterError::Cancelled;
                  keepGoing = false;
               }
            }
            wxYield();
        }
        printout->OnEndDocument();
    }

    printout->OnEndPrinting();
    delete progressDialog;

    wxEndBusyCursor();

    delete dc;

    return (sm_lastError == wxPrinterError::NoError);
}

wxDC* wxPostScriptPrinter::PrintDialog(wxWindow *parent)
{
    wxDC* dc = nullptr;

    wxGenericPrintDialog dialog( parent, &m_printDialogData );
    if (dialog.ShowModal() == wxID_OK)
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

bool wxPostScriptPrinter::Setup([[maybe_unused]] wxWindow *parent)
{
#if 0
    wxGenericPrintDialog* dialog = new wxGenericPrintDialog(parent, & m_printDialogData);
    dialog->GetPrintDialogData().SetSetupDialog(true);

    int ret = dialog->ShowModal();

    if (ret == wxID_OK)
    {
        m_printDialogData = dialog->GetPrintDialogData();
    }

    dialog->Destroy();

    return (ret == wxID_OK);
#endif

    return false;
}

// ----------------------------------------------------------------------------
// Print preview
// ----------------------------------------------------------------------------

void wxPostScriptPrintPreview::Init([[maybe_unused]] wxPrintout * printout,
                                    [[maybe_unused]] wxPrintout * printoutForPrinting)
{
    // Have to call it here since base constructor can't call it
    DetermineScaling();
}

wxPostScriptPrintPreview::wxPostScriptPrintPreview(wxPrintout *printout,
                                                   wxPrintout *printoutForPrinting,
                                                   wxPrintDialogData *data)
                        : wxPrintPreviewBase(printout, printoutForPrinting, data)
{
    Init(printout, printoutForPrinting);
}

wxPostScriptPrintPreview::wxPostScriptPrintPreview(wxPrintout *printout,
                                                   wxPrintout *printoutForPrinting,
                                                   wxPrintData *data)
                        : wxPrintPreviewBase(printout, printoutForPrinting, data)
{
    Init(printout, printoutForPrinting);
}

bool wxPostScriptPrintPreview::Print(bool interactive)
{
    if (!m_printPrintout)
        return false;

    // Assume that on Unix, the preview may use the PostScript
    // (generic) version, but printing using the native system is required.
    // TODO: make a generic print preview class from which wxPostScriptPrintPreview
    // is derived.
#ifdef __UNIX__
    wxPrinter printer(& m_printDialogData);
#else
    wxPostScriptPrinter printer(& m_printDialogData);
#endif
    return printer.Print(m_previewFrame, m_printPrintout, interactive);
}

void wxPostScriptPrintPreview::DetermineScaling()
{
    wxPaperSize paperType = m_printDialogData.GetPrintData().GetPaperId();
    if (paperType == wxPaperSize::None)
        paperType = wxPaperSize::None;

    wxPrintPaperType *paper = wxThePrintPaperDatabase->FindPaperType(paperType);
    if (!paper)
        paper = wxThePrintPaperDatabase->FindPaperType(wxPaperSize::A4);

    if (paper)
    {
        int resolution = 600;  // TODO, this is correct, but get this from wxPSDC somehow

        const wxSize screenPPI = wxGetDisplayPPI();
        int logPPIScreenX = screenPPI.GetWidth();
        int logPPIScreenY = screenPPI.GetHeight();
        int logPPIPrinterX = resolution;
        int logPPIPrinterY = resolution;

        m_previewPrintout->SetPPIScreen( logPPIScreenX, logPPIScreenY );
        m_previewPrintout->SetPPIPrinter( logPPIPrinterX, logPPIPrinterY );

        wxSize sizeDevUnits(paper->GetSizeDeviceUnits());
        sizeDevUnits.x = sizeDevUnits.x * resolution / 72;
        sizeDevUnits.y = sizeDevUnits.y * resolution / 72;
        wxSize sizeTenthsMM(paper->GetSize());
        wxSize sizeMM(sizeTenthsMM.x / 10, sizeTenthsMM.y / 10);

        // If in landscape mode, we need to swap the width and height.
        if ( m_printDialogData.GetPrintData().GetOrientation() == wxPrintOrientation::Landscape )
        {
            m_pageWidth = sizeDevUnits.y;
            m_pageHeight = sizeDevUnits.x;
            m_previewPrintout->SetPageSizeMM(sizeMM.y, sizeMM.x);
        }
        else
        {
            m_pageWidth = sizeDevUnits.x;
            m_pageHeight = sizeDevUnits.y;
            m_previewPrintout->SetPageSizeMM(sizeMM.x, sizeMM.y);
        }
        m_previewPrintout->SetPageSizePixels(m_pageWidth, m_pageHeight);
        m_previewPrintout->SetPaperRectPixels(wxRect(0, 0, m_pageWidth, m_pageHeight));

        // At 100%, the page should look about page-size on the screen.
        m_previewScaleX = float(logPPIScreenX) / logPPIPrinterX;
        m_previewScaleY = float(logPPIScreenY) / logPPIPrinterY;
    }
}

#endif
