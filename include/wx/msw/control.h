/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/control.h
// Purpose:     wxControl class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CONTROL_H_
#define _WX_CONTROL_H_

import WX.WinDef;
import WX.Cfg.Flags;

import <string>;
import <vector>;

// General item class
class wxControl : public wxControlBase
{
public:
    wxControl() = default;
    wxControl(wxWindow *parent, wxWindowID id,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize, unsigned int style = 0,
              const wxValidator& validator = {},
              std::string_view name = wxControlNameStr)
    {
        Create(parent, id, pos, size, style, validator, name);
    }

    wxControl& operator=(wxControl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, unsigned int style = 0,
            const wxValidator& validator = {},
            std::string_view name = wxControlNameStr);


    // Simulates an event
    void Command(wxCommandEvent& event) override { ProcessCommand(event); }

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal);

    // Calls the callback and appropriate event handlers
    bool ProcessCommand(wxCommandEvent& event);

    // MSW-specific
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;

    // For ownerdraw items
    virtual bool MSWOnDraw([[maybe_unused]] WXDRAWITEMSTRUCT *item) { return false; }
    virtual bool MSWOnMeasure([[maybe_unused]] WXMEASUREITEMSTRUCT *item) { return false; }

    const std::vector<long>& GetSubcontrols() const { return m_subControls; }

    // default handling of WM_CTLCOLORxxx: this is public so that wxWindow
    // could call it
    virtual WXHBRUSH MSWControlColor(WXHDC pDC, WXHWND hWnd);

    // default style for the control include WS_TABSTOP if it AcceptsFocus()
    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

protected:
    // Hook for common controls for which we don't want to set the default font
    // as if we do set it, the controls don't update their font size
    // automatically in response to WM_SETTINGCHANGE if it's changed in the
    // display properties in the control panel, so avoid doing this for them.
    virtual bool MSWShouldSetDefaultFont() const { return true; }

    // choose the default border for this window
    wxBorder GetDefaultBorder() const override;

    // return default best size (doesn't really make any sense, override this)
    wxSize DoGetBestSize() const override;

    // create the control of the given Windows class: this is typically called
    // from Create() method of the derived class passing its label, pos and
    // size parameter (style parameter is not needed because m_windowStyle is
    // supposed to had been already set and so is used instead when this
    // function is called)
    bool MSWCreateControl(const std::string& classname,
                          const std::string& label,
                          const wxPoint& pos,
                          const wxSize& size);

    // NB: the method below is deprecated now, with MSWGetStyle() the method
    //     above should be used instead! Once all the controls are updated to
    //     implement MSWGetStyle() this version will disappear.
    //
    // create the control of the given class with the given style (combination
    // of WS_XXX flags, i.e. Windows style, not wxWidgets one), returns
    // false if creation failed
    //
    // All parameters except classname and style are optional, if the
    // size/position are not given, they should be set later with SetSize()
    // and, label (the title of the window), of course, is left empty. The
    // extended style is determined from the style and the app 3D settings
    // automatically if it's not specified explicitly.
    bool MSWCreateControl(const std::string& classname,
                          WXDWORD style,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          const std::string& label = {},
                          WXDWORD exstyle = (WXDWORD)-1);

    // call this from the derived class MSWControlColor() if you want to show
    // the control greyed out (and opaque)
    WXHBRUSH MSWControlColorDisabled(WXHDC pDC);

    // common part of the 3 functions above: pass wxNullColour to use the
    // appropriate background colour (meaning ours or our parents) or a fixed
    // one
    virtual WXHBRUSH DoMSWControlColor(WXHDC pDC, wxColour colBg, WXHWND hWnd);

    // Look in our GetSubcontrols() for the windows with the given ID.
    wxWindow *MSWFindItem(long id, WXHWND hWnd) const override;


    // for controls like radiobuttons which are really composite this array
    // holds the ids (not HWNDs!) of the sub controls
    std::vector<long> m_subControls;
};

#endif // _WX_CONTROL_H_
