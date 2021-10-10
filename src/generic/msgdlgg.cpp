/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/msgdlgg.cpp
// Purpose:     wxGenericMessageDialog
// Author:      Julian Smart, Robert Roebling
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart and Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_MSGDLG

#ifndef WX_PRECOMP
    #include <string>
#endif

#include "wx/dialogflags.h"
#include "wx/utils.h"
#include "wx/dialog.h"
#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/statbmp.h"
#include "wx/layout.h"
#include "wx/intl.h"
#include "wx/icon.h"
#include "wx/sizer.h"
#include "wx/app.h"
#include "wx/settings.h"

#define __WX_COMPILING_MSGDLGG_CPP__ 1
#include "wx/msgdlg.h"
#include "wx/artprov.h"
#include "wx/textwrapper.h"
#include "wx/modalhook.h"

#if wxUSE_STATLINE
    #include "wx/statline.h"
#endif

// ----------------------------------------------------------------------------
// wxTitleTextWrapper: simple class to create wrapped text in "title font"
// ----------------------------------------------------------------------------

class wxTitleTextWrapper : public wxTextSizerWrapper
{
public:
    explicit wxTitleTextWrapper(wxWindow *win)
        : wxTextSizerWrapper(win)
    {
    }

protected:
    wxWindow *OnCreateLine(const std::string& s) override
    {
        wxWindow * const win = wxTextSizerWrapper::OnCreateLine(s);

        win->SetFont(win->GetFont().Larger().MakeBold());

        return win;
    }
};

// ----------------------------------------------------------------------------
// icons
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxGenericMessageDialog, wxDialog)
        EVT_BUTTON(wxID_YES, wxGenericMessageDialog::OnYes)
        EVT_BUTTON(wxID_NO, wxGenericMessageDialog::OnNo)
        EVT_BUTTON(wxID_HELP, wxGenericMessageDialog::OnHelp)
        EVT_BUTTON(wxID_CANCEL, wxGenericMessageDialog::OnCancel)
wxEND_EVENT_TABLE()

wxIMPLEMENT_CLASS(wxGenericMessageDialog, wxDialog);

wxGenericMessageDialog::wxGenericMessageDialog( wxWindow *parent,
                                                const std::string& message,
                                                const std::string& caption,
                                                DialogFlags style,
                                                const wxPoint& pos)
                      : wxMessageDialogBase(GetParentForModalDialog(parent, style),
                                            message,
                                            caption,
                                            style),
                        m_pos(pos)
{
}

wxSizer *wxGenericMessageDialog::CreateMsgDlgButtonSizer()
{
    if ( HasCustomLabels() )
    {
        wxStdDialogButtonSizer * const sizerStd = new wxStdDialogButtonSizer;

        wxButton *btnDef = nullptr;

        if ( m_dialogStyle & wxDialogFlags::OK )
        {
            btnDef = new wxButton(this, wxID_OK, GetCustomOKLabel());
            sizerStd->AddButton(btnDef);
        }

        if ( m_dialogStyle & wxDialogFlags::Cancel )
        {
            wxButton * const
                cancel = new wxButton(this, wxID_CANCEL, GetCustomCancelLabel());
            sizerStd->AddButton(cancel);

            if ( m_dialogStyle & wxDialogDefaultFlags::Cancel )
                btnDef = cancel;
        }

        if ( m_dialogStyle & wxDialogFlags::Yes_No )
        {
            wxButton * const
                yes = new wxButton(this, wxID_YES, GetCustomYesLabel());
            sizerStd->AddButton(yes);

            wxButton * const
                no = new wxButton(this, wxID_NO, GetCustomNoLabel());
            sizerStd->AddButton(no);
            if ( m_dialogStyle & wxDialogDefaultFlags::No )
                btnDef = no;
            else if ( !btnDef )
                btnDef = yes;
        }

        if ( m_dialogStyle & wxDialogFlags::Help )
        {
            wxButton * const
                help = new wxButton(this, wxID_HELP, GetCustomHelpLabel());
            sizerStd->AddButton(help);
        }

        if ( btnDef )
        {
            btnDef->SetDefault();
            btnDef->SetFocus();
        }

        sizerStd->Realize();

        return CreateSeparatedSizer(sizerStd);
    }

    // Use standard labels for all buttons
    return CreateSeparatedButtonSizer
           (
                m_dialogStyle & DialogFlags{wxDialogFlags::OK, wxDialogFlags::Cancel, wxDialogFlags::Help, wxDialogFlags::Yes_No,
                                            wxDialogDefaultFlags::No, wxDialogDefaultFlags::Cancel})
           );
}

