/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/prntdlgg.cpp
// Purpose:     Generic print dialogs
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_PRINTING_ARCHITECTURE && (!defined(__WXMSW__) || wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW)

#include "wx/generic/prntdlgg.h"

#include "wx/utils.h"
#include "wx/dc.h"
#include "wx/stattext.h"
#include "wx/statbox.h"
#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/textctrl.h"
#include "wx/radiobox.h"
#include "wx/filedlg.h"
#include "wx/combobox.h"
#include "wx/intl.h"

#if wxUSE_STATLINE
    #include "wx/statline.h"
#endif

#if wxUSE_POSTSCRIPT
    #include "wx/generic/dcpsg.h"
#endif

#include "wx/prntbase.h"
#include "wx/printdlg.h"
#include "wx/paper.h"
#include "wx/imaglist.h"

#ifndef __WXUNIVERSAL__

#if wxUSE_GTKPRINT
    #include "wx/link.h"
    wxFORCE_LINK_MODULE(gtk_print)
#endif

#endif // !wxUniv

#include <fmt/core.h>

import Utils.Strings;
import WX.Core.Cmndata;
import WX.Core.Sizer;
import WX.File.Filename;

import <array>;
import <string>;
import <vector>;

#if wxUSE_POSTSCRIPT

//----------------------------------------------------------------------------
// wxPostScriptNativeData
//----------------------------------------------------------------------------

wxIMPLEMENT_CLASS(wxPostScriptPrintNativeData, wxPrintNativeDataBase);

wxPostScriptPrintNativeData::wxPostScriptPrintNativeData()
{
#ifdef __VMS__
    m_printerCommand = "print";
    m_printerOptions = "/nonotify/queue=psqueue";
    m_afmPath = "sys$ps_font_metrics:";
#endif

#ifdef __WXMSW__
    m_printerCommand = "print";
    m_afmPath = "c:\\windows\\system\\";
#endif

#if !defined(__VMS__) && !defined(__WXMSW__)
    m_printerCommand = "lpr";
#endif

    m_printerScaleX = 1.0;
    m_printerScaleY = 1.0;
    m_printerTranslateX = 0;
    m_printerTranslateY = 0;
}

bool wxPostScriptPrintNativeData::TransferTo( [[maybe_unused]] wxPrintData &data )
{
    return true;
}

bool wxPostScriptPrintNativeData::TransferFrom( [[maybe_unused]] const wxPrintData &data )
{
    return true;
}

// ----------------------------------------------------------------------------
// Generic print dialog for non-Windows printing use.
// ----------------------------------------------------------------------------

wxIMPLEMENT_CLASS(wxGenericPrintDialog, wxPrintDialogBase);

wxBEGIN_EVENT_TABLE(wxGenericPrintDialog, wxPrintDialogBase)
    EVT_BUTTON(wxID_OK, wxGenericPrintDialog::OnOK)
    EVT_BUTTON(wxPRINTID_SETUP, wxGenericPrintDialog::OnSetup)
    EVT_RADIOBOX(wxPRINTID_RANGE, wxGenericPrintDialog::OnRange)
wxEND_EVENT_TABLE()

wxGenericPrintDialog::wxGenericPrintDialog(wxWindow *parent,
                                           wxPrintDialogData* data)
                    : wxPrintDialogBase(GetParentForModalDialog(parent, 0),
                               wxID_ANY, _("Print"),
                               wxPoint(0,0), wxSize(600, 600),
                               wxDEFAULT_DIALOG_STYLE |
                               wxTAB_TRAVERSAL)
{
    if ( data )
        m_printDialogData = *data;

    Init(parent);
}

wxGenericPrintDialog::wxGenericPrintDialog(wxWindow *parent,
                                           wxPrintData* data)
                    : wxPrintDialogBase(GetParentForModalDialog(parent, 0),
                                wxID_ANY, _("Print"),
                               wxPoint(0,0), wxSize(600, 600),
                               wxDEFAULT_DIALOG_STYLE |
                               wxTAB_TRAVERSAL)
{
    if ( data )
        m_printDialogData = *data;

    Init(parent);
}

