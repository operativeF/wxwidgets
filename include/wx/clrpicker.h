/////////////////////////////////////////////////////////////////////////////
// Name:        wx/clrpicker.h
// Purpose:     wxColourPickerCtrl base header
// Author:      Francesco Montorsi (based on Vadim Zeitlin's code)
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Vadim Zeitlin, Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CLRPICKER_H_BASE_
#define _WX_CLRPICKER_H_BASE_

#if wxUSE_COLOURPICKERCTRL

#include "wx/colour.h"
#include "wx/pickerbase.h"

import <string>;


class wxColourPickerEvent;

inline constexpr std::string_view wxColourPickerCtrlNameStr = "colourpicker";
inline constexpr std::string_view wxColourPickerWidgetNameStr = "colourpickerwidget";

// show the colour in HTML form (#AABBCC) as colour button label
inline constexpr unsigned int wxCLRBTN_SHOW_LABEL = 100;

// the default style
inline constexpr unsigned int wxCLRBTN_DEFAULT_STYLE = wxCLRBTN_SHOW_LABEL;



// ----------------------------------------------------------------------------
// wxColourPickerWidgetBase: a generic abstract interface which must be
//                           implemented by controls used by wxColourPickerCtrl
// ----------------------------------------------------------------------------

class wxColourPickerWidgetBase
{
public:
    wxColourPickerWidgetBase() : m_colour(*wxBLACK) {}
    virtual ~wxColourPickerWidgetBase() = default;

    wxColour GetColour() const
        { return m_colour; }
    virtual void SetColour(const wxColour &col)
        { m_colour = col; UpdateColour(); }
    virtual void SetColour(const std::string& col)
        { m_colour.Set(col); UpdateColour(); }

protected:
    virtual void UpdateColour() = 0;

    // the current colour (may be invalid if none)
    wxColour m_colour;
};


// Styles which must be supported by all controls implementing wxColourPickerWidgetBase
// NB: these styles must be defined to carefully-chosen values to
//     avoid conflicts with wxButton's styles

// show the colour in HTML form (#AABBCC) as colour button label
// (instead of no label at all)
// NOTE: this style is supported just by wxColourButtonGeneric and
//       thus is not exposed in wxColourPickerCtrl
inline constexpr unsigned int wxCLRP_SHOW_LABEL             = 0x0008;
inline constexpr unsigned int wxCLRP_SHOW_ALPHA             = 0x0010;

// map platform-dependent controls which implement the wxColourPickerWidgetBase
// under the name "wxColourPickerWidget".
// NOTE: wxColourPickerCtrl allocates a wxColourPickerWidget and relies on the
//       fact that all classes being mapped as wxColourPickerWidget have the
//       same prototype for their constructor (and also explains why we use
//       define instead of a typedef)
// since GTK > 2.4, there is GtkColorButton
#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/clrpicker.h"
    using wxColourPickerWidget = wxColourButton;
#elif defined(__WXQT__) && !defined(__WXUNIVERSAL__)
    #include "wx/qt/clrpicker.h"
#else
    #include "wx/generic/clrpickerg.h"
    using wxColourPickerWidget = wxGenericColourButton;
#endif


// ----------------------------------------------------------------------------
// wxColourPickerCtrl: platform-independent class which embeds a
// platform-dependent wxColourPickerWidget and, if wxCLRP_USE_TEXTCTRL style is
// used, a textctrl next to it.
// ----------------------------------------------------------------------------

#define wxCLRP_USE_TEXTCTRL       (wxPB_USE_TEXTCTRL)
#define wxCLRP_DEFAULT_STYLE      0

class wxColourPickerCtrl : public wxPickerBase
{
public:
    wxColourPickerCtrl() = default;

    wxColourPickerCtrl(wxWindow *parent, wxWindowID id,
        const wxColour& col = *wxBLACK, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, unsigned int style = wxCLRP_DEFAULT_STYLE,
        const wxValidator& validator = {},
        std::string_view name = wxColourPickerCtrlNameStr)
        { Create(parent, id, col, pos, size, style, validator, name); }

    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
           const wxColour& col = *wxBLACK,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize,
           unsigned int style = wxCLRP_DEFAULT_STYLE,
           const wxValidator& validator = {},
           std::string_view name = wxColourPickerCtrlNameStr);

    // get the colour chosen
    wxColour GetColour() const
        { return ((wxColourPickerWidget *)m_picker)->GetColour(); }

    // set currently displayed color
    void SetColour(const wxColour& col);

    // set colour using RGB(r,g,b) syntax or considering given text as a colour name;
    // returns true if the given text was successfully recognized.
    bool SetColour(const std::string& text);

    // update the button colour to match the text control contents
    void UpdatePickerFromTextCtrl() override;

    // update the text control to match the button's colour
    void UpdateTextCtrlFromPicker() override;

    // event handler for our picker
    void OnColourChange(wxColourPickerEvent &);

protected:
    unsigned int GetPickerStyle(unsigned int style) const override
        { return (style & (wxCLRP_SHOW_LABEL | wxCLRP_SHOW_ALPHA)); }
};


// ----------------------------------------------------------------------------
// wxColourPickerEvent: used by wxColourPickerCtrl only
// ----------------------------------------------------------------------------

wxDECLARE_EVENT( wxEVT_COLOURPICKER_CHANGED, wxColourPickerEvent );
wxDECLARE_EVENT( wxEVT_COLOURPICKER_CURRENT_CHANGED, wxColourPickerEvent );
wxDECLARE_EVENT( wxEVT_COLOURPICKER_DIALOG_CANCELLED, wxColourPickerEvent );

class wxColourPickerEvent : public wxCommandEvent
{
public:
    wxColourPickerEvent() = default;
    wxColourPickerEvent(wxObject *generator, int id, const wxColour &col, wxEventType commandType = wxEVT_COLOURPICKER_CHANGED)
        : wxCommandEvent(commandType, id),
          m_colour(col)
    {
        SetEventObject(generator);
    }

    wxColour GetColour() const { return m_colour; }
    void SetColour(const wxColour &c) { m_colour = c; }

    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxColourPickerEvent>(*this); }

private:
    wxColour m_colour;
};

// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*wxColourPickerEventFunction)(wxColourPickerEvent&);

#define wxColourPickerEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxColourPickerEventFunction, func)

#define EVT_COLOURPICKER_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COLOURPICKER_CHANGED, id, wxColourPickerEventHandler(fn))

#define EVT_COLOURPICKER_CURRENT_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COLOURPICKER_CURRENT_CHANGED, id, wxColourPickerEventHandler(fn))

#define EVT_COLOURPICKER_DIALOG_CANCELLED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COLOURPICKER_DIALOG_CANCELLED, id, wxColourPickerEventHandler(fn))

#endif // wxUSE_COLOURPICKERCTRL

#endif // _WX_CLRPICKER_H_BASE_

