/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/spinbutt.h
// Purpose:     wxSpinButton class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SPINBUTT_H_
#define _WX_SPINBUTT_H_

#include "wx/control.h"
#include "wx/event.h"

/*
    The wxSpinButton is like a small scrollbar than is often placed next
    to a text control.

    wxSP_HORIZONTAL:   horizontal spin button
    wxSP_VERTICAL:     vertical spin button (the default)
    wxSP_ARROW_KEYS:   arrow keys increment/decrement value
    wxSP_WRAP:         value wraps at either end
 */

class wxSpinButton : public wxSpinButtonBase
{
public:
    // construction
    wxSpinButton();

    wxSpinButton(wxWindow *parent,
                 wxWindowID id = -1,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxSP_VERTICAL | wxSP_ARROW_KEYS,
                 const wxString& name = wxT("wxSpinButton"))
    {
        Create(parent, id, pos, size, style, name);
    }

    virtual ~wxSpinButton();

    bool Create(wxWindow *parent,
                wxWindowID id = -1,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxSP_VERTICAL | wxSP_ARROW_KEYS,
                const wxString& name = wxT("wxSpinButton"));


    void SetRange(int minVal, int maxVal) override;
    int GetValue() const override;
    void SetValue(int val) override;

    void TriggerScrollEvent( wxEventType scrollEvent ) override;

    // osx specific event handling common for all osx-ports

    bool OSXHandleClicked( double timestampsec ) override;

protected:
    void         SendThumbTrackEvent() ;

    wxSize DoGetBestSize() const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxSpinButton);
};

#endif
    // _WX_SPINBUTT_H_