void wxGenericPrintDialog::Init([[maybe_unused]] wxWindow * parent)
{
  //    wxDialog::Create(parent, wxID_ANY, _("Print"), wxPoint(0,0), wxSize(600, 600),
  //                     wxDEFAULT_DIALOG_STYLE | wxTAB_TRAVERSAL);

    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    // 1) top row

    wxPrintFactory* factory = wxPrintFactory::GetFactory();

    wxStaticBoxSizer *topsizer = new wxStaticBoxSizer(
        new wxStaticBox( this, wxID_ANY, _( "Printer options" ) ), wxHORIZONTAL );
    wxFlexGridSizer *flex = new wxFlexGridSizer( 2 );
    flex->AddGrowableCol( 1 );
    topsizer->Add( flex, 1, wxGROW );

    m_printToFileCheckBox = new wxCheckBox( this, wxPRINTID_PRINTTOFILE, _("Print to File") );
    flex->Add( m_printToFileCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_setupButton = new wxButton(this, wxPRINTID_SETUP, _("Setup...") );
    flex->Add( m_setupButton, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

    if (!factory->HasPrintSetupDialog())
        m_setupButton->Enable( false );

    if (factory->HasPrinterLine())
    {
        flex->Add( new wxStaticText( this, wxID_ANY, _("Printer:") ),
            0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
        flex->Add( new wxStaticText( this, wxID_ANY, factory->CreatePrinterLine() ),
            0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    }

    if (factory->HasStatusLine())
    {
        flex->Add( new wxStaticText( this, wxID_ANY, _("Status:") ),
            0, wxALIGN_CENTER_VERTICAL|(wxALL-wxTOP), 5 );
        flex->Add( new wxStaticText( this, wxID_ANY, factory->CreateStatusLine() ),
            0, wxALIGN_CENTER_VERTICAL|(wxALL-wxTOP), 5 );
    }

    mainsizer->Add( topsizer, 0, wxDirection::wxLEFT | wxDirection::wxTOP | wxDirection::wxRIGHT|wxGROW, 10 );

    // 2) middle row with radio box

    std::array<std::string, 2> choices {
        _("All"),
        _("Pages")
    };

    m_fromText = nullptr;
    m_toText = nullptr;
    m_rangeRadioBox = nullptr;

    if (m_printDialogData.GetFromPage() != 0)
    {
        m_rangeRadioBox = new wxRadioBox(this, wxPRINTID_RANGE, _("Print Range"),
                                         wxDefaultPosition, wxDefaultSize,
                                         2, choices);
        m_rangeRadioBox->SetSelection(1);

        mainsizer->Add( m_rangeRadioBox, 0, wxDirection::wxLEFT | wxDirection::wxTOP | wxDirection::wxRIGHT, 10 );
    }

    // 3) bottom row

    wxBoxSizer *bottomsizer = new wxBoxSizer( wxHORIZONTAL );

    if (m_printDialogData.GetFromPage() != 0)
    {
        bottomsizer->Add( new wxStaticText(this, wxPRINTID_STATIC, _("From:") ), 0, wxCENTER|wxALL, 5 );
        m_fromText = new wxTextCtrl(this, wxPRINTID_FROM, "", wxDefaultPosition, wxSize(40, wxDefaultCoord));
        bottomsizer->Add( m_fromText, 1, wxCENTER|wxRIGHT, 10 );

        bottomsizer->Add( new wxStaticText(this, wxPRINTID_STATIC, _("To:") ), 0, wxCENTER|wxALL, 5);
        m_toText = new wxTextCtrl(this, wxPRINTID_TO, "", wxDefaultPosition, wxSize(40, wxDefaultCoord));
        bottomsizer->Add( m_toText, 1, wxCENTER|wxRIGHT, 10 );
    }

    bottomsizer->Add( new wxStaticText(this, wxPRINTID_STATIC, _("Copies:") ), 0, wxCENTER|wxALL, 5 );
    m_noCopiesText = new wxTextCtrl(this, wxPRINTID_COPIES, "", wxPoint(252, 130), wxSize(40, wxDefaultCoord));
    bottomsizer->Add( m_noCopiesText, 1, wxCENTER|wxRIGHT, 10 );

    mainsizer->Add( bottomsizer, 0, wxDirection::wxTOP | wxDirection::wxLEFT | wxDirection::wxRIGHT, 12 );

    // 4) buttons

    wxSizer *sizerBtn = CreateSeparatedButtonSizer( wxOK|wxCANCEL);
    if ( sizerBtn )
        mainsizer->Add(sizerBtn, 0, wxEXPAND|wxALL, 10 );

    SetAutoLayout( true );
    SetSizer( mainsizer );

    mainsizer->Fit( this );
    Centre(wxBOTH);

    // Calls wxWindow::OnInitDialog and then wxGenericPrintDialog::TransferDataToWindow
    InitDialog();
    delete[] choices;
}

int wxGenericPrintDialog::ShowModal()
{
    return wxDialog::ShowModal();
}

void wxGenericPrintDialog::OnOK([[maybe_unused]] wxCommandEvent& event)
{
    TransferDataFromWindow();

    // An empty 'to' field signals printing just the
    // 'from' page.
    if (m_printDialogData.GetToPage() < 1)
        m_printDialogData.SetToPage(m_printDialogData.GetFromPage());

    // There are some interactions between the global setup data
    // and the standard print dialog. The global printing 'mode'
    // is determined by whether the user checks Print to file
    // or not.
    if (m_printDialogData.GetPrintToFile())
    {
        m_printDialogData.GetPrintData().SetPrintMode(wxPrintMode::File);

        wxFileName fname( m_printDialogData.GetPrintData().GetFilename() );

        wxFileDialog dialog( this, _("PostScript file"),
            fname.GetPath(), fname.GetFullName(), "*.ps", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
        if (dialog.ShowModal() != wxID_OK) return;

        m_printDialogData.GetPrintData().SetFilename( dialog.GetPath() );
    }
    else
    {
        m_printDialogData.GetPrintData().SetPrintMode(wxPrintMode::Printer);
    }

    EndModal(wxID_OK);
}

void wxGenericPrintDialog::OnRange(wxCommandEvent& event)
{
    if (!m_fromText) return;

    if (event.GetInt() == 0)
    {
        m_fromText->Enable(false);
        m_toText->Enable(false);
    }
    else if (event.GetInt() == 1)
    {
        m_fromText->Enable(true);
        m_toText->Enable(true);
    }
}

void wxGenericPrintDialog::OnSetup([[maybe_unused]] wxCommandEvent& event)
{
    wxPrintFactory* factory = wxPrintFactory::GetFactory();

    if (factory->HasPrintSetupDialog())
    {
        // The print setup dialog should change the
        // print data in-place if not cancelled.
        wxDialog *dialog = factory->CreatePrintSetupDialog( this, &m_printDialogData.GetPrintData() );
        dialog->ShowModal();
        dialog->Destroy();
    }
}

bool wxGenericPrintDialog::TransferDataToWindow()
{
    if(m_printDialogData.GetFromPage() != 0)
    {
       if(m_fromText)
       {
          if (m_printDialogData.GetEnablePageNumbers())
          {
             m_fromText->Enable(true);
             m_toText->Enable(true);
             if (m_printDialogData.GetFromPage() > 0)
                m_fromText->SetValue(fmt::format("{:d}"), m_printDialogData.GetFromPage()));
             if (m_printDialogData.GetToPage() > 0)
                m_toText->SetValue(fmt::format("{:d}"), m_printDialogData.GetToPage()));
             if(m_rangeRadioBox)
             {
                if (m_printDialogData.GetAllPages() || m_printDialogData.GetFromPage() == 0)
                   m_rangeRadioBox->SetSelection(0);
                else
                   m_rangeRadioBox->SetSelection(1);
             }
          }
          else
          {
             m_fromText->Enable(false);
             m_toText->Enable(false);
             if(m_rangeRadioBox)
             {
                m_rangeRadioBox->SetSelection(0);
                m_rangeRadioBox->wxRadioBox::Enable(1, false);
             }
          }
       }
    }
    m_noCopiesText->SetValue(
        fmt::format("{:d}"), m_printDialogData.GetNoCopies()));

    m_printToFileCheckBox->SetValue(m_printDialogData.GetPrintToFile());
    m_printToFileCheckBox->Enable(m_printDialogData.GetEnablePrintToFile());
    return true;
}

bool wxGenericPrintDialog::TransferDataFromWindow()
{
    long res = 0;
    if(m_printDialogData.GetFromPage() != -1)
    {
        if (m_printDialogData.GetEnablePageNumbers())
        {
            if(m_fromText)
            {
                wxString value = m_fromText->GetValue();
                if (value.ToLong( &res ))
                    m_printDialogData.SetFromPage( res );
            }
            if(m_toText)
            {
                wxString value = m_toText->GetValue();
                if (value.ToLong( &res ))
                    m_printDialogData.SetToPage( res );
            }
        }
        if(m_rangeRadioBox)
        {
            if (m_rangeRadioBox->GetSelection() == 0)
            {
                m_printDialogData.SetAllPages(true);

                // This means all pages, more or less
                m_printDialogData.SetFromPage(1);
                m_printDialogData.SetToPage(32000);
            }
            else
                m_printDialogData.SetAllPages(false);
        }
    }
    else
    { // continuous printing
        m_printDialogData.SetFromPage(1);
        m_printDialogData.SetToPage(32000);
    }

    wxString value = m_noCopiesText->GetValue();
    if (value.ToLong( &res ))
        m_printDialogData.SetNoCopies( res );

    m_printDialogData.SetPrintToFile(m_printToFileCheckBox->GetValue());

    return true;
}

wxDC *wxGenericPrintDialog::GetPrintDC()
{
    return new wxPostScriptDC(GetPrintDialogData().GetPrintData());
}

// ----------------------------------------------------------------------------
// Generic print setup dialog
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxGenericPrintSetupDialog, wxDialog)
    EVT_LIST_ITEM_ACTIVATED(wxPRINTID_PRINTER, wxGenericPrintSetupDialog::OnPrinter)
wxEND_EVENT_TABLE()

wxGenericPrintSetupDialog::wxGenericPrintSetupDialog(wxWindow *parent, wxPrintData* data):
wxDialog(parent, wxID_ANY, _("Print Setup"), wxPoint(0,0), wxSize(600, 600), wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL)
{
    Init(data);
}

/* XPM */
static const char* const check_xpm[] = {
/* width height ncolors chars_per_pixel */
"16 16 3 1",
/* colors */
"  s None c None",
"X c #000000",
". c #808080",
/* pixels */
"                ",
"                ",
"                ",
"             .. ",
"            XX  ",
"           XX.  ",
"          .XX   ",
"          XX    ",
"   X     XX.    ",
"   XX   .XX     ",
"    XX  XX      ",
"     XXXX.      ",
"      XX.       ",
"       .        ",
"                ",
"                "
};


void wxGenericPrintSetupDialog::Init(wxPrintData* data)
{
    if ( data )
        m_printData = *data;

    m_targetData = data;

    wxBoxSizer *main_sizer = new wxBoxSizer( wxVERTICAL );

    // printer selection

    wxStaticBoxSizer *printer_sizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Printer") ), wxVERTICAL );
    main_sizer->Add( printer_sizer, 0, wxALL|wxGROW, 10 );

    m_printerListCtrl = new wxListCtrl( this, wxPRINTID_PRINTER,
        wxDefaultPosition, wxSize(wxDefaultCoord,100), wxLC_REPORT|wxLC_SINGLE_SEL|wxSUNKEN_BORDER );
    wxImageList *image_list = new wxImageList;
    image_list->Add( wxBitmap(check_xpm) );
    m_printerListCtrl->AssignImageList( image_list, wxIMAGE_LIST_SMALL );

    m_printerListCtrl->InsertColumn( 0, " ", wxListColumnFormat::Left, 20 );
    m_printerListCtrl->InsertColumn( 1, "Printer", wxListColumnFormat::Left, 150 );
    m_printerListCtrl->InsertColumn( 2, "Device", wxListColumnFormat::Left, 150 );
    m_printerListCtrl->InsertColumn( 3, "Status", wxListColumnFormat::Left, 80 );

    wxListItem item;
    item.SetMask( wxLIST_MASK_TEXT );
    item.SetColumn( 1 );
    item.SetText( _("Default printer") );
    item.SetId( m_printerListCtrl->InsertItem( item ) );

    if (data->GetPrinterName().empty())
    {
        wxListItem item2;
        item2.SetId( item.GetId() );
        item2.SetMask( wxLIST_MASK_IMAGE );
        item2.SetImage( 0 );
        m_printerListCtrl->SetItem( item2 );
        // also select item
        m_printerListCtrl->SetItemState( item.GetId(), ListStates::Selected, ListStates::Selected );
    }

    item.SetId( 1+ item.GetId() );

    std::vector<wxString> errors;
    std::vector<wxString> output;
    long res = wxExecute( "lpstat -v", output, errors, wxEXEC_NODISABLE );
    if (res >= 0 && errors.GetCount() == 0)
    {
        size_t i;
        for (i = 0; i < output.GetCount(); i++)
        {
            wxStringTokenizer tok( output[i], " " );
            wxString tmp = tok.GetNextToken(); // "device"
            if (tmp != "device")
                break;  // the lpstat syntax must have changed.
            tmp = tok.GetNextToken();          // "for"
            if (tmp != "for")
                break;  // the lpstat syntax must have changed.
            tmp = tok.GetNextToken();          // "hp_deskjet930c:"
            if (tmp[tmp.length()-1] == wxT(':'))
                tmp.Remove(tmp.length()-1,1);
            wxString name = tmp;
            item.SetText( name );
            item.SetId( m_printerListCtrl->InsertItem( item ) );
            tmp = tok.GetNextToken();          // "parallel:/dev/lp0"
            item.SetColumn( 2 );
            item.SetText( tmp );
            m_printerListCtrl->SetItem( item );
            if (data->GetPrinterName() == name)
            {
                wxListItem item2;
                item2.SetId( item.GetId() );
                item2.SetMask( wxLIST_MASK_IMAGE );
                item2.SetImage( 0 );
                m_printerListCtrl->SetItem( item2 );
                // also select item
                m_printerListCtrl->SetItemState( item.GetId(), ListStates::Selected, ListStates::Selected );
            }

            wxString command = "lpstat -p ";
            command += name;
            std::vector<wxString> errors2;
            std::vector<wxString> output2;
            res = wxExecute( command, output2, errors2, wxEXEC_NODISABLE );
            if (res >= 0 && errors2.GetCount() == 0 && output2.GetCount() > 0)
            {
                tmp = output2[0]; // "printer hp_deskjet930c is idle. enable since ..."
                int pos = tmp.Find( wxT('.') );
                if (pos != wxNOT_FOUND)
                    tmp.Remove( (size_t)pos, tmp.length()-(size_t)pos );
                wxStringTokenizer tok2( tmp, " " );
                tmp = tok2.GetNextToken();  // "printer"
                tmp = tok2.GetNextToken();  // "hp_deskjet930c"
                tmp.clear();
                while (tok2.HasMoreTokens())
                {
                    tmp += tok2.GetNextToken();
                    tmp += " ";
                }
                item.SetColumn( 3 );
                item.SetText( tmp );
                m_printerListCtrl->SetItem( item );
            }

            item.SetColumn( 1 );
            item.SetId( 1+ item.GetId() );
        }
    }


    printer_sizer->Add( m_printerListCtrl, 0, wxALL|wxGROW, 5 );

    wxBoxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );
    main_sizer->Add( item1, 0, wxALL, 5 );

    // printer options (on the left)

    wxBoxSizer *item2 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item4 = new wxStaticBox( this, wxPRINTID_STATIC, _("Paper size") );
    wxStaticBoxSizer *item3 = new wxStaticBoxSizer( item4, wxVERTICAL );

    m_paperTypeChoice = CreatePaperTypeChoice();
    item3->Add( m_paperTypeChoice, 0, wxALIGN_CENTER|wxALL, 5 );

    item2->Add( item3, 0, wxALIGN_CENTER|wxALL, 5 );

    wxString strs6[] =
    {
        _("Portrait"),
        _("Landscape")
    };
    m_orientationRadioBox= new wxRadioBox( this, wxPRINTID_ORIENTATION, _("Orientation"), wxDefaultPosition, wxDefaultSize, 2, strs6, 1, wxRA_SPECIFY_ROWS );
    item2->Add( m_orientationRadioBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxStaticBox *item8 = new wxStaticBox( this, wxID_ANY, _("Options") );
    wxStaticBoxSizer *item7 = new wxStaticBoxSizer( item8, wxHORIZONTAL );

    m_colourCheckBox = new wxCheckBox( this, wxPRINTID_PRINTCOLOUR, _("Print in colour") );
    item7->Add( m_colourCheckBox, 0, wxALIGN_CENTER|wxALL, 5 );

    item2->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item1->Add( item2, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    // spooling options (on the right)

    wxStaticBox *item11 = new wxStaticBox( this, wxID_ANY, _("Print spooling") );
    wxStaticBoxSizer *item10 = new wxStaticBoxSizer( item11, wxVERTICAL );

    wxStaticText *item12 = new wxStaticText( this, wxID_ANY, _("Printer command:") );
    item10->Add( item12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxBoxSizer *item13 = new wxBoxSizer( wxHORIZONTAL );

    item13->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );

    m_printerCommandText = new wxTextCtrl( this, wxPRINTID_COMMAND, "", wxDefaultPosition, wxSize(160,wxDefaultCoord) );
    item13->Add( m_printerCommandText, 0, wxALIGN_CENTER|wxALL, 5 );

    item10->Add( item13, 0, wxALIGN_CENTER|wxALL, 0 );

    wxStaticText *item15 = new wxStaticText( this, wxID_ANY, _("Printer options:") );
    item10->Add( item15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxBoxSizer *item16 = new wxBoxSizer( wxHORIZONTAL );

    item16->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );

    m_printerOptionsText = new wxTextCtrl( this, wxPRINTID_OPTIONS, "", wxDefaultPosition, wxSize(160,wxDefaultCoord) );
    item16->Add( m_printerOptionsText, 0, wxALIGN_CENTER|wxALL, 5 );

    item10->Add( item16, 0, wxALIGN_CENTER|wxALL, 0 );

    item1->Add( item10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );


#if wxUSE_STATLINE
    // static line
    main_sizer->Add( new wxStaticLine( this, wxID_ANY ), 0, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 10 );
#endif

    // buttons

    main_sizer->Add( CreateButtonSizer( wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 10 );

    SetAutoLayout( true );
    SetSizer( main_sizer );

    main_sizer->Fit( this );
    Centre(wxBOTH);


    Fit();
    Centre(wxBOTH);

    InitDialog();
}

void wxGenericPrintSetupDialog::OnPrinter(wxListEvent& event)
{
    // Delete check mark
    for (long item = 0; item < m_printerListCtrl->GetItemCount(); item++)
        m_printerListCtrl->SetItemImage( item, -1 );

    m_printerListCtrl->SetItemImage( event.GetIndex(), 0 );

    if (event.GetIndex() == 0)
    {
        m_printerCommandText->SetValue( "lpr" );
    }
    else
    {
        wxListItem li;
        li.SetColumn( 1 );
        li.SetMask( wxLIST_MASK_TEXT );
        li.SetId( event.GetIndex() );
        m_printerListCtrl->GetItem( li );
        m_printerCommandText->SetValue( "lpr -P" + li.GetText() );
    }
}

bool wxGenericPrintSetupDialog::TransferDataToWindow()
{
    wxPostScriptPrintNativeData *data =
        (wxPostScriptPrintNativeData *) m_printData.GetNativeData();

    if (m_printerCommandText && !data->GetPrinterCommand().empty())
        m_printerCommandText->SetValue(data->GetPrinterCommand());
    if (m_printerOptionsText && !data->GetPrinterOptions().empty())
        m_printerOptionsText->SetValue(data->GetPrinterOptions());
    if (m_colourCheckBox)
        m_colourCheckBox->SetValue(m_printData.GetColour());

    if (m_orientationRadioBox)
    {
        if (m_printData.GetOrientation() == wxPrintOrientation::Portrait)
            m_orientationRadioBox->SetSelection(0);
        else
            m_orientationRadioBox->SetSelection(1);
    }
    return true;
}

bool wxGenericPrintSetupDialog::TransferDataFromWindow()
{
    wxPostScriptPrintNativeData *data =
        (wxPostScriptPrintNativeData *) m_printData.GetNativeData();

    // find selected printer
    long id = m_printerListCtrl->GetNextItem( -1, wxListGetNextItem::All, ListStates::Selected );
    if (id == 0)
    {
        m_printData.SetPrinterName( {} );
    }
    else
    {
        wxListItem item;
        item.SetId( id );
        item.SetMask( wxLIST_MASK_TEXT );
        item.SetColumn( 1 );
        m_printerListCtrl->GetItem( item );
        m_printData.SetPrinterName( item.GetText() );
    }

    if (m_printerCommandText)
        data->SetPrinterCommand(m_printerCommandText->GetValue());
    if (m_printerOptionsText)
        data->SetPrinterOptions(m_printerOptionsText->GetValue());
    if (m_colourCheckBox)
        m_printData.SetColour(m_colourCheckBox->GetValue());
    if (m_orientationRadioBox)
    {
        int sel = m_orientationRadioBox->GetSelection();
        if (sel == 0)
            m_printData.SetOrientation(wxPrintOrientation::Portrait);
        else
            m_printData.SetOrientation(wxPrintOrientation::Landscape);
    }
    if (m_paperTypeChoice)
    {
        int selectedItem = m_paperTypeChoice->GetSelection();
        if (selectedItem != -1)
        {
            wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(selectedItem);
            if (paper != nullptr)
              m_printData.SetPaperId( paper->GetId());
        }
    }

    if (m_targetData)
        *m_targetData = m_printData;

    return true;
}

wxComboBox *wxGenericPrintSetupDialog::CreatePaperTypeChoice()
{
    size_t n = wxThePrintPaperDatabase->GetCount();
    wxString *choices = new wxString [n];
    size_t sel = 0;

    for (size_t i = 0; i < n; i++)
    {
        wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(i);
        choices[i] = paper->GetName();
        if (m_printData.GetPaperId() == paper->GetId())
            sel = i;
    }

    int width = 250;

    wxComboBox *choice = new wxComboBox( this,
                                         wxPRINTID_PAPERSIZE,
                                         _("Paper size"),
                                         wxDefaultPosition,
                                         wxSize(width, wxDefaultCoord),
                                         n, choices );

    delete[] choices;

    choice->SetSelection(sel);
    return choice;
}

#endif // wxUSE_POSTSCRIPT

// ----------------------------------------------------------------------------
// Generic page setup dialog
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxGenericPageSetupDialog, wxPageSetupDialogBase)
    EVT_BUTTON(wxPRINTID_SETUP, wxGenericPageSetupDialog::OnPrinter)
wxEND_EVENT_TABLE()

wxGenericPageSetupDialog::wxGenericPageSetupDialog( wxWindow *parent,
                                                    wxPageSetupDialogData* data)
    : wxPageSetupDialogBase( parent,
                wxID_ANY,
                _("Page setup"),
                wxPoint(0,0),
                wxSize(600, 600),
                wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL )
{
    if (data)
        m_pageData = *data;

    int textWidth = 80;

    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    // 1) top
    wxStaticBoxSizer *topsizer = new wxStaticBoxSizer(
      new wxStaticBox(this,wxPRINTID_STATIC, _("Paper size")), wxHORIZONTAL );

    size_t      n = wxThePrintPaperDatabase->GetCount();
    std::vector<std::string> choices(n);

    for (size_t i = 0; i < n; i++)
    {
        wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(i);
        choices[i] = paper->GetName();
    }

    m_paperTypeChoice = new wxComboBox( this,
                                        wxPRINTID_PAPERSIZE,
                                        _("Paper size"),
                                        wxDefaultPosition,
                                        wxSize(300, wxDefaultCoord),
                                        choices );
    topsizer->Add( m_paperTypeChoice, 1, wxEXPAND|wxALL, 5 );
//  m_paperTypeChoice->SetSelection(sel);

    mainsizer->Add( topsizer, 0, wxDirection::wxTOP | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxEXPAND, 10 );

    // 2) middle sizer with radio box

    std::vector<std::string> choices2(2);
    choices2[0] = "Portrait";
    choices2[1] = "Landscape";
    m_orientationRadioBox = new wxRadioBox(this, wxPRINTID_ORIENTATION, "Orientation",
        wxDefaultPosition, wxDefaultSize, choices2, 2);
    m_orientationRadioBox->SetSelection(0);

    mainsizer->Add( m_orientationRadioBox, 0, wxDirection::wxTOP | wxDirection::wxLEFT | wxDirection::wxRIGHT, 10 );

    // 3) margins

    wxBoxSizer *table = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer *column1 = new wxBoxSizer( wxVERTICAL );
    column1->Add( new wxStaticText(this, wxPRINTID_STATIC, _("Left margin (mm):")),1,wxALL|wxALIGN_RIGHT,5 );
    column1->Add( new wxStaticText(this, wxPRINTID_STATIC, _("Top margin (mm):")),1,wxALL|wxALIGN_RIGHT,5 );
    table->Add( column1, 0, wxALL | wxEXPAND, 5 );

    wxBoxSizer *column2 = new wxBoxSizer( wxVERTICAL );
    m_marginLeftText = new wxTextCtrl(this, wxPRINTID_LEFTMARGIN, "", wxDefaultPosition, wxSize(textWidth, wxDefaultCoord));
    m_marginTopText = new wxTextCtrl(this, wxPRINTID_TOPMARGIN, "", wxDefaultPosition, wxSize(textWidth, wxDefaultCoord));
    column2->Add( m_marginLeftText, 1, wxALL, 5 );
    column2->Add( m_marginTopText, 1, wxALL, 5 );
    table->Add( column2, 0, wxRIGHT|wxTOP|wxBOTTOM | wxEXPAND, 5 );

    wxBoxSizer *column3 = new wxBoxSizer( wxVERTICAL );
    column3->Add( new wxStaticText(this, wxPRINTID_STATIC, _("Right margin (mm):")),1,wxALL|wxALIGN_RIGHT,5 );
    column3->Add( new wxStaticText(this, wxPRINTID_STATIC, _("Bottom margin (mm):")),1,wxALL|wxALIGN_RIGHT,5 );
    table->Add( column3, 0, wxALL | wxEXPAND, 5 );

    wxBoxSizer *column4 = new wxBoxSizer( wxVERTICAL );
    m_marginRightText = new wxTextCtrl(this, wxPRINTID_RIGHTMARGIN, "", wxDefaultPosition, wxSize(textWidth, wxDefaultCoord));
    m_marginBottomText = new wxTextCtrl(this, wxPRINTID_BOTTOMMARGIN, "", wxDefaultPosition, wxSize(textWidth, wxDefaultCoord));
    column4->Add( m_marginRightText, 1, wxALL, 5 );
    column4->Add( m_marginBottomText, 1, wxALL, 5 );
    table->Add( column4, 0, wxDirection::wxRIGHT | wxDirection::wxTOP | wxDirection::wxBOTTOM | wxEXPAND, 5 );

    mainsizer->Add( table, 0 );

#if wxUSE_STATLINE
    // 5) static line
    mainsizer->Add( new wxStaticLine( this, wxID_ANY ), 0, wxEXPAND | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 10 );
#endif

    // 6) buttons

    wxSizer* buttonsizer = CreateButtonSizer( wxOK|wxCANCEL);

    if (wxPrintFactory::GetFactory()->HasPrintSetupDialog())
    {
        m_printerButton = new wxButton(this, wxPRINTID_SETUP, _("Printer...") );
        buttonsizer->Add( m_printerButton, 0, wxDirection::wxLEFT | wxDirection::wxRIGHT, 10 );
        if ( !m_pageData.GetEnablePrinter() )
            m_printerButton->Enable(false);
    }
    else
    {
        m_printerButton = nullptr;
    }

    //  if (m_printData.GetEnableHelp())
    //  wxButton *helpButton = new wxButton(this, (wxFunction)wxGenericPageSetupHelpProc, _("Help"), wxDefaultCoord, wxDefaultCoord, buttonWidth, buttonHeight);
    mainsizer->Add( buttonsizer, 0, wxEXPAND|wxALL, 10 );


    SetAutoLayout( true );
    SetSizer( mainsizer );

    mainsizer->Fit( this );
    Centre(wxBOTH);

    InitDialog();
}

wxPageSetupDialogData& wxGenericPageSetupDialog::GetPageSetupDialogData()
{
    return m_pageData;
}

bool wxGenericPageSetupDialog::TransferDataToWindow()
{
    if (m_marginLeftText)
        m_marginLeftText->SetValue(fmt::format("{:d}", m_pageData.GetMarginTopLeft().x));
    if (m_marginTopText)
        m_marginTopText->SetValue(fmt::format("{:d}", m_pageData.GetMarginTopLeft().y));
    if (m_marginRightText)
        m_marginRightText->SetValue(fmt::format("{:d}", m_pageData.GetMarginBottomRight().x));
    if (m_marginBottomText)
        m_marginBottomText->SetValue(fmt::format("{:d}", m_pageData.GetMarginBottomRight().y));

    if (m_orientationRadioBox)
    {
        if (m_pageData.GetPrintData().GetOrientation() == wxPrintOrientation::Portrait)
            m_orientationRadioBox->SetSelection(0);
        else
            m_orientationRadioBox->SetSelection(1);
    }

    // Find the paper type from either the current paper size in the wxPageSetupDialogData, or
    // failing that, the id in the wxPrintData object.

    wxPrintPaperType* type = wxThePrintPaperDatabase->FindPaperType(
             wxSize(m_pageData.GetPaperSize().x * 10, m_pageData.GetPaperSize().y * 10));

    if (!type && m_pageData.GetPrintData().GetPaperId() != wxPaperSize::None)
        type = wxThePrintPaperDatabase->FindPaperType(m_pageData.GetPrintData().GetPaperId());

    if (type)
    {
        m_paperTypeChoice->SetStringSelection(type->GetName());
    }

    return true;
}

bool wxGenericPageSetupDialog::TransferDataFromWindow()
{
    if (m_marginLeftText && m_marginTopText)
    {
        int left = wxAtoi( m_marginLeftText->GetValue().c_str() );
        int top = wxAtoi( m_marginTopText->GetValue().c_str() );
        m_pageData.SetMarginTopLeft( wxPoint(left,top) );
    }
    if (m_marginRightText && m_marginBottomText)
    {
        int right = wxAtoi( m_marginRightText->GetValue().c_str() );
        int bottom = wxAtoi( m_marginBottomText->GetValue().c_str() );
        m_pageData.SetMarginBottomRight( wxPoint(right,bottom) );
    }

    if (m_orientationRadioBox)
    {
        int sel = m_orientationRadioBox->GetSelection();
        if (sel == 0)
        {
            m_pageData.GetPrintData().SetOrientation(wxPrintOrientation::Portrait);
        }
        else
        {
            m_pageData.GetPrintData().SetOrientation(wxPrintOrientation::Landscape);
        }
    }

    if (m_paperTypeChoice)
    {
        int selectedItem = m_paperTypeChoice->GetSelection();
        if (selectedItem != -1)
        {
            wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(selectedItem);
            if ( paper )
            {
                m_pageData.SetPaperSize(wxSize(paper->GetWidth()/10, paper->GetHeight()/10));
                m_pageData.GetPrintData().SetPaperId(paper->GetId());
            }
        }
    }

    return true;
}

wxComboBox *wxGenericPageSetupDialog::CreatePaperTypeChoice(int *x, int *y)
{
/*
    if (!wxThePrintPaperDatabase)
    {
        wxThePrintPaperDatabase = new wxPrintPaperDatabase;
        wxThePrintPaperDatabase->CreateDatabase();
    }
*/

    size_t      n = wxThePrintPaperDatabase->GetCount();
    std::vector<std::string> choices(n);

    for (size_t i = 0; i < n; i++)
    {
        wxPrintPaperType *paper = wxThePrintPaperDatabase->Item(i);
        choices[i] = paper->GetName();
    }

    // FIXME: This isn't good.
    (void) new wxStaticText(this, wxPRINTID_STATIC, _("Paper size"), wxPoint(*x, *y));
    *y += 25;

    wxComboBox *choice = new wxComboBox( this,
                                         wxPRINTID_PAPERSIZE,
                                         _("Paper size"),
                                         wxPoint(*x, *y),
                                         wxSize(300, wxDefaultCoord),
                                         choices );
    *y += 35;

//    choice->SetSelection(sel);
    return choice;
}

void wxGenericPageSetupDialog::OnPrinter([[maybe_unused]] wxCommandEvent& event)
{
    // We no longer query GetPrintMode, so we can eliminate the need
    // to call SetPrintMode.
    // This has the limitation that we can't explicitly call the PostScript
    // print setup dialog from the generic Page Setup dialog under Windows,
    // but since this choice would only happen when trying to do PostScript
    // printing under Windows (and only in 16-bit Windows which
    // doesn't have a Windows-specific page setup dialog) it's worth it.

    // First save the current settings, so the wxPrintData object is up to date.
    TransferDataFromWindow();

    // Transfer the current print settings from this dialog to the page setup dialog.

#if 0
    // Use print factory later

    wxPrintDialogData data;
    data = GetPageSetupData().GetPrintData();
    data.SetSetupDialog(true);
    wxPrintDialog printDialog(this, & data);
    printDialog.ShowModal();

    // Transfer the page setup print settings from the page dialog to this dialog again, in case
    // the page setup dialog changed something.
    GetPageSetupData().GetPrintData() = printDialog.GetPrintDialogData().GetPrintData();
    GetPageSetupData().CalculatePaperSizeFromId(); // Make sure page size reflects the id in wxPrintData

    // Now update the dialog in case the page setup dialog changed some of our settings.
    TransferDataToWindow();
#endif
}

#endif
