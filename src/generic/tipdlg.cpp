///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/tipdlg.cpp
// Purpose:     implementation of wxTipDialog
// Author:      Vadim Zeitlin
// Modified by:
// Created:     28.06.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_STARTUP_TIPS

#include "wx/button.h"
#include "wx/checkbox.h"
#include "wx/statbox.h"
#include "wx/dialog.h"
#include "wx/icon.h"
#include "wx/intl.h"
#include "wx/textctrl.h"
#include "wx/statbmp.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/statline.h"
#include "wx/artprov.h"
#include "wx/tipdlg.h"

import WX.Utils.Settings;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr int wxID_NEXT_TIP = 32000;  // whatever

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// an implementation which takes the tips from the text file - each line
// represents a tip
#if wxUSE_TEXTFILE
class wxFileTipProvider : public wxTipProvider
{
public:
    wxFileTipProvider(const std::string& filename, size_t currentTip);

    wxFileTipProvider(const wxFileTipProvider&) = delete;
	wxFileTipProvider& operator=(const wxFileTipProvider&) = delete;

    std::string GetTip() override;

private:
    wxTextFile m_textfile;
};
#endif // wxUSE_TEXTFILE

#ifdef __WIN32__
// TODO an implementation which takes the tips from the given registry key
class wxRegTipProvider : public wxTipProvider
{
public:
    explicit wxRegTipProvider(const std::string& keyname);

    std::string GetTip() override;
};

// Empty implementation for now to keep the linker happy
std::string wxRegTipProvider::GetTip()
{
    return "";
}

#endif // __WIN32__

// the dialog we show in wxShowTip()
class wxTipDialog : public wxDialog
{
public:
    wxTipDialog(wxWindow *parent,
                wxTipProvider *tipProvider,
                bool showAtStartup);

    wxTipDialog(const wxTipDialog&) = delete;
	wxTipDialog& operator=(const wxTipDialog&) = delete;

    // the tip dialog has "Show tips on startup" checkbox - return true if it
    // was checked (or wasn't unchecked)
    bool ShowTipsOnStartup() const { return m_checkbox->GetValue(); }

    // sets the (next) tip text
    void SetTipText() { m_text->SetValue(m_tipProvider->GetTip()); }

    // "Next" button handler
    void OnNextTip([[maybe_unused]] wxCommandEvent& event) { SetTipText(); }

private:
    wxTipProvider *m_tipProvider;

    wxTextCtrl *m_text;
    wxCheckBox *m_checkbox;

    wxDECLARE_EVENT_TABLE();
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxFileTipProvider
// ----------------------------------------------------------------------------
#if wxUSE_TEXTFILE
wxFileTipProvider::wxFileTipProvider(const std::string& filename,
                                     size_t currentTip)
                 : wxTipProvider(currentTip), m_textfile(filename)
{
    m_textfile.Open();
}

std::string wxFileTipProvider::GetTip()
{
    size_t count = m_textfile.GetLineCount();
    if ( !count )
    {
        return _("Tips not available, sorry!").ToStdString();
    }

    wxString tip;

    // Comments start with a # symbol.
    // Loop reading lines until get the first one that isn't a comment.
    // The max number of loop executions is the number of lines in the
    // textfile so that can't go into an infinite loop in the [oddball]
    // case of a comment-only tips file.
    for ( size_t i=0; i < count; i++ )
    {
        // The current tip may be at the last line of the textfile, (or
        // past it, if the number of lines in the textfile changed, such
        // as changing to a different textfile, with less tips). So check
        // to see at last line of text file, (or past it)...
        if ( m_currentTip >= count )
        {
            // .. and if so, wrap back to line 0.
            m_currentTip = 0;
        }

        // Read the tip, and increment the current tip counter.
        tip = m_textfile.GetLine(m_currentTip++);

        // Break if tip isn't a comment, and isn't an empty string
        // (or only stray space characters).
        if ( !tip.StartsWith("#") && !tip.Trim().empty() )
        {
            break;
        }
    }

    // If tip starts with '_(', then it is a gettext string of format
    // _("My \"global\" tip text") so first strip off the leading '_("'...
    if ( tip.StartsWith(wxT("_(\"" ), &tip))
    {
        //...and strip off the trailing '")'...
        tip = tip.BeforeLast(wxT('\"'));
        // ...and replace escaped quotes
        tip.Replace("\\\"", "\"");

        // and translate it as requested
        tip = wxGetTranslation(tip);
    }

    return tip.ToStdString();
}
#endif // wxUSE_TEXTFILE

// ----------------------------------------------------------------------------
// wxTipDialog
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxTipDialog, wxDialog)
    EVT_BUTTON(wxID_NEXT_TIP, wxTipDialog::OnNextTip)
