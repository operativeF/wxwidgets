///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/gaugetest.cpp
// Purpose:     wxGauge unit test
// Author:      Steven Lamerton
// Created:     2010-07-15
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_GAUGE


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/gauge.h"
#endif // WX_PRECOMP


TEST_CASE("Gauge test")
{
    auto m_gauge = new wxGauge(wxTheApp->GetTopWindow(), wxID_ANY, 100);

    SUBCASE("Direction")
    {
        //We should default to a horizontal gauge
        CHECK(!m_gauge->IsVertical());

        wxDELETE(m_gauge);
        m_gauge = new wxGauge(wxTheApp->GetTopWindow(), wxID_ANY, 100,
                              wxDefaultPosition, wxDefaultSize, wxGA_VERTICAL);

        CHECK(m_gauge->IsVertical());

        wxDELETE(m_gauge);
        m_gauge = new wxGauge(wxTheApp->GetTopWindow(), wxID_ANY, 100,
                              wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);

        CHECK(!m_gauge->IsVertical());
    }

    SUBCASE("Range")
    {
        CHECK_EQ(100, m_gauge->GetRange());

        m_gauge->SetRange(50);

        CHECK_EQ(50, m_gauge->GetRange());

        m_gauge->SetRange(0);

        CHECK_EQ(0, m_gauge->GetRange());
    }

    SUBCASE("Value")
    {
        CHECK_EQ(0, m_gauge->GetValue());

        m_gauge->SetValue(50);

        CHECK_EQ(50, m_gauge->GetValue());

        m_gauge->SetValue(0);

        CHECK_EQ(0, m_gauge->GetValue());

        m_gauge->SetValue(100);

        CHECK_EQ(100, m_gauge->GetValue());
    }

    wxTheApp->GetTopWindow()->DestroyChildren();
}

#endif //wxUSE_GAUGE
