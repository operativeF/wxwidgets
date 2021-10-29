/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/clipcmn.cpp
// Purpose:     common (to all ports) wxClipboard functions
// Author:      Robert Roebling
// Modified by:
// Created:     28.06.99
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_CLIPBOARD

#include "wx/clipbrd.h"
#include "wx/dataobj.h"
#include "wx/module.h"

// ---------------------------------------------------------
// wxClipboardEvent
// ---------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxClipboardEvent,wxEvent);

wxDEFINE_EVENT( wxEVT_CLIPBOARD_CHANGED, wxClipboardEvent );

bool wxClipboardEvent::SupportsFormat( const wxDataFormat &format ) const
{
#ifdef __WXGTK20__
    for (std::vector<wxDataFormat>::size_type n = 0; n < m_formats.size(); n++)
    {
        if (m_formats[n] == format)
            return true;
    }

    return false;
#else
    // All other ports just query the clipboard directly
    // from here
    wxClipboard* clipboard = (wxClipboard*) GetEventObject();
    return clipboard->IsSupported( format );
#endif
}

void wxClipboardEvent::AddFormat(const wxDataFormat& format)
{
    m_formats.push_back( format );
}

// ---------------------------------------------------------
// wxClipboardBase
// ---------------------------------------------------------

// FIXME: Global unique_ptr
static std::unique_ptr<wxClipboard> gs_clipboard;

/*static*/ wxClipboard *wxClipboardBase::Get()
{
    if ( !gs_clipboard )
    {
        gs_clipboard = std::make_unique<wxClipboard>();
    }

    return gs_clipboard.get();
}

bool wxClipboardBase::IsSupportedAsync( wxEvtHandler *sink )
{
    // We just imitate an asynchronous API on most platforms.
    // This method is overridden uner GTK.
    wxClipboardEvent *event = new wxClipboardEvent(wxEVT_CLIPBOARD_CHANGED);
    event->SetEventObject( this );

    sink->QueueEvent( event );

    return true;
}


// ----------------------------------------------------------------------------
// wxClipboardModule: module responsible for destroying the global clipboard
// object
// ----------------------------------------------------------------------------

class wxClipboardModule : public wxModule
{
public:
    bool OnInit() override { return true; }
    void OnExit() override {}

private:
    wxDECLARE_DYNAMIC_CLASS(wxClipboardModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxClipboardModule, wxModule);

#endif // wxUSE_CLIPBOARD
