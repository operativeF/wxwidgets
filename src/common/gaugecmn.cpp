///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/gaugecmn.cpp
// Purpose:     wxGaugeBase: common to all ports methods of wxGauge
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.02.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////




#if wxUSE_GAUGE

#include "wx/gauge.h"
#include "wx/appprogress.h"

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
                         std::string_view name)
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
    m_nDirection = wxRIGHT;
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

    if (m_nDirection == wxRIGHT)
    {
        if (curr < max)
            SetValue(curr + 1);
        else
        {
            SetValue(max - 1);
            m_nDirection = wxLEFT;
        }
    }
    else
    {
        if (curr > 0)
            SetValue(curr - 1);
        else
        {
            SetValue(1);
            m_nDirection = wxRIGHT;
        }
    }
#endif

    if ( m_appProgressIndicator )
        m_appProgressIndicator->Pulse();
}

#endif // wxUSE_GAUGE
