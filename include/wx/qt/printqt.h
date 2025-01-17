/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/printqt.h
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_PRINTQT_H_
#define _WX_QT_PRINTQT_H_

#include "wx/prntbase.h"

class wxQtPrinter : public wxPrinterBase
{
public:
    wxQtPrinter( wxPrintDialogData *data = NULL );

    bool Setup(wxWindow *parent) override;
    bool Print(wxWindow *parent, wxPrintout *printout, bool prompt = true) override;
    wxDC* PrintDialog(wxWindow *parent) override;
private:
};



class wxQtPrintPreview : public wxPrintPreviewBase
{
public:
    wxQtPrintPreview(wxPrintout *printout,
                          wxPrintout *printoutForPrinting = NULL,
                          wxPrintDialogData *data = NULL);
    wxQtPrintPreview(wxPrintout *printout,
                          wxPrintout *printoutForPrinting,
                          wxPrintData *data);

    bool Print(bool interactive) override;
    void DetermineScaling() override;

protected:
};

#endif // _WX_QT_PRINTQT_H_
