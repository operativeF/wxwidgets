///////////////////////////////////////////////////////////////////////////////
// Name:        wx/fontdlg.h
// Purpose:     common interface for different wxFontDialog classes
// Author:      Vadim Zeitlin
// Modified by:
// Created:     12.05.02
// Copyright:   (c) 1997-2002 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONTDLG_H_BASE_
#define _WX_FONTDLG_H_BASE_

#if wxUSE_FONTDLG

#include "wx/dialog.h"          // the base class
#include "wx/fontdata.h"

import <string>;

// ----------------------------------------------------------------------------
// wxFontDialog interface
// ----------------------------------------------------------------------------

class wxFontDialogBase : public wxDialog
{
public:
    // create the font dialog
    wxFontDialogBase() = default;
    wxFontDialogBase(wxWindow *parent) { m_parent = parent; }
    wxFontDialogBase(wxWindow *parent, const wxFontData& data)
        { m_parent = parent; InitFontData(&data); }

    wxFontDialogBase& operator=(wxFontDialogBase&&) = delete;
    
    [[maybe_unused]] bool Create(wxWindow *parent)
        { return DoCreate(parent); }
    [[maybe_unused]] bool Create(wxWindow *parent, const wxFontData& data)
        { InitFontData(&data); return Create(parent); }

    // retrieve the font data
    const wxFontData& GetFontData() const { return m_fontData; }
    wxFontData& GetFontData() { return m_fontData; }

protected:
    virtual bool DoCreate(wxWindow *parent) { m_parent = parent; return true; }

    void InitFontData(const wxFontData *data = nullptr)
        { if ( data ) m_fontData = *data; }

    wxFontData m_fontData;
};

// ----------------------------------------------------------------------------
// platform-specific wxFontDialog implementation
// ----------------------------------------------------------------------------

#if defined( __WXOSX_MAC__ )
//set to 1 to use native mac font and color dialogs
#define USE_NATIVE_FONT_DIALOG_FOR_MACOSX 1
#else
//not supported on these platforms, leave 0
#define USE_NATIVE_FONT_DIALOG_FOR_MACOSX 0
#endif

#if defined(__WXUNIVERSAL__) || \
    defined(__WXMOTIF__)     || \
    defined(__WXGPE__)

    #include "wx/generic/fontdlgg.h"
    using wxFontDialog = wxGenericFontDialog;
#elif defined(__WXMSW__)
    #include "wx/msw/fontdlg.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/fontdlg.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/fontdlg.h"
#elif defined(__WXMAC__)
    #include "wx/osx/fontdlg.h"
#elif defined(__WXQT__)
    #include "wx/qt/fontdlg.h"
#endif

// ----------------------------------------------------------------------------
// global public functions
// ----------------------------------------------------------------------------

// get the font from user and return it, returns wxNullFont if the dialog was
// cancelled
wxFont wxGetFontFromUser(wxWindow *parent = nullptr,
                                          const wxFont& fontInit = wxNullFont,
                                          const std::string& caption = {});

#endif // wxUSE_FONTDLG

#endif
    // _WX_FONTDLG_H_BASE_
