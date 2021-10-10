///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/gaugecmn.cpp
// Purpose:     wxGaugeBase: common to all ports methods of wxGauge
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.02.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_GAUGE

#include "wx/gauge.h"
#include "wx/appprogress.h"

// ----------------------------------------------------------------------------
// XTI
// ----------------------------------------------------------------------------

wxDEFINE_FLAGS( wxGaugeStyle )
wxBEGIN_FLAGS( wxGaugeStyle )
// new style border flags, we put them first to
// use them for streaming out
wxFLAGS_MEMBER(wxBorder::Simple)
wxFLAGS_MEMBER(wxBorder::Sunken)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBorder::Raised)
wxFLAGS_MEMBER(wxBorder::Static)
wxFLAGS_MEMBER(wxBorder::None)

// old style border flags
wxFLAGS_MEMBER(wxBorder::Simple)
wxFLAGS_MEMBER(wxBorder::Sunken)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxBorder::Raised)
wxFLAGS_MEMBER(wxBorder::Static)
wxFLAGS_MEMBER(wxBORDER)

// standard window styles
wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxGA_HORIZONTAL)
wxFLAGS_MEMBER(wxGA_VERTICAL)
wxFLAGS_MEMBER(wxGA_SMOOTH)
wxFLAGS_MEMBER(wxGA_PROGRESS)
wxEND_FLAGS( wxGaugeStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxGauge, wxControl, "wx/gauge.h");

wxBEGIN_PROPERTIES_TABLE(wxGauge)
wxPROPERTY( Value, int, SetValue, GetValue, 0, 0 /*flags*/, \
           wxT("Helpstring"), wxT("group"))
wxPROPERTY( Range, int, SetRange, GetRange, 0, 0 /*flags*/, \
           wxT("Helpstring"), wxT("group"))

wxPROPERTY_FLAGS( WindowStyle, wxGaugeStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 /*flags*/, \
                 wxT("Helpstring"), wxT("group")) // style
wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxGauge)

wxCONSTRUCTOR_6( wxGauge, wxWindow*, Parent, wxWindowID, Id, int, Range, \
                wxPoint, Position, wxSize, Size, long, WindowStyle )

// ----------------------------------------------------------------------------
// wxGauge creation
// ----------------------------------------------------------------------------

void wxGaugeBase::InitProgressIndicatorIfNeeded()
{
    if ( HasFlag(wxGA_PROGRESS) )
    {
        wxWindow* topParent = wxGetTopLevelParent(this);
        if ( topParent != nullptr )
        {
            m_appProgressIndicator = std::make_unique<wxAppProgressIndicator>
                                                        (topParent, GetRange());
        }
    }
}

bool wxGaugeBase::Create(wxWindow *parent,
                         wxWindowID id,
                         int range,
                         const wxPoint& pos,
                         const wxSize& size,
                         unsigned int style,
                         const wxValidator& validator,
                         const std::string& name)
{
    if ( !wxControl::Create(parent, id, pos, size, style, validator, name) )
        return false;

    SetName(name);

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif // wxUSE_VALIDATORS

    SetRange(range);
    SetValue(0);

#if wxGAUGE_EMULATE_INDETERMINATE_MODE
    m_nDirection = wxDirection::Right;
#endif

    InitProgressIndicatorIfNeeded();

    return true;
}

// ----------------------------------------------------------------------------
// wxGauge determinate mode range/position
// ----------------------------------------------------------------------------

void wxGaugeBase::SetRange(int range)
{
    m_rangeMax = range;

    if ( m_appProgressIndicator )
        m_appProgressIndicator->SetRange(m_rangeMax);
}

int wxGaugeBase::GetRange() const
{
    return m_rangeMax;
}

void wxGaugeBase::SetValue(int pos)
{
    m_gaugePos = pos;

    if ( m_appProgressIndicator )
    {
        m_appProgressIndicator->SetValue(pos);

        if ( pos == 0 )
        {
            m_appProgressIndicator->Reset();
        }
    }
}

int wxGaugeBase::GetValue() const
{
    return m_gaugePos;
}

// ----------------------------------------------------------------------------
// wxGauge indeterminate mode
// ----------------------------------------------------------------------------

void wxGaugeBase::Pulse()
{
#if wxGAUGE_EMULATE_INDETERMINATE_MODE
    // simulate indeterminate mode
    const int curr = GetValue();
    const int max = GetRange();

    if (m_nDirection == wxDirection::Right)
    {
        if (curr < max)
            SetValue(curr + 1);
        else
        {
            SetValue(max - 1);
            m_nDirection = wxDirection::Left;
        }
    }
    else
    {
        if (curr > 0)
            SetValue(curr - 1);
        else
        {
            SetValue(1);
            m_nDirection = wxDirection::Right;
        }
    }
#endif

    if ( m_appProgressIndicator )
        m_appProgressIndicator->Pulse();
}

#endif // wxUSE_GAUGE
