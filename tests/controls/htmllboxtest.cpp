///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/htmllboxtest.cpp
// Purpose:     wxSimpleHtmlListBoxNameStr unit test
// Author:      Vadim Zeitlin
// Created:     2010-11-27
// Copyright:   (c) 2010 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"

#if wxUSE_HTML


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/htmllbox.h"
#include "itemcontainertest.h"

class HtmlListBoxTestCase : public ItemContainerTestCase,
                            public CppUnit::TestCase
{
public:
    HtmlListBoxTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    wxItemContainer *GetContainer() const override { return m_htmllbox; }
    wxWindow *GetContainerWindow() const override { return m_htmllbox; }

    CPPUNIT_TEST_SUITE( HtmlListBoxTestCase );
        wxITEM_CONTAINER_TESTS();
    CPPUNIT_TEST_SUITE_END();

    wxSimpleHtmlListBox* m_htmllbox;

    HtmlListBoxTestCase(const HtmlListBoxTestCase&) = delete;
	HtmlListBoxTestCase& operator=(const HtmlListBoxTestCase&) = delete;
};

wxREGISTER_UNIT_TEST_WITH_TAGS(HtmlListBoxTestCase,
                               "[HtmlListBoxTestCase][item-container]");

void HtmlListBoxTestCase::setUp()
{
    m_htmllbox = new wxSimpleHtmlListBox(wxTheApp->GetTopWindow(), wxID_ANY);
}

void HtmlListBoxTestCase::tearDown()
{
    wxDELETE(m_htmllbox);
}

#endif //wxUSE_HTML
