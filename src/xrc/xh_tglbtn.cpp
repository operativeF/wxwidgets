/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_tglbtn.cpp
// Purpose:     XRC resource for wxToggleButton
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_TOGGLEBTN

# if !defined(__WXUNIVERSAL__) && !defined(__WXMOTIF__) && !(defined(__WXGTK__) && !defined(__WXGTK20__))
#define wxHAVE_BITMAPS_IN_BUTTON 1;
# endif

#include "wx/xrc/xh_tglbtn.h"
#include "wx/tglbtn.h"
#include "wx/button.h" // solely for wxBU_EXACTFIT

wxIMPLEMENT_DYNAMIC_CLASS(wxToggleButtonXmlHandler, wxXmlResourceHandler);

wxToggleButtonXmlHandler::wxToggleButtonXmlHandler()
     
{
    XRC_ADD_STYLE(wxBU_EXACTFIT);

    AddWindowStyles();
}

wxObject *wxToggleButtonXmlHandler::DoCreateResource()
{

   wxObject *control = m_instance;

#ifdef wxHAVE_BITMAPS_IN_BUTTON

    if (m_class == "wxBitmapToggleButton")
    {
       if (!control)
           control = new wxBitmapToggleButton;

        DoCreateBitmapToggleButton(control);
    }
    else
#endif
    {
       if (!control)
           control = new wxToggleButton;

        DoCreateToggleButton(control);
    }

    SetupWindow(dynamic_cast<wxWindow*>(control));

    return control;
}

bool wxToggleButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return (
               IsOfClass(node, "wxToggleButton") ||
               IsOfClass(node, "wxBitmapToggleButton")
           );
}

void wxToggleButtonXmlHandler::DoCreateToggleButton(wxObject *control)
{
    wxToggleButton *button = dynamic_cast<wxToggleButton*>(control);

    button->Create(m_parentAsWindow,
                   GetID(),
                   GetText("label"),
                   GetPosition(), GetSize(),
                   GetStyle(),
                   wxValidator{},
                   GetName());

#ifdef wxHAVE_BITMAPS_IN_BUTTON
    if ( GetParamNode("bitmap") )
    {
        button->SetBitmap(GetBitmap("bitmap", wxART_BUTTON),
                          GetDirection("bitmapposition"));
    }
#endif

    button->SetValue(GetBool( "checked"));
}

#ifdef wxHAVE_BITMAPS_IN_BUTTON
void wxToggleButtonXmlHandler::DoCreateBitmapToggleButton(wxObject *control)
{
    wxBitmapToggleButton *button = dynamic_cast<wxBitmapToggleButton*>(control);

    button->Create(m_parentAsWindow,
                   GetID(),
                   GetBitmap("bitmap", wxART_BUTTON),
                   GetPosition(), GetSize(),
                   GetStyle(),
                   wxValidator{},
                   GetName());

    button->SetValue(GetBool( "checked"));
}
#endif
#endif // wxUSE_XRC && wxUSE_TOGGLEBTN
