/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/printdlg.h
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_PRINTDLG_H_
#define _WX_QT_PRINTDLG_H_

#include "wx/prntbase.h"
#include "wx/printdlg.h"

class wxQtPrintNativeData: public wxPrintNativeDataBase
{
public:
    wxQtPrintNativeData();

    bool TransferTo( wxPrintData &data ) override;
    bool TransferFrom( const wxPrintData &data ) override;

    bool IsOk() const override;

};

class wxQtPrintDialog : public wxPrintDialogBase
{
public:
    wxQtPrintDialog(wxWindow *parent, wxPrintDialogData *data);
    wxQtPrintDialog(wxWindow *parent, wxPrintData *data);

    wxPrintDialogData& GetPrintDialogData() override;
    wxPrintData& GetPrintData() override;
    wxDC *GetPrintDC() override;

protected:

private:
};



class wxQtPageSetupDialog: public wxPageSetupDialogBase
{
public:
    wxQtPageSetupDialog();
    wxQtPageSetupDialog(wxWindow *parent, wxPageSetupDialogData *data = NULL);

    bool Create(wxWindow *parent, wxPageSetupDialogData *data = NULL);

    wxPageSetupDialogData& GetPageSetupDialogData() override;

private:
};

#endif // _WX_QT_PRINTDLG_H_
