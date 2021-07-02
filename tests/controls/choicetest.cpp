///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/choice.cpp
// Purpose:     wxChoice unit test
// Author:      Steven Lamerton
// Created:     2010-06-29
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_CHOICE


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/choice.h"
#endif // WX_PRECOMP

#include "itemcontainertest.h"


using ChoiceControlTest = ItemContainerTest<wxChoice>;

TEST_CASE_FIXTURE(ChoiceControlTest, "Choice control test.")
{
    m_container = std::make_unique<wxChoice>(wxTheApp->GetTopWindow(),
                                             wxID_ANY, wxDefaultPosition,
                                             wxDefaultSize, std::vector<wxString>{}, wxCB_SORT);
#if !defined(__WXOSX__)
    SUBCASE("Sort")
    {
        std::vector<wxString> testitems =
        {
            "aaa",
            "Aaa",
            "aba",
            "aaab",
            "aab",
            "AAA"
        };

        m_container->Append(testitems);

        CHECK_EQ("AAA", m_container->GetString(0));
        CHECK_EQ("Aaa", m_container->GetString(1));
        CHECK_EQ("aaa", m_container->GetString(2));
        CHECK_EQ("aaab", m_container->GetString(3));
        CHECK_EQ("aab", m_container->GetString(4));
        CHECK_EQ("aba", m_container->GetString(5));

        m_container->Append("a");

        CHECK_EQ("a", m_container->GetString(0));
    }
#endif

    SUBCASE("GetBestSize")
    {
        std::vector<wxString> testitems = {
            "1",
            "11"
        };
    
        m_container->Append(testitems);


        // Ensure that the hidden control return a valid best size too.
        SUBCASE("Hidden best size")
        {
            m_container->Hide();
        }

        wxYield();

        m_container->InvalidateBestSize();
        const wxSize bestSize = m_container->GetBestSize();

        CHECK(bestSize.GetWidth() > m_container->FromDIP(30));
        CHECK(bestSize.GetWidth() < m_container->FromDIP(120));
        CHECK(bestSize.GetHeight() > m_container->FromDIP(15));
        CHECK(bestSize.GetHeight() < m_container->FromDIP(35));
    }

    // FIXME: ? Sorted containers aren't allowed to insert.
    m_container = std::make_unique<wxChoice>(wxTheApp->GetTopWindow(),
        wxID_ANY, wxDefaultPosition,
        wxDefaultSize, std::vector<wxString>{});

    wxITEM_CONTAINER_TESTS();
}
#endif //wxUSE_CHOICE
