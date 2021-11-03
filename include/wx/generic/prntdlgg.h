/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/prntdlgg.h
// Purpose:     wxGenericPrintDialog, wxGenericPrintSetupDialog,
//              wxGenericPageSetupDialog
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __PRINTDLGH_G_
#define __PRINTDLGH_G_

#include "wx/defs.h"

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/dialog.h"
#include "wx/cmndata.h"
#include "wx/prntbase.h"
#include "wx/printdlg.h"
#include "wx/listctrl.h"

#include "wx/dc.h"
#if wxUSE_POSTSCRIPT
    #include "wx/dcps.h"
#endif

class WXDLLIMPEXP_FWD_CORE wxTextCtrl;
class WXDLLIMPEXP_FWD_CORE wxButton;
class WXDLLIMPEXP_FWD_CORE wxCheckBox;
class WXDLLIMPEXP_FWD_CORE wxComboBox;
class WXDLLIMPEXP_FWD_CORE wxStaticText;
class WXDLLIMPEXP_FWD_CORE wxRadioBox;
class WXDLLIMPEXP_FWD_CORE wxPageSetupDialogData;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// This is not clear why all these enums start with 10 or 30 but do not change it
// without good reason to avoid some subtle backwards compatibility breakage

enum
{
    wxPRINTID_STATIC = 10,
    wxPRINTID_RANGE,
    wxPRINTID_FROM,
    wxPRINTID_TO,
    wxPRINTID_COPIES,
    wxPRINTID_PRINTTOFILE,
    wxPRINTID_SETUP
};

enum
{
    wxPRINTID_LEFTMARGIN = 30,
    wxPRINTID_RIGHTMARGIN,
    wxPRINTID_TOPMARGIN,
    wxPRINTID_BOTTOMMARGIN
};

enum
{
    wxPRINTID_PRINTCOLOUR = 10,
    wxPRINTID_ORIENTATION,
    wxPRINTID_COMMAND,
    wxPRINTID_OPTIONS,
    wxPRINTID_PAPERSIZE,
    wxPRINTID_PRINTER
};

#if wxUSE_POSTSCRIPT

//----------------------------------------------------------------------------
// wxPostScriptNativeData
//----------------------------------------------------------------------------

class wxPostScriptPrintNativeData: public wxPrintNativeDataBase
{
public:
    wxPostScriptPrintNativeData();

    bool TransferTo( wxPrintData &data ) override;
    bool TransferFrom( const wxPrintData &data ) override;

    bool IsOk() const override { return true; }

    const std::string& GetPrinterCommand() const { return m_printerCommand; }
    const std::string& GetPrinterOptions() const { return m_printerOptions; }
    const std::string& GetPreviewCommand() const { return m_previewCommand; }
    const std::string& GetFontMetricPath() const { return m_afmPath; }
    double GetPrinterScaleX() const { return m_printerScaleX; }
    double GetPrinterScaleY() const { return m_printerScaleY; }
    long GetPrinterTranslateX() const { return m_printerTranslateX; }
    long GetPrinterTranslateY() const { return m_printerTranslateY; }

    void SetPrinterCommand(const std::string& command) { m_printerCommand = command; }
    void SetPrinterOptions(const std::string& options) { m_printerOptions = options; }
    void SetPreviewCommand(const std::string& command) { m_previewCommand = command; }
    void SetFontMetricPath(const std::string& path) { m_afmPath = path; }
    void SetPrinterScaleX(double x) { m_printerScaleX = x; }
    void SetPrinterScaleY(double y) { m_printerScaleY = y; }
    void SetPrinterScaling(double x, double y) { m_printerScaleX = x; m_printerScaleY = y; }
    void SetPrinterTranslateX(long x) { m_printerTranslateX = x; }
    void SetPrinterTranslateY(long y) { m_printerTranslateY = y; }
    void SetPrinterTranslation(long x, long y) { m_printerTranslateX = x; m_printerTranslateY = y; }

#if wxUSE_STREAMS
    wxOutputStream *GetOutputStream() { return m_outputStream; }
    void SetOutputStream( wxOutputStream *output ) { m_outputStream = output; }
#endif

private:
    std::string        m_printerCommand;
    std::string        m_previewCommand;
    std::string        m_printerOptions;
    std::string        m_afmPath;
    double          m_printerScaleX;
    double          m_printerScaleY;
    long            m_printerTranslateX;
    long            m_printerTranslateY;
#if wxUSE_STREAMS
    wxOutputStream *m_outputStream;
#endif

private:
    wxDECLARE_DYNAMIC_CLASS(wxPostScriptPrintNativeData);
};

