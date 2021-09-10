/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/combo.h
// Purpose:     wxComboCtrl class
// Author:      Jaakko Salli
// Modified by:
// Created:     Apr-30-2006
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COMBOCONTROL_H_
#define _WX_COMBOCONTROL_H_

// NB: Definition of _WX_COMBOCONTROL_H_ is used in wx/generic/combo.h to
//     determine whether there is native wxComboCtrl, so make sure you
//     use it in all native wxComboCtrls.

#if wxUSE_COMBOCTRL

#if wxUSE_TIMER
    #include "wx/timer.h"
    #define wxUSE_COMBOCTRL_POPUP_ANIMATION     1
#endif

#include <string>


// ----------------------------------------------------------------------------
// Native wxComboCtrl
// ----------------------------------------------------------------------------

// Define this only if native implementation includes all features
#define wxCOMBOCONTROL_FULLY_FEATURED

class WXDLLIMPEXP_CORE wxComboCtrl : public wxComboCtrlBase
{
public:
    
    wxComboCtrl()  = default;

    wxComboCtrl(wxWindow *parent,
                   wxWindowID id = wxID_ANY,
                   const std::string& value = {},
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const std::string& name = wxComboCtrlNameStr)
         
    {
        Create(parent, id, value, pos, size, style, validator, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const std::string& value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxComboCtrlNameStr);

    ~wxComboCtrl() = default;

    void PrepareBackground( wxDC& dc, const wxRect& rect, int flags ) const override;
    bool IsKeyPopupToggle(const wxKeyEvent& event) const override;

    static int GetFeatures() { return wxComboCtrlFeatures::All; }

#if wxUSE_COMBOCTRL_POPUP_ANIMATION
    void OnTimerEvent(wxTimerEvent& WXUNUSED(event)) { DoTimerEvent(); }

protected:
    void DoTimerEvent();

    bool AnimateShow( const wxRect& rect, int flags ) override;
#endif // wxUSE_COMBOCTRL_POPUP_ANIMATION

protected:

    // Dummy method - we override all functions that call this
    WXHWND GetEditHWND() const override { return nullptr; }

    // customization
    void OnResize() override;
    wxCoord GetNativeTextIndent() const override;

    // event handlers
    void OnPaintEvent( wxPaintEvent& event );
    void OnMouseEvent( wxMouseEvent& event );

    bool HasTransparentBackground() override { return IsDoubleBuffered(); }

private:

#if wxUSE_COMBOCTRL_POPUP_ANIMATION
    // Popup animation related
    wxMilliClock_t m_animStart;
    wxTimer     m_animTimer;
    wxRect      m_animRect;
    int         m_animFlags{};
#endif

    wxDECLARE_EVENT_TABLE();

    wxDECLARE_DYNAMIC_CLASS(wxComboCtrl);
};


#endif // wxUSE_COMBOCTRL
#endif
    // _WX_COMBOCONTROL_H_
