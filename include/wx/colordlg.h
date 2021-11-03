/////////////////////////////////////////////////////////////////////////////
// Name:        wx/colordlg.h
// Purpose:     wxColourDialog
// Author:      Vadim Zeitlin
// Modified by:
// Created:     01/02/97
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLORDLG_H_BASE_
#define _WX_COLORDLG_H_BASE_

#include "wx/defs.h"

#if wxUSE_COLOURDLG

#include "wx/colourdata.h"

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/colordlg.h"
#elif defined(__WXMAC__) && !defined(__WXUNIVERSAL__)
    #include "wx/osx/colordlg.h"
#elif defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/colordlg.h"
#elif defined(__WXQT__)
    #include "wx/qt/colordlg.h"
#else
    #include "wx/generic/colrdlgg.h"

    #define wxColourDialog wxGenericColourDialog
#endif

// Under some platforms (currently only wxMSW) wxColourDialog can send events
// of this type while it is shown.
//
// Notice that this class is almost identical to wxColourPickerEvent but it
// doesn't really sense to reuse the same class for both controls.
class wxColourDialogEvent : public wxCommandEvent
{
public:
    wxColourDialogEvent() = default;

    wxColourDialogEvent(wxEventType evtType,
                        wxColourDialog* dialog,
                        const wxColour& colour)
        : wxCommandEvent(evtType, dialog->GetId()),
          m_colour(colour)
    {
        SetEventObject(dialog);
    }

	wxColourDialogEvent& operator=(const wxColourDialogEvent&) = delete;

    // default copy ctor and dtor are ok

    wxColour GetColour() const { return m_colour; }
    void SetColour(const wxColour& colour) { m_colour = colour; }

    wxEvent *Clone() const override
    {
        return new wxColourDialogEvent(*this);
    }

private:
    wxColour m_colour;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COLOUR_CHANGED, wxColourDialogEvent);

#define wxColourDialogEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxColourDialogEventFunction, func)

#define EVT_COLOUR_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COLOUR_CHANGED, id, wxColourDialogEventHandler(fn))


// get the colour from user and return it
wxColour wxGetColourFromUser(wxWindow *parent = nullptr,
                                              const wxColour& colInit = wxNullColour,
                                              const wxString& caption = {},
                                              wxColourData *data = nullptr);

#endif // wxUSE_COLOURDLG

#endif
    // _WX_COLORDLG_H_BASE_
