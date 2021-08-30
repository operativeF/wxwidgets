/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/fontpickerg.h
// Purpose:     wxGenericFontButton header
// Author:      Francesco Montorsi
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONTPICKER_H_
#define _WX_FONTPICKER_H_

#include "wx/button.h"
#include "wx/fontdata.h"

//-----------------------------------------------------------------------------
// wxGenericFontButton: a button which brings up a wxFontDialog
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxGenericFontButton : public wxButton,
                                             public wxFontPickerWidgetBase
{
public:
    wxGenericFontButton() = default;
    wxGenericFontButton(wxWindow *parent,
                        wxWindowID id,
                        const wxFont &initial = wxNullFont,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxFONTBTN_DEFAULT_STYLE,
                        const wxValidator& validator = wxDefaultValidator,
                        const std::string& name = wxFontPickerWidgetNameStr)
    {
        Create(parent, id, initial, pos, size, style, validator, name);
    }

    wxColour GetSelectedColour() const override
        { return m_data.GetColour(); }

    void SetSelectedColour(const wxColour &colour) override
        { m_data.SetColour(colour); UpdateFont(); }

    ~wxGenericFontButton() = default;


public:     // API extensions specific for wxGenericFontButton

    // user can override this to init font data in a different way
    virtual void InitFontData();

    // returns the font data shown in wxFontDialog
    wxFontData *GetFontData() { return &m_data; }


public:

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxFont &initial = *wxNORMAL_FONT,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxFONTBTN_DEFAULT_STYLE,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxFontPickerWidgetNameStr);

    void OnButtonClick(wxCommandEvent &);


protected:

    void UpdateFont() override;

    wxFontData m_data;

private:
    wxDECLARE_DYNAMIC_CLASS(wxGenericFontButton);
};


#endif // _WX_FONTPICKER_H_
