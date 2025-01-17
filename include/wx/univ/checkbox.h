///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/checkbox.h
// Purpose:     wxCheckBox declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     07.09.00
// Copyright:   (c) 2000 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_CHECKBOX_H_
#define _WX_UNIV_CHECKBOX_H_

#include "wx/button.h" // for wxStdButtonInputHandler

// ----------------------------------------------------------------------------
// the actions supported by wxCheckBox
// ----------------------------------------------------------------------------

constexpr wxChar wxACTION_CHECKBOX_CHECK[]   = wxT("check");   // SetValue(true)
constexpr wxChar wxACTION_CHECKBOX_CLEAR[]   = wxT("clear");   // SetValue(false)
constexpr wxChar wxACTION_CHECKBOX_TOGGLE[]  = wxT("toggle");  // toggle the check state

// additionally it accepts wxACTION_BUTTON_PRESS and RELEASE

// ----------------------------------------------------------------------------
// wxCheckBox
// ----------------------------------------------------------------------------

class wxCheckBox : public wxCheckBoxBase
{
public:
    // checkbox constants
    enum State
    {
        State_Normal,
        State_Pressed,
        State_Disabled,
        State_Current,
        State_Max
    };

    enum Status
    {
        Status_Checked,
        Status_Unchecked,
        Status_3rdState,
        Status_Max
    };

    // constructors
    wxCheckBox() { Init(); }

    wxCheckBox(wxWindow *parent,
               wxWindowID id,
               const wxString& label,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxASCII_STR(wxCheckBoxNameStr))
    {
        Init();

        Create(parent, id, label, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxCheckBoxNameStr));

    // implement the checkbox interface
    void SetValue(bool value) override;
    bool GetValue() const override;

    // set/get the bitmaps to use for the checkbox indicator
    void SetBitmap(const wxBitmap& bmp, State state, Status status);
    virtual wxBitmap GetBitmap(State state, Status status) const;

    // wxCheckBox actions
    void Toggle();
    virtual void Press();
    virtual void Release();
    virtual void ChangeValue(bool value);

    // overridden base class virtuals
    bool IsPressed() const override { return m_isPressed; }

    virtual bool PerformAction(const wxControlAction& action,
                               long numArg = -1,
                               const wxString& strArg = wxEmptyString) override;

    bool CanBeHighlighted() const override { return true; }
    virtual wxInputHandler *CreateStdInputHandler(wxInputHandler *handlerDef);
    wxInputHandler *DoGetStdInputHandler(wxInputHandler *handlerDef) override
    {
        return CreateStdInputHandler(handlerDef);
    }

protected:
    void DoSet3StateValue(wxCheckBoxState WXUNUSED(state)) override;
    wxCheckBoxState DoGet3StateValue() const override;

    void DoDraw(wxControlRenderer *renderer) override;
    wxSize DoGetBestClientSize() const override;

    // get the size of the bitmap using either the current one or the default
    // one (query renderer then)
    virtual wxSize GetBitmapSize() const;

    // common part of all ctors
    void Init();

    // send command event notifying about the checkbox state change
    virtual void SendEvent();

    // called when the checkbox becomes checked - radio button hook
    virtual void OnCheck();

    // get the state corresponding to the flags (combination of wxCONTROL_XXX)
    wxCheckBox::State GetState(int flags) const;

    // directly access the bitmaps array without trying to find a valid bitmap
    // to use as GetBitmap() does
    wxBitmap DoGetBitmap(State state, Status status) const
        { return m_bitmaps[state][status]; }

    // get the current status
    Status GetStatus() const { return m_status; }

private:
    // the current check status
    Status m_status;

    // the bitmaps to use for the different states
    wxBitmap m_bitmaps[State_Max][Status_Max];

    // is the checkbox currently pressed?
    bool m_isPressed;

    wxDECLARE_DYNAMIC_CLASS(wxCheckBox);
};

#endif // _WX_UNIV_CHECKBOX_H_
