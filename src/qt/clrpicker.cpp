/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/clrpicker.cpp
// Author:      Peter Most
// Copyright:   (c) Peter Most
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////



#include "wx/clrpicker.h"

wxColourPickerWidget::wxColourPickerWidget()
{
}

wxColourPickerWidget::wxColourPickerWidget(wxWindow *parent,
               wxWindowID id,
               const wxColour& initial,
               const wxPoint& pos,
               const wxSize& size,
               unsigned int style,
               const wxValidator& validator,
               const wxString& name )
{
    Create( parent, id, initial, pos, size, style, validator, name );
}


bool wxColourPickerWidget::Create(wxWindow *parent,
            wxWindowID id,
            const wxColour& initial,
            const wxPoint& pos,
            const wxSize& size,
            unsigned int style,
            const wxValidator& validator,
            const wxString& name )
{
    return wxGenericColourButton::Create( parent, id, initial, pos, size, style, validator, name );
}


void wxColourPickerWidget::UpdateColour()
{
    wxGenericColourButton::UpdateColour();
}