wxEND_EVENT_TABLE()

wxTipDialog::wxTipDialog(wxWindow *parent,
                         wxTipProvider *tipProvider,
                         bool showAtStartup)
           : wxDialog(GetParentForModalDialog(parent, 0), wxID_ANY, _("Tip of the Day").ToStdString(),
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
                      )
{
    m_tipProvider = tipProvider;
    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

    // 1) create all controls in tab order

    wxStaticText *text = new wxStaticText(this, wxID_ANY, _("Did you know...").ToStdString());

    if (!isPda)
    {
        wxFont font = text->GetFont();
        font.SetFractionalPointSize(1.6 * font.GetFractionalPointSize());
        font.SetWeight(wxFONTWEIGHT_BOLD);
        text->SetFont(font);
    }

    m_text = new wxTextCtrl(this, wxID_ANY, "",
                            wxDefaultPosition, wxSize(200, 160),
                            wxTE_MULTILINE |
                            wxTE_READONLY |
                            wxTE_NO_VSCROLL |
                            wxTE_RICH2 | // a hack to get rid of vert scrollbar
                            wxDEFAULT_CONTROL_BORDER
                            );
#if defined(__WXMSW__)
    m_text->SetFont(wxFont(12, wxFontFamily::Swiss, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL));
#endif

    wxIcon icon = wxArtProvider::GetIcon(wxART_TIP, wxART_CMN_DIALOG);
    wxStaticBitmap *bmp = new wxStaticBitmap(this, wxID_ANY, icon);

    m_checkbox = new wxCheckBox(this, wxID_ANY, _("&Show tips at startup").ToStdString());
    m_checkbox->SetValue(showAtStartup);
    m_checkbox->SetFocus();

    wxButton *btnNext = new wxButton(this, wxID_NEXT_TIP, _("&Next Tip").ToStdString());

    wxButton *btnClose = new wxButton(this, wxID_CLOSE);
    SetAffirmativeId(wxID_CLOSE);


    // 2) put them in boxes

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );
    icon_text->Add( bmp, 0, wxCENTER );
    icon_text->Add( text, 1, wxCENTER | wxLEFT, 20 );
    topsizer->Add( icon_text, 0, wxEXPAND | wxALL, 10 );

    topsizer->Add( m_text, 1, wxEXPAND | wxLEFT|wxRIGHT, 10 );

    wxBoxSizer *bottom = new wxBoxSizer( wxHORIZONTAL );
    if (isPda)
        topsizer->Add( m_checkbox, 0, wxCENTER|wxTOP );
    else
        bottom->Add( m_checkbox, 0, wxCENTER );

    if (!isPda)
        bottom->Add( 10,10,1 );
    bottom->Add( btnNext, 0, wxCENTER | wxLEFT, 10 );
    bottom->Add( btnClose, 0, wxCENTER | wxLEFT, 10 );

    if (isPda)
        topsizer->Add( bottom, 0, wxCENTER | wxALL, 5 );
    else
        topsizer->Add( bottom, 0, wxEXPAND | wxALL, 10 );

    SetTipText();

    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre(wxBOTH | wxCENTER_FRAME);
}

// ----------------------------------------------------------------------------
// our public interface
// ----------------------------------------------------------------------------

#if wxUSE_TEXTFILE
wxTipProvider *wxCreateFileTipProvider(const std::string& filename,
                                       size_t currentTip)
{
    return new wxFileTipProvider(filename, currentTip);
}
#endif // wxUSE_TEXTFILE

bool wxShowTip(wxWindow *parent,
               wxTipProvider *tipProvider,
               bool showAtStartup)
{
    wxTipDialog dlg(parent, tipProvider, showAtStartup);
    dlg.ShowModal();

    return dlg.ShowTipsOnStartup();
}

#endif // wxUSE_STARTUP_TIPS
