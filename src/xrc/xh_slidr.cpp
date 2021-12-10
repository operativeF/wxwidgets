/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_slidr.cpp
// Purpose:     XRC resource for wxSlider
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_XRC && wxUSE_SLIDER

#include "wx/xrc/xh_slidr.h"

#include "wx/slider.h"

constexpr long DEFAULT_VALUE = 0;
constexpr long DEFAULT_MIN = 0;
constexpr long DEFAULT_MAX = 100;


wxIMPLEMENT_DYNAMIC_CLASS(wxSliderXmlHandler, wxXmlResourceHandler);

wxSliderXmlHandler::wxSliderXmlHandler()
                   
{
    XRC_ADD_STYLE(wxSL_HORIZONTAL);
    XRC_ADD_STYLE(wxSL_VERTICAL);
    XRC_ADD_STYLE(wxSL_AUTOTICKS);
    XRC_ADD_STYLE(wxSL_MIN_MAX_LABELS);
    XRC_ADD_STYLE(wxSL_VALUE_LABEL);
    XRC_ADD_STYLE(wxSL_LABELS);
    XRC_ADD_STYLE(wxSL_LEFT);
    XRC_ADD_STYLE(wxSL_TOP);
    XRC_ADD_STYLE(wxSL_RIGHT);
    XRC_ADD_STYLE(wxSL_BOTTOM);
    XRC_ADD_STYLE(wxSL_BOTH);
    XRC_ADD_STYLE(wxSL_SELRANGE);
    XRC_ADD_STYLE(wxSL_INVERSE);
    AddWindowStyles();
}

wxObject *wxSliderXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSlider)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetLong("value", DEFAULT_VALUE),
                    GetLong("min", DEFAULT_MIN),
                    GetLong("max", DEFAULT_MAX),
                    GetPosition(), GetSize(),
                    GetStyle(),
                    wxValidator{},
                    GetName());

    if( HasParam("tickfreq"))
    {
        control->SetTickFreq(GetLong("tickfreq"));
    }
    if( HasParam("pagesize"))
    {
        control->SetPageSize(GetLong("pagesize"));
    }
    if( HasParam("linesize"))
    {
        control->SetLineSize(GetLong("linesize"));
    }
    if( HasParam("thumb"))
    {
        control->SetThumbLength(GetLong("thumb"));
    }
    if( HasParam("tick"))
    {
        control->SetTick(GetLong("tick"));
    }
    if( HasParam("selmin")) && HasParam(wxT("selmax"))
    {
        control->SetSelection(GetLong("selmin")), GetLong(wxT("selmax"));
    }

    SetupWindow(control);

    return control;
}

bool wxSliderXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "wxSlider");
}

#endif // wxUSE_XRC && wxUSE_SLIDER
