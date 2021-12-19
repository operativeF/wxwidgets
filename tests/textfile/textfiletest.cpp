///////////////////////////////////////////////////////////////////////////////
// Name:        tests/textfile/textfile.cpp
// Purpose:     wxTextFile unit test
// Author:      Vadim Zeitlin
// Created:     2006-03-31
// Copyright:   (c) 2006 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#if wxUSE_TEXTFILE

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#include "wx/ffile.h"

import WX.Cmn.TextFile;

#ifdef __VISUALC__
    #define unlink _unlink
#endif

// return the name of the test file we use
constexpr char GetTestFileName[] = "textfiletest.txt";

static void CreateTestFile(size_t len, const char *contents)
{
    FILE *f = fopen(GetTestFileName, "wb");
    CHECK( f );

    CHECK_EQ( len, fwrite(contents, 1, len, f) );
    CHECK_EQ( 0, fclose(f) );
}

// create the test file with the given contents
static void CreateTestFile(const char* contents)
{
    CreateTestFile(strlen(contents), contents);
}


TEST_CASE("Text file tests")
{
    srand((unsigned)time(NULL));

    SUBCASE("ReadEmpty")
    {
        CreateTestFile("");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );

        CHECK_EQ( (size_t)0, f.GetLineCount() );
        CHECK( f.Eof() );
        CHECK_EQ( "", f.GetFirstLine() );
        CHECK_EQ( "", f.GetLastLine() );
    }

    SUBCASE("ReadDOS")
    {
        CreateTestFile("foo\r\nbar\r\nbaz");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );

        CHECK_EQ( (size_t)3, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Dos, f.GetLineType(0) );
        CHECK_EQ( wxTextFileType::None, f.GetLineType(2) );
        CHECK_EQ( wxString(wxT("bar")), f.GetLine(1) );
        CHECK_EQ( wxString(wxT("baz")), f.GetLastLine() );
    }

    SUBCASE("ReadDOSLast")
    {
        CreateTestFile("foo\r\n");

        wxTextFile f;
        CHECK( f.Open(GetTestFileName) );

        CHECK_EQ( 1, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Dos, f.GetLineType(0) );
        CHECK_EQ( "foo", f.GetFirstLine() );
    }

    SUBCASE("ReadUnix")
    {
        CreateTestFile("foo\nbar\nbaz");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );

        CHECK_EQ( (size_t)3, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Unix, f.GetLineType(0) );
        CHECK_EQ( wxTextFileType::None, f.GetLineType(2) );
        CHECK_EQ( wxString(wxT("bar")), f.GetLine(1) );
        CHECK_EQ( wxString(wxT("baz")), f.GetLastLine() );
    }

    SUBCASE("ReadUnixLast")
    {
        CreateTestFile("foo\n");

        wxTextFile f;
        CHECK( f.Open(GetTestFileName) );

        CHECK_EQ( 1, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Unix, f.GetLineType(0) );
        CHECK_EQ( "foo", f.GetFirstLine() );
    }

    SUBCASE("ReadMac")
    {
        CreateTestFile("foo\rbar\r\rbaz");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );

        CHECK_EQ( (size_t)4, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Mac, f.GetLineType(0) );
        CHECK_EQ( wxTextFileType::Mac, f.GetLineType(1) );
        CHECK_EQ( wxTextFileType::Mac, f.GetLineType(2) );
        CHECK_EQ( wxTextFileType::None, f.GetLineType(3) );
        CHECK_EQ( wxString(wxT("foo")), f.GetLine(0) );
        CHECK_EQ( wxString(wxT("bar")), f.GetLine(1) );
        CHECK_EQ( wxString(wxT("")), f.GetLine(2) );
        CHECK_EQ( wxString(wxT("baz")), f.GetLastLine() );
    }

    SUBCASE("ReadMacLast")
    {
        CreateTestFile("foo\r");

        wxTextFile f;
        CHECK( f.Open(GetTestFileName) );

        CHECK_EQ( 1, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Mac, f.GetLineType(0) );
        CHECK_EQ( "foo", f.GetFirstLine() );
    }

    SUBCASE("ReadMixed")
    {
        CreateTestFile("foo\rbar\r\nbaz\n");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );

        CHECK_EQ( (size_t)3, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Mac, f.GetLineType(0) );
        CHECK_EQ( wxTextFileType::Dos, f.GetLineType(1) );
        CHECK_EQ( wxTextFileType::Unix, f.GetLineType(2) );
        CHECK_EQ( wxString(wxT("foo")), f.GetFirstLine() );
        CHECK_EQ( wxString(wxT("bar")), f.GetLine(1) );
        CHECK_EQ( wxString(wxT("baz")), f.GetLastLine() );
    }

    SUBCASE("ReadMixedWithFuzzing")
    {
        for ( int iteration = 0; iteration < 100; iteration++)
        {
            // Create a random buffer with lots of newlines. This is intended to catch
            // bad parsing in unexpected situations such as the one from ReadCRCRLF()
            // (which is so common it deserves a test of its own).
            static constexpr char CHOICES[] = {'\r', '\n', 'X'};

            const size_t BUF_LEN = 100;
            char data[BUF_LEN + 1];
            data[0] = 'X';
            data[BUF_LEN] = '\0';
            unsigned linesCnt = 0;
            for ( size_t i = 1; i < BUF_LEN; i++ )
            {
                char ch = CHOICES[rand() % WXSIZEOF(CHOICES)];
                data[i] = ch;
                if ( ch == '\r' || (ch == '\n' && data[i-1] != '\r') )
                    linesCnt++;
            }
            if (data[BUF_LEN-1] != '\r' && data[BUF_LEN-1] != '\n')
                linesCnt++; // last line was unterminated

            CreateTestFile(data);

            wxTextFile f;
            CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );
            CHECK_EQ( (size_t)linesCnt, f.GetLineCount() );
        }
    }

    SUBCASE("ReadCRCRLF")
    {
        // Notepad may create files with CRCRLF line endings (see
        // https://stackoverflow.com/questions/6998506/text-file-with-0d-0d-0a-line-breaks).
        // Older versions of wx would loose all data when reading such files.
        // Test that the data are read, but don't worry about empty lines in between or
        // line endings. Also include a longer streak of CRs, because they can
        // happen as well.
        CreateTestFile("foo\r\r\nbar\r\r\r\nbaz\r\r\n");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName)) );

        wxString all;
        for ( wxString str = f.GetFirstLine(); !f.Eof(); str = f.GetNextLine() )
            all += str;

        CHECK_EQ( "foobarbaz", all );
    }

    SUBCASE("ReadUTF8")
    {
        CreateTestFile("\xd0\x9f\n"
                       "\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82");

        wxTextFile f;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName), wxConvUTF8) );

        CHECK_EQ( (size_t)2, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Unix, f.GetLineType(0) );
        CHECK_EQ( wxTextFileType::None, f.GetLineType(1) );
        CHECK_EQ( wxString(L"\u041f"), f.GetFirstLine() );
        CHECK_EQ( wxString(L"\u0440\u0438\u0432\u0435\u0442"),
                              f.GetLastLine() );
    }

    SUBCASE("ReadUTF16")
    {
        CreateTestFile(16,
                       "\x1f\x04\x0d\x00\x0a\x00"
                       "\x40\x04\x38\x04\x32\x04\x35\x04\x42\x04");

        wxTextFile f;
        wxMBConvUTF16LE conv;
        CHECK( f.Open(wxString::FromAscii(GetTestFileName), conv) );

        CHECK_EQ( (size_t)2, f.GetLineCount() );
        CHECK_EQ( wxTextFileType::Dos, f.GetLineType(0) );
        CHECK_EQ( wxTextFileType::None, f.GetLineType(1) );

        CHECK_EQ( wxString(L"\u041f"), f.GetFirstLine() );
        CHECK_EQ( wxString(L"\u0440\u0438\u0432\u0435\u0442"),
                              f.GetLastLine() );
    }

    SUBCASE("ReadBig")
    {
        static constexpr size_t NUM_LINES = 10000;

        {
            wxFFile f(GetTestFileName, "w");
            for ( size_t n = 0; n < NUM_LINES; n++ )
            {
                fprintf(f.fp(), "Line %lu\n", (unsigned long)n + 1);
            }
        }

        wxTextFile f;
        CHECK( f.Open(GetTestFileName) );

        CHECK_EQ( NUM_LINES, f.GetLineCount() );
        CHECK_EQ( wxString("Line 1"), f[0] );
        CHECK_EQ( wxString("Line 999"), f[998] );
        CHECK_EQ( wxString("Line 1000"), f[999] );
        CHECK_EQ( wxString::Format("Line %lu", (unsigned long)NUM_LINES),
                              f[NUM_LINES - 1] );
    }

    #ifdef __LINUX__

    // Check if using wxTextFile with special files, whose reported size doesn't
    // correspond to the real amount of data in them, works.
    SUBCASE("wxTextFile::Special")
    {
        // LXC containers don't (always) populate /proc and /sys, so skip these
        // tests there.
        if ( IsRunningInLXC() )
            return;

        SECTION("/proc")
        {
            wxTextFile f;
            CHECK( f.Open("/proc/cpuinfo") );
            CHECK( f.GetLineCount() > 1 );
        }

        SECTION("/sys")
        {
            wxTextFile f;
            CHECK( f.Open("/sys/power/state") );
            REQUIRE( f.GetLineCount() == 1 );
            INFO( "/sys/power/state contains \"" << f[0] << "\"" );
            CHECK( (f[0].find("mem") != wxString::npos || f[0].find("disk") != wxString::npos) );
        }
    }

    #endif // __LINUX__

    unlink(GetTestFileName);
}

#endif // wxUSE_TEXTFILE
