/////////////////////////////////////////////////////////////////////////////
// Name:        wx/tglbtn.h
// Purpose:     This dummy header includes the proper header file for the
//              system we're compiling under.
// Author:      John Norris, minor changes by Axel Schlueter
// Modified by:
// Created:     08.02.01
// Copyright:   (c) 2000 Johnny C. Norris II
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TOGGLEBUTTON_H_BASE_
#define _WX_TOGGLEBUTTON_H_BASE_

#if wxUSE_TOGGLEBTN

#include "wx/app.h"
#include "wx/event.h"
#include "wx/anybutton.h"     // base class

wxDECLARE_EVENT( wxEVT_TOGGLEBUTTON, wxCommandEvent );

inline constexpr std::string_view wxToggleButtonNameStr = "toggle";


// ----------------------------------------------------------------------------
// wxToggleButtonBase
// ----------------------------------------------------------------------------

class wxToggleButtonBase : public wxAnyButton
{
public:
   wxToggleButtonBase& operator=(wxToggleButtonBase&&) = delete;

    // Get/set the value
    virtual void SetValue(bool state) = 0;
    virtual bool GetValue() const = 0;

    // The current "normal" state for the toggle button depends upon its value.
    State GetNormalState() const override
    {
        return GetValue() ? State_Pressed : State_Normal;
    }

    void UpdateWindowUI(unsigned int flags) override
    {
        wxControl::UpdateWindowUI(flags);

        if ( !IsShown() )
            return;

        wxWindow *tlw = wxGetTopLevelParent( this );
        if (tlw && wxPendingDelete.Member( tlw ))
           return;

        wxUpdateUIEvent event( GetId() );
        event.SetEventObject(this);

        if (GetEventHandler()->ProcessEvent(event) )
        {
            if ( event.GetSetChecked() )
                SetValue( event.GetChecked() );
        }
    }
};


#define EVT_TOGGLEBUTTON(id, fn) \
    wx__DECLARE_EVT1(wxEVT_TOGGLEBUTTON, id, wxCommandEventHandler(fn))

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/tglbtn.h"
#elif defined(__WXMSW__)
    #include "wx/msw/tglbtn.h"
    #define wxHAS_BITMAPTOGGLEBUTTON
#elif defined(__WXGTK20__)
    #include "wx/gtk/tglbtn.h"
    #define wxHAS_BITMAPTOGGLEBUTTON
#elif defined(__WXGTK__)
    #include "wx/gtk1/tglbtn.h"
# elif defined(__WXMOTIF__)
    #include "wx/motif/tglbtn.h"
#elif defined(__WXMAC__)
    #include "wx/osx/tglbtn.h"
    #define wxHAS_BITMAPTOGGLEBUTTON
#elif defined(__WXQT__)
    #include "wx/qt/tglbtn.h"
#endif

#endif // wxUSE_TOGGLEBTN

#endif // _WX_TOGGLEBUTTON_H_BASE_

