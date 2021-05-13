/////////////////////////////////////////////////////////////////////////////
// Name:        wx/printdlg.h
// Purpose:     Base header and class for print dialogs
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRINTDLG_H_BASE_
#define _WX_PRINTDLG_H_BASE_

#include "wx/defs.h"

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/event.h"
#include "wx/dialog.h"
#include "wx/intl.h"
#include "wx/cmndata.h"


// ---------------------------------------------------------------------------
// wxPrintDialogBase: interface for the dialog for printing
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrintDialogBase : public wxDialog
{
public:
    wxPrintDialogBase() { }
    wxPrintDialogBase(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const wxString &title = wxEmptyString,
                      const wxPoint &pos = wxDefaultPosition,
                      const wxSize &size = wxDefaultSize,
                      long style = wxDEFAULT_DIALOG_STYLE);

    virtual wxPrintDialogData& GetPrintDialogData() = 0;
    virtual wxPrintData& GetPrintData() = 0;
    virtual wxDC *GetPrintDC() = 0;

private:
    wxDECLARE_ABSTRACT_CLASS(wxPrintDialogBase);
    wxPrintDialogBase(const wxPrintDialogBase&) = delete;
	wxPrintDialogBase& operator=(const wxPrintDialogBase&) = delete;
};

// ---------------------------------------------------------------------------
// wxPrintDialog: the dialog for printing.
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPrintDialog : public wxObject
{
public:
    wxPrintDialog(wxWindow *parent, wxPrintDialogData* data = NULL);
    wxPrintDialog(wxWindow *parent, wxPrintData* data);
    virtual ~wxPrintDialog();

    virtual int ShowModal();

    virtual wxPrintDialogData& GetPrintDialogData();
    virtual wxPrintData& GetPrintData();
    virtual wxDC *GetPrintDC();

private:
    wxPrintDialogBase  *m_pimpl;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPrintDialog);
    wxPrintDialog(const wxPrintDialog&) = delete;
	wxPrintDialog& operator=(const wxPrintDialog&) = delete;
};

// ---------------------------------------------------------------------------
// wxPageSetupDialogBase: interface for the page setup dialog
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPageSetupDialogBase: public wxDialog
{
public:
    wxPageSetupDialogBase() { }
    wxPageSetupDialogBase(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const wxString &title = wxEmptyString,
                      const wxPoint &pos = wxDefaultPosition,
                      const wxSize &size = wxDefaultSize,
                      long style = wxDEFAULT_DIALOG_STYLE);

    virtual wxPageSetupDialogData& GetPageSetupDialogData() = 0;

private:
    wxDECLARE_ABSTRACT_CLASS(wxPageSetupDialogBase);
    wxPageSetupDialogBase(const wxPageSetupDialogBase&) = delete;
	wxPageSetupDialogBase& operator=(const wxPageSetupDialogBase&) = delete;
};

// ---------------------------------------------------------------------------
// wxPageSetupDialog: the page setup dialog
// ---------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPageSetupDialog: public wxObject
{
public:
    wxPageSetupDialog(wxWindow *parent, wxPageSetupDialogData *data = NULL);
    virtual ~wxPageSetupDialog();

    int ShowModal();
    wxPageSetupDialogData& GetPageSetupDialogData();
    // old name
    wxPageSetupDialogData& GetPageSetupData();

private:
    wxPageSetupDialogBase  *m_pimpl;

private:
    wxDECLARE_DYNAMIC_CLASS(wxPageSetupDialog);
    wxPageSetupDialog(const wxPageSetupDialog&) = delete;
	wxPageSetupDialog& operator=(const wxPageSetupDialog&) = delete;
};

#endif

#endif
    // _WX_PRINTDLG_H_BASE_
