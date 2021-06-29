///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/headerctrltest.cpp
// Purpose:     wxHeaderCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2008-11-26
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "testprec.h"


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/headerctrl.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

class HeaderCtrlTestCase : public CppUnit::TestCase
{
public:
    HeaderCtrlTestCase() { }

    void setUp() override;
    void tearDown() override;

private:
    CPPUNIT_TEST_SUITE( HeaderCtrlTestCase );
        CPPUNIT_TEST( AddDelete );
        CPPUNIT_TEST( BestSize );
        CPPUNIT_TEST( Reorder );
    CPPUNIT_TEST_SUITE_END();

    void AddDelete();
    void BestSize();
    void Reorder();

    wxHeaderCtrlSimple *m_header;

    HeaderCtrlTestCase(const HeaderCtrlTestCase&) = delete;
	HeaderCtrlTestCase& operator=(const HeaderCtrlTestCase&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( HeaderCtrlTestCase );

// also include in its own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( HeaderCtrlTestCase, "HeaderCtrlTestCase" );

// ----------------------------------------------------------------------------
// test initialization
// ----------------------------------------------------------------------------

void HeaderCtrlTestCase::setUp()
{
    m_header = new wxHeaderCtrlSimple(wxTheApp->GetTopWindow());
}

void HeaderCtrlTestCase::tearDown()
{
    delete m_header;
    m_header = NULL;
}

// ----------------------------------------------------------------------------
// the tests themselves
// ----------------------------------------------------------------------------

void HeaderCtrlTestCase::AddDelete()
{
    CHECK_EQ( 0, m_header->GetColumnCount() );

    m_header->AppendColumn(wxHeaderColumnSimple("Column 1"));
    CHECK_EQ( 1, m_header->GetColumnCount() );

    m_header->AppendColumn(wxHeaderColumnSimple("Column 2"));
    CHECK_EQ( 2, m_header->GetColumnCount() );

    m_header->InsertColumn(wxHeaderColumnSimple("Column 0"), 0);
    CHECK_EQ( 3, m_header->GetColumnCount() );

    m_header->DeleteColumn(2);
    CHECK_EQ( 2, m_header->GetColumnCount() );
}

void HeaderCtrlTestCase::BestSize()
{
    const wxSize sizeEmpty = m_header->GetBestSize();
    // this fails under wxGTK where wxControl::GetBestSize() is 0 in horizontal
    // direction
    //CHECK( sizeEmpty.x > 0 );
    CHECK( sizeEmpty.y > 0 );

    m_header->AppendColumn(wxHeaderColumnSimple("Foo"));
    m_header->AppendColumn(wxHeaderColumnSimple("Bar"));
    const wxSize size = m_header->GetBestSize();
    CHECK_EQ( sizeEmpty.y, size.y );
}

void HeaderCtrlTestCase::Reorder()
{
    static constexpr int COL_COUNT = 4;

    int n;

    for ( n = 0; n < COL_COUNT; n++ )
        m_header->AppendColumn(wxHeaderColumnSimple(wxString::Format("%d", n)));

    auto order = m_header->GetColumnsOrder(); // initial order: [0 1 2 3]
    for ( n = 0; n < COL_COUNT; n++ )
        CHECK_EQ( n, order[n] );

    wxHeaderCtrl::MoveColumnInOrderArray(order, 0, 2);
    m_header->SetColumnsOrder(order);   // change order to [1 2 0 3]

    order = m_header->GetColumnsOrder();
    CHECK_EQ( 1, order[0] );
    CHECK_EQ( 2, order[1] );
    CHECK_EQ( 0, order[2] );
    CHECK_EQ( 3, order[3] );

    order[2] = 3;
    order[3] = 0;
    m_header->SetColumnsOrder(order);   // and now [1 2 3 0]
    order = m_header->GetColumnsOrder();
    CHECK_EQ( 1, order[0] );
    CHECK_EQ( 2, order[1] );
    CHECK_EQ( 3, order[2] );
    CHECK_EQ( 0, order[3] );

    wxHeaderCtrl::MoveColumnInOrderArray(order, 1, 3);
    m_header->SetColumnsOrder(order);    // finally [2 3 0 1]
    order = m_header->GetColumnsOrder();
    CHECK_EQ( 2, order[0] );
    CHECK_EQ( 3, order[1] );
    CHECK_EQ( 0, order[2] );
    CHECK_EQ( 1, order[3] );
}

