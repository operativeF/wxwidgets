/////////////////////////////////////////////////////////////////////////////
// Name:        wx/pickerbase.h
// Purpose:     wxPickerBase definition
// Author:      Francesco Montorsi (based on Vadim Zeitlin's code)
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Vadim Zeitlin, Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PICKERBASE_H_BASE_
#define _WX_PICKERBASE_H_BASE_

#include "wx/control.h"
#include "wx/sizer.h"
#include "wx/containr.h"

import <string>;

class wxTextCtrl;
class wxToolTip;

inline constexpr std::string_view wxPickerNameStr = "picker";

// ----------------------------------------------------------------------------
// wxPickerBase is the base class for the picker controls which support
// a wxPB_USE_TEXTCTRL style; i.e. for those pickers which can use an auxiliary
// text control next to the 'real' picker.
//
// The wxTextPickerHelper class manages enabled/disabled state of the text control,
// its sizing and positioning.
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxPB_USE_TEXTCTRL = 0x0002;
inline constexpr unsigned int wxPB_SMALL        = 0x8000;

class wxPickerBase : public wxNavigationEnabled<wxControl>
{
public:
    // if present, intercepts wxPB_USE_TEXTCTRL style and creates the text control
    // The 3rd argument is the initial std::string to display in the text control
    bool CreateBase(wxWindow *parent,
                    wxWindowID id,
                    std::string_view text = {},
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = 0,
                    const wxValidator& validator = {},
                    std::string_view name = wxPickerNameStr);

    // margin between the text control and the picker
    void SetInternalMargin(int newmargin)
        { GetTextCtrlItem()->SetBorder(newmargin); m_sizer->Layout(); }
    int GetInternalMargin() const
        { return GetTextCtrlItem()->GetBorder(); }

    // proportion of the text control
    void SetTextCtrlProportion(int prop)
        { GetTextCtrlItem()->SetProportion(prop); m_sizer->Layout(); }
    int GetTextCtrlProportion() const
        { return GetTextCtrlItem()->GetProportion(); }

    // proportion of the picker control
    void SetPickerCtrlProportion(int prop)
        { GetPickerCtrlItem()->SetProportion(prop); m_sizer->Layout(); }
    int GetPickerCtrlProportion() const
        { return GetPickerCtrlItem()->GetProportion(); }

    bool IsTextCtrlGrowable() const
        { return (GetTextCtrlItem()->GetFlag() & wxGROW) != 0; }
    void SetTextCtrlGrowable(bool grow = true)
    {
        DoSetGrowableFlagFor(GetTextCtrlItem(), grow);
    }

    bool IsPickerCtrlGrowable() const
        { return (GetPickerCtrlItem()->GetFlag() & wxGROW) != 0; }
    void SetPickerCtrlGrowable(bool grow = true)
    {
        DoSetGrowableFlagFor(GetPickerCtrlItem(), grow);
    }

    bool HasTextCtrl() const
        { return m_text != nullptr; }
    wxTextCtrl *GetTextCtrl()
        { return m_text; }
    wxControl *GetPickerCtrl()
        { return m_picker; }

    void SetTextCtrl(wxTextCtrl* text)
        { m_text = text; }
    void SetPickerCtrl(wxControl* picker)
        { m_picker = picker; }

    // methods that derived class must/may override
    virtual void UpdatePickerFromTextCtrl() = 0;
    virtual void UpdateTextCtrlFromPicker() = 0;

protected:
    // overridden base class methods
#if wxUSE_TOOLTIPS
    void DoSetToolTip(wxToolTip *tip) override;
#endif // wxUSE_TOOLTIPS


    // event handlers
    void OnTextCtrlDelete(wxWindowDestroyEvent &);
    void OnTextCtrlUpdate(wxCommandEvent &);
    void OnTextCtrlKillFocus(wxFocusEvent &);

    // returns the set of styles for the attached wxTextCtrl
    // from given wxPickerBase's styles
    virtual unsigned int GetTextCtrlStyle(unsigned int style) const
        { return (style & wxWINDOW_STYLE_MASK); }

    // returns the set of styles for the m_picker
    virtual unsigned int GetPickerStyle(unsigned int style) const
        { return (style & wxWINDOW_STYLE_MASK); }


    wxSizerItem *GetPickerCtrlItem() const
    {
        if (this->HasTextCtrl())
            return m_sizer->GetItem((size_t)1);
        return m_sizer->GetItem((size_t)0);
    }

    wxSizerItem *GetTextCtrlItem() const
    {
        wxASSERT(this->HasTextCtrl());
        return m_sizer->GetItem((size_t)0);
    }

    void PostCreation();

protected:
    wxTextCtrl *m_text{nullptr};     // can be NULL
    wxControl *m_picker{nullptr};
    wxBoxSizer *m_sizer{nullptr};

private:
    // Common implementation of Set{Text,Picker}CtrlGrowable().
    void DoSetGrowableFlagFor(wxSizerItem* item, bool grow);

    wxDECLARE_ABSTRACT_CLASS(wxPickerBase);
};


#endif
    // _WX_PICKERBASE_H_BASE_