// ----------------------------------------------------------------------------
// Simulated Print and Print Setup dialogs for non-Windows platforms (and
// Windows using PostScript print/preview)
// ----------------------------------------------------------------------------

class wxGenericPrintDialog : public wxPrintDialogBase
{
public:
    wxGenericPrintDialog(wxWindow *parent,
                         wxPrintDialogData* data = nullptr);
    wxGenericPrintDialog(wxWindow *parent, wxPrintData* data);

    void OnSetup(wxCommandEvent& event);
    void OnRange(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);

    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

    int ShowModal() override;

    wxPrintData& GetPrintData() override
        { return m_printDialogData.GetPrintData(); }

    wxPrintDialogData& GetPrintDialogData() override { return m_printDialogData; }
    wxDC *GetPrintDC() override;

public:
//    wxStaticText*       m_printerMessage;
    wxButton*           m_setupButton;
//    wxButton*           m_helpButton;
    wxRadioBox*         m_rangeRadioBox;
    wxTextCtrl*         m_fromText;
    wxTextCtrl*         m_toText;
    wxTextCtrl*         m_noCopiesText;
    wxCheckBox*         m_printToFileCheckBox;
//    wxCheckBox*         m_collateCopiesCheckBox;

    wxPrintDialogData   m_printDialogData;

protected:
    void Init(wxWindow *parent);

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxGenericPrintDialog);
};

class wxGenericPrintSetupDialog : public wxDialog
{
public:
    // There are no configuration options for the dialog, so we
    // just pass the wxPrintData object (no wxPrintSetupDialogData class needed)
    wxGenericPrintSetupDialog(wxWindow *parent, wxPrintData* data);

    void Init(wxPrintData* data);

    void OnPrinter(wxListEvent& event);

    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

    virtual wxComboBox *CreatePaperTypeChoice();

public:
    wxListCtrl*         m_printerListCtrl;
    wxRadioBox*         m_orientationRadioBox;
    wxTextCtrl*         m_printerCommandText;
    wxTextCtrl*         m_printerOptionsText;
    wxCheckBox*         m_colourCheckBox;
    wxComboBox*         m_paperTypeChoice;

    wxPrintData         m_printData;
    wxPrintData&        GetPrintData() { return m_printData; }

    // After pressing OK, write data here.
    wxPrintData*        m_targetData;

private:
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxGenericPrintSetupDialog);
};
#endif
    // wxUSE_POSTSCRIPT

class wxGenericPageSetupDialog : public wxPageSetupDialogBase
{
public:
    wxGenericPageSetupDialog(wxWindow *parent = nullptr,
                             wxPageSetupDialogData* data = nullptr);

	wxGenericPageSetupDialog(const wxGenericPageSetupDialog&) = delete;
	wxGenericPageSetupDialog& operator=(const wxGenericPageSetupDialog&) = delete;

    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

    wxPageSetupDialogData& GetPageSetupDialogData() override;

    void OnPrinter(wxCommandEvent& event);
    wxComboBox *CreatePaperTypeChoice(int* x, int* y);

public:
    wxButton*       m_printerButton;
    wxRadioBox*     m_orientationRadioBox;
    wxTextCtrl*     m_marginLeftText;
    wxTextCtrl*     m_marginTopText;
    wxTextCtrl*     m_marginRightText;
    wxTextCtrl*     m_marginBottomText;
    wxComboBox*       m_paperTypeChoice;

    wxPageSetupDialogData m_pageData;

private:
    wxDECLARE_EVENT_TABLE();

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif

#endif
// __PRINTDLGH_G_
