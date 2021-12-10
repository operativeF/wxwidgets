/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/clrpickerg.h
// Purpose:     wxGenericColourButton header
// Author:      Francesco Montorsi (based on Vadim Zeitlin's code)
// Modified by:
// Created:     14/4/2006
// Copyright:   (c) Vadim Zeitlin, Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CLRPICKER_H_
#define _WX_CLRPICKER_H_

#include "wx/button.h"
#include "wx/bmpbuttn.h"
#include "wx/colourdata.h"

import <string>;

class wxColourDialogEvent;

//-----------------------------------------------------------------------------
// wxGenericColourButton: a button which brings up a wxColourDialog
//-----------------------------------------------------------------------------

class wxGenericColourButton : public wxBitmapButton, public wxColourPickerWidgetBase
{
public:
    wxGenericColourButton() = default;
    wxGenericColourButton(wxWindow *parent,
                          wxWindowID id,
                          const wxColour& col = *wxBLACK,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          unsigned int style = wxCLRBTN_DEFAULT_STYLE,
                          const wxValidator& validator = {},
                          std::string_view name = wxColourPickerWidgetNameStr)
    {
        Create(parent, id, col, pos, size, style, validator, name);
    }

    // user can override this to init colour data in a different way
    virtual void InitColourData();

    // returns the colour data shown in wxColourDialog
    wxColourData *GetColourData() { return &ms_data; }


public:

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxColour& col = *wxBLACK,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxCLRBTN_DEFAULT_STYLE,
                const wxValidator& validator = {},
                std::string_view name = wxColourPickerWidgetNameStr);

    void OnButtonClick(wxCommandEvent &);


protected:
    wxBitmap    m_bitmap;

    wxSize DoGetBestSize() const override;

    void UpdateColour() override;

    void OnDPIChanged(wxDPIChangedEvent& event);

    // the colour data shown in wxColourPickerCtrlGeneric
    // controls. This member is static so that all colour pickers
    // in the program share the same set of custom colours.
    inline static wxColourData ms_data{};

private:
    void OnColourChanged(wxColourDialogEvent& event);
};


#endif // _WX_CLRPICKER_H_
