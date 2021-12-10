/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fontpicker.h
// Purpose:     wxFontPickerCtrl base header
// Author:      Francesco Montorsi
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONTPICKER_H_BASE_
#define _WX_FONTPICKER_H_BASE_

#if wxUSE_FONTPICKERCTRL

#include "wx/pickerbase.h"

import <string>;


class wxFontPickerEvent;

inline constexpr std::string_view wxFontPickerCtrlNameStr = "fontpicker";
inline constexpr std::string_view wxFontPickerWidgetNameStr = "fontpickerwidget";

// ----------------------------------------------------------------------------
// wxFontPickerWidgetBase: a generic abstract interface which must be
//                         implemented by controls used by wxFontPickerCtrl
// ----------------------------------------------------------------------------

class wxFontPickerWidgetBase
{
public:
    wxFontPickerWidgetBase() : m_selectedFont(*wxNORMAL_FONT) { }
    virtual ~wxFontPickerWidgetBase() = default;

    wxFont GetSelectedFont() const
        { return m_selectedFont; }
    virtual void SetSelectedFont(const wxFont &f)
        { m_selectedFont = f; UpdateFont(); }

    virtual wxColour GetSelectedColour() const = 0;
    virtual void SetSelectedColour(const wxColour &colour) = 0;

protected:

    virtual void UpdateFont() = 0;

    // the current font (may be invalid if none)
    // NOTE: don't call this m_font as wxWindow::m_font already exists
    wxFont m_selectedFont;
};

// Styles which must be supported by all controls implementing wxFontPickerWidgetBase
// NB: these styles must be defined to carefully-chosen values to
//     avoid conflicts with wxButton's styles


// keeps the label of the button updated with the fontface name + font size
// E.g. choosing "Times New Roman bold, italic with size 10" from the fontdialog,
//      updates the wxFontButtonGeneric's label (overwriting any previous label)
//      with the "Times New Roman, 10" text (only fontface + fontsize is displayed
//      to avoid extralong labels).
inline constexpr unsigned int wxFNTP_FONTDESC_AS_LABEL      = 0x0008;

// uses the currently selected font to draw the label of the button
inline constexpr unsigned int wxFNTP_USEFONT_FOR_LABEL      = 0x0010;

inline constexpr unsigned int wxFONTBTN_DEFAULT_STYLE = wxFNTP_FONTDESC_AS_LABEL | wxFNTP_USEFONT_FOR_LABEL;

// native version currently only exists in wxGTK2
#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/fontpicker.h"
    using wxFontPickerWidget = wxFontButton;
#else
    #include "wx/generic/fontpickerg.h"
    using wxFontPickerWidget = wxGenericFontButton;
#endif


// ----------------------------------------------------------------------------
// wxFontPickerCtrl specific flags
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxFNTP_USE_TEXTCTRL       = wxPB_USE_TEXTCTRL;
inline constexpr unsigned int wxFNTP_DEFAULT_STYLE      = wxFNTP_FONTDESC_AS_LABEL|wxFNTP_USEFONT_FOR_LABEL;

// not a style but rather the default value of the minimum/maximum pointsize allowed
inline constexpr auto wxFNTP_MINPOINT_SIZE = 0;
inline constexpr auto wxFNTP_MAXPOINT_SIZE = 100;


// ----------------------------------------------------------------------------
// wxFontPickerCtrl: platform-independent class which embeds the
// platform-dependent wxFontPickerWidget andm if wxFNTP_USE_TEXTCTRL style is
// used, a textctrl next to it.
// ----------------------------------------------------------------------------

class wxFontPickerCtrl : public wxPickerBase
{
public:
    wxFontPickerCtrl() = default;

    wxFontPickerCtrl(wxWindow *parent,
                     wxWindowID id,
                     const wxFont& initial = wxNullFont,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxFNTP_DEFAULT_STYLE,
                     const wxValidator& validator = {},
                     std::string_view name = wxFontPickerCtrlNameStr)
        : m_nMinPointSize(wxFNTP_MINPOINT_SIZE), m_nMaxPointSize(wxFNTP_MAXPOINT_SIZE)
    {
        Create(parent, id, initial, pos, size, style, validator, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxFont& initial = wxNullFont,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxFNTP_DEFAULT_STYLE,
                const wxValidator& validator = {},
                std::string_view name = wxFontPickerCtrlNameStr);


    // get the font chosen
    wxFont GetSelectedFont() const
        { return GetPickerWidget()->GetSelectedFont(); }

    // sets currently displayed font
    void SetSelectedFont(const wxFont& f);

    // returns the selected color
    wxColour GetSelectedColour() const
        { return GetPickerWidget()->GetSelectedColour(); }

    // sets the currently selected color
    void SetSelectedColour(const wxColour& colour)
        { GetPickerWidget()->SetSelectedColour(colour); }

    // set/get the min point size
    void SetMinPointSize(unsigned int min);
    unsigned int GetMinPointSize() const
        { return m_nMinPointSize; }

    // set/get the max point size
    void SetMaxPointSize(unsigned int max);
    unsigned int GetMaxPointSize() const
        { return m_nMaxPointSize; }

    void UpdatePickerFromTextCtrl() override;
    void UpdateTextCtrlFromPicker() override;

    // event handler for our picker
    void OnFontChange(wxFontPickerEvent &);

    // used to convert std::string <-> wxFont
    virtual std::string Font2String(const wxFont &font);
    virtual wxFont String2Font(const std::string &font);

protected:

    // extracts the style for our picker from wxFontPickerCtrl's style
    unsigned int GetPickerStyle(unsigned int style) const override
        { return (style & (wxFNTP_FONTDESC_AS_LABEL|wxFNTP_USEFONT_FOR_LABEL)); }

private:
    // the minimum pointsize allowed to the user
    unsigned int m_nMinPointSize{wxFNTP_MINPOINT_SIZE};

    // the maximum pointsize allowed to the user
    unsigned int m_nMaxPointSize{wxFNTP_MAXPOINT_SIZE};

    wxFontPickerWidget* GetPickerWidget() const
        { return dynamic_cast<wxFontPickerWidget*>(m_picker); }
};


// ----------------------------------------------------------------------------
// wxFontPickerEvent: used by wxFontPickerCtrl only
// ----------------------------------------------------------------------------

wxDECLARE_EVENT( wxEVT_FONTPICKER_CHANGED, wxFontPickerEvent );

class wxFontPickerEvent : public wxCommandEvent
{
public:
    wxFontPickerEvent() = default;
    wxFontPickerEvent(wxObject *generator, int id, const wxFont &f)
        : wxCommandEvent(wxEVT_FONTPICKER_CHANGED, id),
          m_font(f)
    {
        SetEventObject(generator);
    }

	wxFontPickerEvent& operator=(const wxFontPickerEvent&) = delete;

    wxFont GetFont() const { return m_font; }
    void SetFont(const wxFont &c) { m_font = c; }

    // default copy ctor, assignment operator and dtor are ok
    wxEvent *Clone() const override { return new wxFontPickerEvent(*this); }

private:
    wxFont m_font;
};

// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*wxFontPickerEventFunction)(wxFontPickerEvent&);

#define wxFontPickerEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxFontPickerEventFunction, func)

#define EVT_FONTPICKER_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FONTPICKER_CHANGED, id, wxFontPickerEventHandler(fn))

#endif // wxUSE_FONTPICKERCTRL

#endif
    // _WX_FONTPICKER_H_BASE_
