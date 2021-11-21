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

#if wxUSE_PRINTING_ARCHITECTURE

#include "wx/defs.h"

#include "wx/event.h"
#include "wx/dialog.h"
#include "wx/intl.h"
#include "wx/cmndata.h"

import <string>;


// ---------------------------------------------------------------------------
// wxPrintDialogBase: interface for the dialog for printing
// ---------------------------------------------------------------------------

class wxPrintDialogBase : public wxDialog
{
public:
    wxPrintDialogBase() = default;
    wxPrintDialogBase(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const std::string &title = {},
                      const wxPoint &pos = wxDefaultPosition,
                      const wxSize &size = wxDefaultSize,
                      unsigned int style = wxDEFAULT_DIALOG_STYLE);

    wxPrintDialogBase& operator=(wxPrintDialogBase&&) = delete;

    virtual wxPrintDialogData& GetPrintDialogData() = 0;
    virtual wxPrintData& GetPrintData() = 0;
    virtual wxDC *GetPrintDC() = 0;

private:
    wxDECLARE_ABSTRACT_CLASS(wxPrintDialogBase);
};

// ---------------------------------------------------------------------------
// wxPrintDialog: the dialog for printing.
// ---------------------------------------------------------------------------

class wxPrintDialog
{
public:
    wxPrintDialog(wxWindow *parent, wxPrintDialogData* data = nullptr);
    wxPrintDialog(wxWindow *parent, wxPrintData* data);
    virtual ~wxPrintDialog();

    wxPrintDialog& operator=(wxPrintDialog&&) = delete;

    virtual int ShowModal();

    virtual wxPrintDialogData& GetPrintDialogData();
    virtual wxPrintData& GetPrintData();
    virtual wxDC *GetPrintDC();

private:
    wxPrintDialogBase  *m_pimpl;
};

// ---------------------------------------------------------------------------
// wxPageSetupDialogBase: interface for the page setup dialog
// ---------------------------------------------------------------------------

class wxPageSetupDialogBase: public wxDialog
{
public:
    wxPageSetupDialogBase() = default;
    wxPageSetupDialogBase(wxWindow *parent,
                      wxWindowID id = wxID_ANY,
                      const std::string& title = {},
                      const wxPoint &pos = wxDefaultPosition,
                      const wxSize &size = wxDefaultSize,
                      unsigned int style = wxDEFAULT_DIALOG_STYLE);

    wxPageSetupDialogBase& operator=(wxPageSetupDialogBase&&) = delete;

    virtual wxPageSetupDialogData& GetPageSetupDialogData() = 0;

private:
    wxDECLARE_ABSTRACT_CLASS(wxPageSetupDialogBase);
};

// ---------------------------------------------------------------------------
// wxPageSetupDialog: the page setup dialog
// ---------------------------------------------------------------------------

class wxPageSetupDialog
{
public:
    wxPageSetupDialog(wxWindow *parent, wxPageSetupDialogData *data = nullptr);
    ~wxPageSetupDialog();

    wxPageSetupDialog& operator=(wxPageSetupDialog&&) = delete;

    int ShowModal();
    wxPageSetupDialogData& GetPageSetupDialogData();
    // old name
    wxPageSetupDialogData& GetPageSetupData();

private:
    wxPageSetupDialogBase  *m_pimpl;
};

#endif

#endif
    // _WX_PRINTDLG_H_BASE_
