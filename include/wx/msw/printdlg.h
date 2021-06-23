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

#include "wx/dialog.h"
#include "wx/cmndata.h"
#include "wx/prntbase.h"
#include "wx/printdlg.h"

class WXDLLIMPEXP_FWD_CORE wxDC;
class WinPrinter;

//----------------------------------------------------------------------------
// wxWindowsPrintNativeData
//----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxWindowsPrintNativeData: public wxPrintNativeDataBase
{
public:
    wxWindowsPrintNativeData() = default;
    ~wxWindowsPrintNativeData() override;

    bool TransferTo( wxPrintData &data ) override;
    bool TransferFrom( const wxPrintData &data ) override;

    bool Ok() const override { return IsOk(); }
    bool IsOk() const override;

    void InitializeDevMode(const wxString &printerName = wxEmptyString, WinPrinter* printer = nullptr);
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

class WXDLLIMPEXP_CORE wxWindowsPrintDialog : public wxPrintDialogBase
{
public:
    wxWindowsPrintDialog(wxWindow *parent, wxPrintDialogData* data = nullptr);
    wxWindowsPrintDialog(wxWindow *parent, wxPrintData* data);
    ~wxWindowsPrintDialog() override;

    bool Create(wxWindow *parent, wxPrintDialogData* data = nullptr);
    int ShowModal() override;

    wxPrintDialogData& GetPrintDialogData() override { return m_printDialogData; }
    wxPrintData& GetPrintData() override { return m_printDialogData.GetPrintData(); }

    wxDC *GetPrintDC() override;

private:
    wxPrintDialogData m_printDialogData;
    wxPrinterDC*      m_printerDC;
    bool              m_destroyDC;
    wxWindow*         m_dialogParent;

private:
    bool ConvertToNative( wxPrintDialogData &data );
    bool ConvertFromNative( wxPrintDialogData &data );

    // holds MSW handle
    void*             m_printDlg;

private:
    wxDECLARE_CLASS(wxWindowsPrintDialog);
    wxWindowsPrintDialog(const wxWindowsPrintDialog&) = delete;
	wxWindowsPrintDialog& operator=(const wxWindowsPrintDialog&) = delete;
};

// ---------------------------------------------------------------------------
// wxWindowsPageSetupDialog: the MSW page setup dialog
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxWindowsPageSetupDialog: public wxPageSetupDialogBase
{
public:
    wxWindowsPageSetupDialog();
    wxWindowsPageSetupDialog(wxWindow *parent, wxPageSetupDialogData *data = nullptr);
    ~wxWindowsPageSetupDialog() override;

    wxWindowsPageSetupDialog(const wxWindowsPageSetupDialog&) = delete;
    wxWindowsPageSetupDialog& operator=(const wxWindowsPageSetupDialog&) = delete;
    wxWindowsPageSetupDialog(wxWindowsPageSetupDialog&&) = delete;
    wxWindowsPageSetupDialog& operator=(wxWindowsPageSetupDialog&&) = delete;

    bool Create(wxWindow *parent, wxPageSetupDialogData *data = nullptr);
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
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // wxUSE_PRINTING_ARCHITECTURE

#endif
    // _WX_PRINTDLG_H_
