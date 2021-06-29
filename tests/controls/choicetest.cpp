///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/choice.cpp
// Purpose:     wxChoice unit test
// Author:      Steven Lamerton
// Created:     2010-06-29
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"

#if wxUSE_CHOICE


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/choice.h"
#endif // WX_PRECOMP

#include "itemcontainertest.h"

class ChoiceTestCase : public ItemContainerTestCase, public CppUnit::TestCase
{
public:
    ChoiceTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    wxItemContainer *GetContainer() const override { return m_choice; }
    wxWindow *GetContainerWindow() const override { return m_choice; }

    CPPUNIT_TEST_SUITE( ChoiceTestCase );
        wxITEM_CONTAINER_TESTS();
        CPPUNIT_TEST( Sort );
        CPPUNIT_TEST( GetBestSize );
    CPPUNIT_TEST_SUITE_END();

    void Sort();
    void GetBestSize();

    wxChoice* m_choice;

    ChoiceTestCase(const ChoiceTestCase&) = delete;
	ChoiceTestCase& operator=(const ChoiceTestCase&) = delete;
};

wxREGISTER_UNIT_TEST_WITH_TAGS(ChoiceTestCase,
                               "[ChoiceTestCase][item-container]");

void ChoiceTestCase::setUp()
{
    m_choice = new wxChoice(wxTheApp->GetTopWindow(), wxID_ANY, wxDefaultPosition, wxDefaultSize);
}

void ChoiceTestCase::tearDown()
{
    wxDELETE(m_choice);
}

void ChoiceTestCase::Sort()
{
#if !defined(__WXOSX__)
    wxDELETE(m_choice);
    m_choice = new wxChoice(wxTheApp->GetTopWindow(), wxID_ANY,
                            wxDefaultPosition, wxDefaultSize, {},
                            wxCB_SORT);

    std::vector<wxString> testitems =
    {
        "aaa",
        "Aaa",
        "aba",
        "aaab",
        "aab",
        "AAA"
    };

    m_choice->Append(testitems);

    CHECK_EQ("AAA", m_choice->GetString(0));
    CHECK_EQ("Aaa", m_choice->GetString(1));
    CHECK_EQ("aaa", m_choice->GetString(2));
    CHECK_EQ("aaab", m_choice->GetString(3));
    CHECK_EQ("aab", m_choice->GetString(4));
    CHECK_EQ("aba", m_choice->GetString(5));

    m_choice->Append("a");

    CHECK_EQ("a", m_choice->GetString(0));
#endif
}

void ChoiceTestCase::GetBestSize()
{
    std::vector<wxString> testitems = {
        "1",
        "11"
    };
    
    m_choice->Append(testitems);

    SECTION("Normal best size")
    {
        // nothing to do here
    }

    // Ensure that the hidden control return a valid best size too.
    SECTION("Hidden best size")
    {
        m_choice->Hide();
    }

    wxYield();

    m_choice->InvalidateBestSize();
    const wxSize bestSize = m_choice->GetBestSize();

    CHECK(bestSize.GetWidth() > m_choice->FromDIP(30));
    CHECK(bestSize.GetWidth() < m_choice->FromDIP(120));
    CHECK(bestSize.GetHeight() > m_choice->FromDIP(15));
    CHECK(bestSize.GetHeight() < m_choice->FromDIP(35));
}

#endif //wxUSE_CHOICE
