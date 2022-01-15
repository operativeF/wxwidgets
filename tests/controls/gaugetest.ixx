///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/gaugetest.cpp
// Purpose:     wxGauge unit test
// Author:      Steven Lamerton
// Created:     2010-07-15
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/gauge.h"

export module WX.Test.Gauge;

import WX.MetaTest;
import WX.Test.Prec;

#if wxUSE_GAUGE

namespace ut = boost::ut;

ut::suite GaugeTests = []
{
    using namespace ut;
    
    auto m_gauge = new wxGauge(wxTheApp->GetTopWindow(), wxID_ANY, 100);

    "Direction"_test = [&]
    {
        //We should default to a horizontal gauge
        expect(!m_gauge->IsVertical());

        wxDELETE(m_gauge);
        m_gauge = new wxGauge(wxTheApp->GetTopWindow(), wxID_ANY, 100,
                              wxDefaultPosition, wxDefaultSize, wxGA_VERTICAL);

        expect(m_gauge->IsVertical());

        wxDELETE(m_gauge);
        m_gauge = new wxGauge(wxTheApp->GetTopWindow(), wxID_ANY, 100,
                              wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);

        expect(!m_gauge->IsVertical());
    };

    "Range"_test = [&]
    {
        expect(100 == m_gauge->GetRange());

        m_gauge->SetRange(50);

        expect(50 == m_gauge->GetRange());

        m_gauge->SetRange(0);

        expect(0 == m_gauge->GetRange());
    };

    "Value"_test = [&]
    {
        expect(0 == m_gauge->GetValue());

        m_gauge->SetValue(50);

        expect(50 == m_gauge->GetValue());

        m_gauge->SetValue(0);

        expect(0 == m_gauge->GetValue());

        m_gauge->SetValue(100);

        expect(100 == m_gauge->GetValue());
    };

    wxTheApp->GetTopWindow()->DestroyChildren();
};

#endif //wxUSE_GAUGE
