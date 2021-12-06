///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/infobar.h
// Purpose:     generic wxInfoBar class declaration
// Author:      Vadim Zeitlin
// Created:     2009-07-28
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_INFOBAR_H_
#define _WX_GENERIC_INFOBAR_H_

import WX.Cfg.Flags;

class wxBitmapButton;
class wxStaticBitmap;
class wxStaticText;

// ----------------------------------------------------------------------------
// wxInfoBar
// ----------------------------------------------------------------------------

class wxInfoBarGeneric : public wxInfoBarBase
{
public:
    // the usual ctors and Create() but remember that info bar is created
    // hidden
    wxInfoBarGeneric() = default;

    wxInfoBarGeneric(wxWindow *parent, wxWindowID winid = wxID_ANY)
    {
        Create(parent, winid);
    }

	wxInfoBarGeneric& operator=(wxInfoBarGeneric&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID winid = wxID_ANY);

    // implement base class methods
    // ----------------------------

    void ShowMessage(const std::string& msg,
                     unsigned int flags = wxICON_INFORMATION) override;

    void Dismiss() override;

    void AddButton(wxWindowID btnid, const std::string& label = {}) override;

    void RemoveButton(wxWindowID btnid) override;

    size_t GetButtonCount() const override;
    wxWindowID GetButtonId(size_t idx) const override;
    bool HasButtonId(wxWindowID btnid) const override;

    // methods specific to this version
    // --------------------------------

    // set the effect(s) to use when showing/hiding the bar, may be
    // wxShowEffect::None to disable any effects entirely
    //
    // by default, slide to bottom/top is used when it's positioned on the top
    // of the window for showing/hiding it and top/bottom when it's positioned
    // at the bottom
    void SetShowHideEffects(wxShowEffect showEffect, wxShowEffect hideEffect)
    {
        m_showEffect = showEffect;
        m_hideEffect = hideEffect;
    }

    // get effect used when showing/hiding the window
    wxShowEffect GetShowEffect() const;
    wxShowEffect GetHideEffect() const;

    // set the duration of animation used when showing/hiding the bar, in ms
    void SetEffectDuration(int duration) { m_effectDuration = duration; }

    // get the currently used effect animation duration
    int GetEffectDuration() const { return m_effectDuration; }


    // overridden base class methods
    // -----------------------------

    // setting the font of this window sets it for the text control inside it
    // (default font is a larger and bold version of the normal one)
    bool SetFont(const wxFont& font) override;

    // same thing with the colour: this affects the text colour
    bool SetForegroundColour(const wxColor& colour) override;

protected:
    // info bar shouldn't have any border by default, the colour difference
    // between it and the main window separates it well enough
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }


    // update the parent to take our new or changed size into account (notably
    // should be called when we're shown or hidden)
    void UpdateParent();

private:
    // handler for the close button
    void OnButton(wxCommandEvent& event);

    // show/hide the bar
    void DoShow();
    void DoHide();

    // determine the placement of the bar from its position in the containing
    // sizer
    enum BarPlacement
    {
        BarPlacement_Top,
        BarPlacement_Bottom,
        BarPlacement_Unknown
    };

    BarPlacement GetBarPlacement() const;


    // different controls making up the bar
    wxStaticBitmap *m_icon{nullptr};
    wxStaticText *m_text{nullptr};
    wxBitmapButton *m_button{nullptr};

    // the effects to use when showing/hiding and duration for them: by default
    // the effect is determined by the info bar automatically depending on its
    // position and the default duration is used
    wxShowEffect m_showEffect{wxShowEffect::Max};
    wxShowEffect m_hideEffect{wxShowEffect::Max};
    
    int m_effectDuration{0};

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_INFOBAR_H_

