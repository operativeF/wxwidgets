///////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/popupwin.h
// Purpose:     wxPopupWindow class for wxMotif
// Author:      Mattia Barbon
// Modified by:
// Created:     28.08.03
// Copyright:   (c) 2003 Mattia Barbon
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MOTIF_POPUPWIN_H_
#define _WX_MOTIF_POPUPWIN_H_

// ----------------------------------------------------------------------------
// wxPopupWindow
// ----------------------------------------------------------------------------

class wxPopupWindow : public wxPopupWindowBase
{
public:
    wxPopupWindow() { Init(); }

    wxPopupWindow( wxWindow *parent, int flags = wxBORDER_NONE )
        { Init(); (void)Create( parent, flags ); }

	wxPopupWindow(const wxPopupWindow&) = delete;
	wxPopupWindow& operator=(const wxPopupWindow&) = delete;

    bool Create( wxWindow *parent, int flags = wxBORDER_NONE );

    virtual bool Show( bool show = true );
private:
    void Init() { m_isShown = false; }

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MOTIF_POPUPWIN_H_