void wxGenericMessageDialog::DoCreateMsgdialog()
{
    wxDialog::Create(m_parent, wxID_ANY, m_caption, m_pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

#if wxUSE_STATBMP
    // 1) icon
    if (m_dialogStyle & wxICON_MASK)
    {
        wxStaticBitmap *icon = new wxStaticBitmap
                                   (
                                    this,
                                    wxID_ANY,
                                    wxArtProvider::GetMessageBoxIcon(m_dialogStyle)
                                   );
        if ( wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA )
            topsizer->Add( icon, 0, wxDirection::Top|wxDirection::Left|wxDirection::Right | wxAlignment::Left, 10 );
        else
            icon_text->Add(icon, wxSizerFlags().Top().Border(wxDirection::Right, 20));
    }
#endif // wxUSE_STATBMP

#if wxUSE_STATTEXT
    // 2) text

    wxBoxSizer * const textsizer = new wxBoxSizer(wxVERTICAL);

    // We want to show the main message in a different font to make it stand
    // out if the extended message is used as well. This looks better and is
    // more consistent with the native dialogs under MSW and GTK.
    std::string lowerMessage;
    if ( !m_extendedMessage.empty() )
    {
        wxTitleTextWrapper titleWrapper(this);
        textsizer->Add(CreateTextSizer(wxGetMessage(), titleWrapper),
                       wxSizerFlags().Border(wxDirection::Bottom, 20));

        lowerMessage = GetExtendedMessage();
    }
    else // no extended message
    {
        lowerMessage = wxGetMessage();
    }

    textsizer->Add(CreateTextSizer(lowerMessage));

    icon_text->Add(textsizer, 0, SizerFlags{wxAlignment::Center}, 10);
    topsizer->Add( icon_text, 1, SizerFlags{wxDirection::Left, wxDirection::Right, wxDirection::Top}, 10 );
#endif // wxUSE_STATTEXT

    // 3) optional checkbox and detailed text
    AddMessageDialogCheckBox( topsizer );
    AddMessageDialogDetails( topsizer );

    // 4) buttons
    wxSizer *sizerBtn = CreateMsgDlgButtonSizer();
    if ( sizerBtn )
        topsizer->Add(sizerBtn, 0, SizerFlags{wxStretch::Expand, wxDirection::All}, 10 );

    SetAutoLayout( true );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );
    wxSize size( GetSize() );
    if (size.x < size.y*3/2)
    {
        size.x = size.y*3/2;
        SetSize( size );
    }

    Centre( wxBOTH | wxCENTER_FRAME);
}

void wxGenericMessageDialog::OnYes(wxCommandEvent& WXUNUSED(event))
{
    EndModal( wxID_YES );
}

void wxGenericMessageDialog::OnNo(wxCommandEvent& WXUNUSED(event))
{
    EndModal( wxID_NO );
}

void wxGenericMessageDialog::OnHelp(wxCommandEvent& WXUNUSED(event))
{
    EndModal( wxID_HELP );
}

void wxGenericMessageDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    // Allow cancellation via ESC/Close button except if
    // only YES and NO are specified.
    const auto style = GetMessageDialogStyle();
    if ( (style & wxDialogFlags::Yes_No) != wxDialogFlags::Yes_No || (style & wxDialogFlags::Cancel) )
    {
        EndModal( wxID_CANCEL );
    }
}

int wxGenericMessageDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    if ( !m_created )
    {
        m_created = true;
        DoCreateMsgdialog();
    }

    return wxMessageDialogBase::ShowModal();
}

#endif // wxUSE_MSGDLG
