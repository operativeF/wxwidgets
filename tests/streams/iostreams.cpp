///////////////////////////////////////////////////////////////////////////////
// Name:        tests/streams/iostreams.cpp
// Purpose:     unit test for input/output streams
// Author:      Vadim Zeitlin
// Created:     2008-06-15
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"


#if wxUSE_STREAMS

import WX.File.Filename;
import WX.Cmn.WFStream;

// --------------------------------------------------------------------------
// test class
// --------------------------------------------------------------------------

class IOStreamsTestCase : public CppUnit::TestCase
{
public:
    IOStreamsTestCase() { }

    void tearDown() override
    {
        if ( !m_fnTemp.empty() )
        {
            wxRemoveFile(m_fnTemp);
            m_fnTemp.clear();
        }
    }

private:
    CPPUNIT_TEST_SUITE( IOStreamsTestCase );
        CPPUNIT_TEST( FStream );
        CPPUNIT_TEST( FFStream );
    CPPUNIT_TEST_SUITE_END();

    void FStream() { wxFileStream s(GetTempFName()); DoTest(s); }
    void FFStream() { wxFFileStream s(GetTempFName()); DoTest(s); }

    wxString GetTempFName()
    {
        m_fnTemp = wxFileName::CreateTempFileName("wxtest");
        return m_fnTemp;
    }

    template <class Stream>
    void DoTest(Stream& s)
    {
        s.PutC('x');
        CPPUNIT_ASSERT_EQUAL( 1, s.LastWrite() );

        s.SeekI(0);
        CPPUNIT_ASSERT_EQUAL( int('x'), s.GetC() );
    }

    wxString m_fnTemp;

    IOStreamsTestCase(const IOStreamsTestCase&) = delete;
	IOStreamsTestCase& operator=(const IOStreamsTestCase&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( IOStreamsTestCase );

// also include in its own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( IOStreamsTestCase, "IOStreamsTestCase" );

#endif // wxUSE_STREAMS
