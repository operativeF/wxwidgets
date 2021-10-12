/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_infobar.cpp
// Purpose:     XML resource handler for wxInfoBar
// Author:      Ilya Sinitsyn
// Created:     2019-09-25
// Copyright:   (c) 2019 TT-Solutions SARL
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_INFOBAR

#include "wx/xrc/xh_infobar.h"

#include "wx/infobar.h"
#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxInfoBarXmlHandler, wxXmlResourceHandler);

#define XRC_ADD_SHOW_EFFECT(style) m_effectNames[style] = #style;

wxInfoBarXmlHandler::wxInfoBarXmlHandler()     
{
    // FIXME: Doesn't work with enum class.
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::None);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::RollToLeft);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::RollToRight);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::RollToTop);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::RollToBottom);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::SlideToLeft);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::SlideToRight);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::SlideToTop);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::SlideToBottom);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::Blend);
    //XRC_ADD_SHOW_EFFECT(wxShowEffect::Expand);
}

wxObject *wxInfoBarXmlHandler::DoCreateResource()
{
    if ( m_class == "wxInfoBar" )
    {
        XRC_MAKE_INSTANCE(control, wxInfoBar)

        control->Create(m_parentAsWindow, GetID());

        SetupWindow(control);

        wxShowEffect showEffect = GetShowEffect("showeffect");
        wxShowEffect hideEffect = GetShowEffect("hideeffect");

        if ( showEffect != wxShowEffect::None || hideEffect != wxShowEffect::None )
            control->SetShowHideEffects(showEffect, hideEffect);

        if ( HasParam("effectduration") )
            control->SetEffectDuration(GetLong("effectduration"));

        m_insideBar = true;
        CreateChildrenPrivately(control);
        m_insideBar = false;

        return control;
    }
    else
    {
        // inside the element now,
        // handle buttons

        wxInfoBar * const infoBar = wxDynamicCast(m_parentAsWindow, wxInfoBar);
        wxCHECK_MSG(infoBar, nullptr, "must have wxInfoBar parent");

        infoBar->AddButton(GetID(), GetText("label"));

        return nullptr;
    }
}

bool wxInfoBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, "wxInfoBar") ||
           (m_insideBar && IsOfClass(node, "button")));
}

wxShowEffect wxInfoBarXmlHandler::GetShowEffect(wxString const& param)
{
    if ( !HasParam(param) )
        return wxShowEffect::None;

    wxString const& value = GetParamValue(param);

    // FIXME: Don't do this.
    for ( int i = 0; i != static_cast<int>(wxShowEffect::Max); ++i )
    {
        if ( value == m_effectNames[i] )
            return static_cast<wxShowEffect>(i);
    }

    ReportParamError
    (
        param,
        wxString::Format("unknown show effect \"%s\"", value)
    );

    return wxShowEffect::None;
}

#endif // wxUSE_XRC && wxUSE_INFOBAR
