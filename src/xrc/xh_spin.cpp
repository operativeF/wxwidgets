/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_spin.cpp
// Purpose:     XRC resource for wxSpinButton
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC

#include "wx/xrc/xh_spin.h"

#if wxUSE_SPINBTN

#include "wx/spinbutt.h"

constexpr long DEFAULT_VALUE = 0;
constexpr long DEFAULT_MIN = 0;
constexpr long DEFAULT_MAX = 100;

wxIMPLEMENT_DYNAMIC_CLASS(wxSpinButtonXmlHandler, wxXmlResourceHandler);

wxSpinButtonXmlHandler::wxSpinButtonXmlHandler()
 
{
    XRC_ADD_STYLE(wxSP_HORIZONTAL);
    XRC_ADD_STYLE(wxSP_VERTICAL);
    XRC_ADD_STYLE(wxSP_ARROW_KEYS);
    XRC_ADD_STYLE(wxSP_WRAP);
    AddWindowStyles();
}

wxObject *wxSpinButtonXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSpinButton)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle("style", wxSP_ARROW_KEYS),
                    GetName());

    control->SetValue(GetLong( "value", DEFAULT_VALUE));
    control->SetRange(GetLong( "min", DEFAULT_MIN),
                      GetLong("max", DEFAULT_MAX));
    SetupWindow(control);

    return control;
}

bool wxSpinButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxSpinButton");
}

#endif // wxUSE_SPINBTN

#if wxUSE_SPINCTRL

#include "wx/spinctrl.h"

constexpr float DEFAULT_INC = 1;

static void AddSpinCtrlStyles(wxXmlResourceHandler& handler)
{
    handler.XRC_ADD_STYLE(wxSP_HORIZONTAL);
    handler.XRC_ADD_STYLE(wxSP_VERTICAL);
    handler.XRC_ADD_STYLE(wxSP_ARROW_KEYS);
    handler.XRC_ADD_STYLE(wxSP_WRAP);
    handler.XRC_ADD_STYLE(wxALIGN_LEFT);
    handler.XRC_ADD_STYLE(wxALIGN_CENTER);
    handler.XRC_ADD_STYLE(wxALIGN_RIGHT);
}

wxIMPLEMENT_DYNAMIC_CLASS(wxSpinCtrlXmlHandler, wxXmlResourceHandler);

wxSpinCtrlXmlHandler::wxSpinCtrlXmlHandler()
     
{
    AddSpinCtrlStyles(*this);
}

wxObject *wxSpinCtrlXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSpinCtrl)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText("value"),
                    GetPosition(), GetSize(),
                    GetStyle("style", wxSP_ARROW_KEYS),
                    GetLong("min", DEFAULT_MIN),
                    GetLong("max", DEFAULT_MAX),
                    GetLong("value", DEFAULT_VALUE),
                    GetName());

    const long base = GetLong("base", 10);
    if ( base != 10 )
        control->SetBase(base);

    SetupWindow(control);

    return control;
}

bool wxSpinCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxSpinCtrl");
}


wxIMPLEMENT_DYNAMIC_CLASS(wxSpinCtrlDoubleXmlHandler, wxXmlResourceHandler);

wxSpinCtrlDoubleXmlHandler::wxSpinCtrlDoubleXmlHandler()
     
{
    AddSpinCtrlStyles(*this);
}

wxObject *wxSpinCtrlDoubleXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSpinCtrlDouble)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText("value"),
                    GetPosition(), GetSize(),
                    GetStyle("style", wxSP_ARROW_KEYS),
                    double(GetFloat("min", DEFAULT_MIN)),
                    double(GetFloat("max", DEFAULT_MAX)),
                    double(GetFloat("value", DEFAULT_VALUE)),
                    double(GetFloat("inc", DEFAULT_INC)),
                    GetName());

    SetupWindow(control);

    return control;
}

bool wxSpinCtrlDoubleXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxSpinCtrlDouble");
}

#endif // wxUSE_SPINCTRL

#endif // wxUSE_XRC
