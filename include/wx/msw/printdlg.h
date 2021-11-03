/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/printdlg.h
// Purpose:     wxPrintDialog, wxPageSetupDialog classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRINTDLG_H_
#define _WX_PRINTDLG_H_

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/prntbase.h"
#include "wx/printdlg.h"

class wxDC;
class WinPrinter;

//----------------------------------------------------------------------------
// wxWindowsPrintNativeData
//----------------------------------------------------------------------------

class wxWindowsPrintNativeData: public wxPrintNativeDataBase
{
public:
    wxWindowsPrintNativeData() = default;
    ~wxWindowsPrintNativeData();

    bool TransferTo( wxPrintData &data ) override;
    bool TransferFrom( const wxPrintData &data ) override;

    bool IsOk() const override;

    void InitializeDevMode(const wxString &printerName = {}, WinPrinter* printer = nullptr);
    void* GetDevMode() const { return m_devMode; }
    void SetDevMode(void* data) { m_devMode = data; }
    void* GetDevNames() const { return m_devNames; }
    void SetDevNames(void* data) { m_devNames = data; }

private:
    void* m_devMode{nullptr};
    void* m_devNames{nullptr};

    short m_customWindowsPaperId{0};

private:
    wxDECLARE_DYNAMIC_CLASS(wxWindowsPrintNativeData);
};

// ---------------------------------------------------------------------------
// wxWindowsPrintDialog: the MSW dialog for printing
// ---------------------------------------------------------------------------

class wxWindowsPrintDialog : public wxPrintDialogBase
{
public:
    wxWindowsPrintDialog(wxWindow *parent, wxPrintDialogData* data = nullptr);
    wxWindowsPrintDialog(wxWindow *parent, wxPrintData* data);
    ~wxWindowsPrintDialog();

    wxWindowsPrintDialog& operator=(wxWindowsPrintDialog&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent, wxPrintDialogData* data = nullptr);
    int ShowModal() override;

    wxPrintDialogData& GetPrintDialogData() override { return m_printDialogData; }
    wxPrintData& GetPrintData() override { return m_printDialogData.GetPrintData(); }

    wxDC *GetPrintDC() override;

private:
    wxPrintDialogData m_printDialogData;
    
    wxPrinterDC*      m_printerDC;
    wxWindow*         m_dialogParent;
    // holds MSW handle
    void*             m_printDlg;

    bool              m_destroyDC;

    bool ConvertToNative( wxPrintDialogData &data );
    bool ConvertFromNative( wxPrintDialogData &data );

    wxDECLARE_CLASS(wxWindowsPrintDialog);
};

// ---------------------------------------------------------------------------
// wxWindowsPageSetupDialog: the MSW page setup dialog
// ---------------------------------------------------------------------------

class wxWindowsPageSetupDialog: public wxPageSetupDialogBase
{
public:
    wxWindowsPageSetupDialog() = default;
    wxWindowsPageSetupDialog(wxWindow *parent, wxPageSetupDialogData *data = nullptr);
    ~wxWindowsPageSetupDialog();

    wxWindowsPageSetupDialog& operator=(wxWindowsPageSetupDialog&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent, wxPageSetupDialogData *data = nullptr);
    int ShowModal() override;
    bool ConvertToNative( wxPageSetupDialogData &data );
    bool ConvertFromNative( wxPageSetupDialogData &data );

    wxPageSetupDialogData& GetPageSetupDialogData() override { return m_pageSetupData; }

private:
    wxPageSetupDialogData   m_pageSetupData;
    wxWindow*               m_dialogParent{nullptr};

    // holds MSW handle
    void*                   m_pageDlg{nullptr};

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // wxUSE_PRINTING_ARCHITECTURE

#endif
    // _WX_PRINTDLG_H_
