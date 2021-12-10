/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/combo.h
// Purpose:     Generic wxComboCtrl
// Author:      Jaakko Salli
// Modified by:
// Created:     Apr-30-2006
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_COMBOCTRL_H_
#define _WX_GENERIC_COMBOCTRL_H_

#if wxUSE_COMBOCTRL

// Only define generic if native doesn't have all the features
#ifndef wxCOMBOCONTROL_FULLY_FEATURED

#include "wx/containr.h"

import <string>;

// ----------------------------------------------------------------------------
// Generic wxComboCtrl
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)

// all actions of single line text controls are supported

// popup/dismiss the choice window
inline constexpr wxChar wxACTION_COMBOBOX_POPUP[]     = "popup";
inline constexpr wxChar wxACTION_COMBOBOX_DISMISS[]   = "dismiss";

#endif

class wxGenericComboCtrl
    : public wxNavigationEnabled<wxComboCtrlBase>
{
public:
    
    wxGenericComboCtrl() { Init(); }

    wxGenericComboCtrl(wxWindow *parent,
                       wxWindowID id = wxID_ANY,
                       const std::string& value = {},
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = 0,
                       const wxValidator& validator = {},
                       const std::string& name = wxComboBoxNameStr)
    {
        Init();

        (void)Create(parent, id, value, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                std::string_view value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxComboBoxNameStr);

    void SetCustomPaintWidth( int width );

    bool IsKeyPopupToggle(const wxKeyEvent& event) const override;

    static int GetFeatures() { return wxComboCtrlFeatures::All; }

#if defined(__WXUNIVERSAL__)
    // we have our own input handler and our own actions
    virtual bool PerformAction(const wxControlAction& action,
                               long numArg = 0l,
                               const std::string& strArg = {});
#endif

protected:

    // Dummies for platform-specific wxTextEntry implementations
#if defined(__WXUNIVERSAL__)
    // Looks like there's nothing we need to override here
#elif defined(__WXMOTIF__)
    virtual WXWidget GetTextWidget() const { return NULL; }
#elif defined(__WXGTK__)
#if defined(__WXGTK20__)
    GtkEditable *GetEditable() const override { return NULL; }
    GtkEntry *GetEntry() const override { return NULL; }
#endif
#elif defined(__WXOSX__)
    wxTextWidgetImpl * GetTextPeer() const override;
#endif

    // For better transparent background rendering
    bool HasTransparentBackground() override;

    // Mandatory virtuals
    void OnResize() override;

    // Event handlers
    void OnPaintEvent( wxPaintEvent& event );
    void OnMouseEvent( wxMouseEvent& event );

private:
    void Init();

    wxDECLARE_EVENT_TABLE();
};


#ifndef _WX_COMBOCONTROL_H_

// If native wxComboCtrl was not defined, then prepare a simple
// front-end so that wxRTTI works as expected.

class wxComboCtrl : public wxGenericComboCtrl
{
public:
    wxComboCtrl() = default;

    wxComboCtrl(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const std::string& value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                const std::string& name = wxComboBoxNameStr)
        : wxGenericComboCtrl()
    {
        std::ignore = Create(parent, id, value, pos, size, style, validator, name);
    }
};

#endif // _WX_COMBOCONTROL_H_

#else

#define wxGenericComboCtrl   wxComboCtrl

#endif // !defined(wxCOMBOCONTROL_FULLY_FEATURED)

#endif // wxUSE_COMBOCTRL
#endif
    // _WX_GENERIC_COMBOCTRL_H_
