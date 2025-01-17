///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/spinbutt.h
// Purpose:     universal version of wxSpinButton
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.01.01
// Copyright:   (c) 2001 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_SPINBUTT_H_
#define _WX_UNIV_SPINBUTT_H_

#include "wx/univ/scrarrow.h"

// ----------------------------------------------------------------------------
// wxSpinButton
// ----------------------------------------------------------------------------

// actions supported by this control
constexpr wxChar wxACTION_SPIN_INC[]    = wxT("inc");
constexpr wxChar wxACTION_SPIN_DEC[]    = wxT("dec");

class wxSpinButton : public wxSpinButtonBase,
                                 public wxControlWithArrows
{
public:
    wxSpinButton();
    wxSpinButton(wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxSP_VERTICAL | wxSP_ARROW_KEYS,
                 const wxString& name = wxSPIN_BUTTON_NAME);

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_VERTICAL | wxSP_ARROW_KEYS,
                const wxString& name = wxSPIN_BUTTON_NAME);

    // implement wxSpinButtonBase methods
    int GetValue() const override;
    void SetValue(int val) override;
    void SetRange(int minVal, int maxVal) override;

    // implement wxControlWithArrows methods
    wxRenderer *GetRenderer() const override { return m_renderer; }
    wxWindow *GetWindow() override { return this; }
    bool IsVertical() const override { return wxSpinButtonBase::IsVertical(); }
    int GetArrowState(wxScrollArrows::Arrow arrow) const override;
    void SetArrowFlag(wxScrollArrows::Arrow arrow, int flag, bool set) override;
    bool OnArrow(wxScrollArrows::Arrow arrow) override;
    wxScrollArrows::Arrow HitTestArrow(const wxPoint& pt) const override;

    // for wxStdSpinButtonInputHandler
    const wxScrollArrows& GetArrows() { return m_arrows; }

    virtual bool PerformAction(const wxControlAction& action,
                               long numArg = 0,
                               const wxString& strArg = wxEmptyString) override;

    static wxInputHandler *GetStdInputHandler(wxInputHandler *handlerDef);
    wxInputHandler *DoGetStdInputHandler(wxInputHandler *handlerDef) override
    {
        return GetStdInputHandler(handlerDef);
    }

protected:
    wxSize DoGetBestClientSize() const override;
    void DoDraw(wxControlRenderer *renderer) override;
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // the common part of all ctors
    void Init();

    // normalize the value to fit into min..max range
    int NormalizeValue(int value) const;

    // change the value by +1/-1 and send the event, return true if value was
    // changed
    bool ChangeValue(int inc);

    // get the rectangles for our 2 arrows
    void CalcArrowRects(wxRect *rect1, wxRect *rect2) const;

    // the current controls value
    int m_value;

private:
    // the object which manages our arrows
    wxScrollArrows m_arrows;

    // the state (combination of wxCONTROL_XXX flags) of the arrows
    int m_arrowsState[wxScrollArrows::Arrow_Max];

    wxDECLARE_DYNAMIC_CLASS(wxSpinButton);
};

// ----------------------------------------------------------------------------
// wxStdSpinButtonInputHandler: manages clicks on them (use arrows like
// wxStdScrollBarInputHandler) and processes keyboard events too
// ----------------------------------------------------------------------------

class wxStdSpinButtonInputHandler : public wxStdInputHandler
{
public:
    wxStdSpinButtonInputHandler(wxInputHandler *inphand);

    virtual bool HandleKey(wxInputConsumer *consumer,
                           const wxKeyEvent& event,
                           bool pressed) override;
    virtual bool HandleMouse(wxInputConsumer *consumer,
                             const wxMouseEvent& event) override;
    virtual bool HandleMouseMove(wxInputConsumer *consumer,
                                 const wxMouseEvent& event) override;
};

#endif // _WX_UNIV_SPINBUTT_H_

