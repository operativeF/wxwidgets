///////////////////////////////////////////////////////////////////////////////
// Name:        tests/file/filetest.cpp
// Purpose:     wxFile unit test
// Author:      Vadim Zeitlin
// Created:     2009-09-12
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////


#include "doctest.h"

#include "testfile.h"

import WX.File.File;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// tests implementation
// ----------------------------------------------------------------------------

TEST_CASE("ReadAll")
{
    TestFile tf;

    const char* text = "Ream\nde";

    {
        wxFile fout(tf.GetName(), wxFile::write);
        CHECK( fout.IsOpened() );
        fout.Write(text, strlen(text));
        CHECK( fout.Close() );
    }

    {
        wxFile fin(tf.GetName(), wxFile::read);
        CHECK( fin.IsOpened() );

        wxString s;
        CHECK( fin.ReadAll(&s) );
        CHECK_EQ( text, s );
    }
}

static void DoRoundTripTest(const wxMBConv& conv)
{
    TestFile tf;

    // Explicit length is needed because of the embedded NUL.
    const wxString data("Hello\0UTF!", 10);

    {
        wxFile fout(tf.GetName(), wxFile::write);
        CHECK( fout.IsOpened() );

        CHECK( fout.Write(data, conv) );
    }

    {
        wxFile fin(tf.GetName(), wxFile::read);
        CHECK( fin.IsOpened() );

        const ssize_t len = fin.Length();
        wxCharBuffer buf(len);
        CHECK_EQ( len, fin.Read(buf.data(), len) );

        wxString dataReadBack(buf, conv, len);
        CHECK_EQ( data, dataReadBack );
    }

    {
        wxFile fin(tf.GetName(), wxFile::read);
        CHECK( fin.IsOpened() );

        wxString dataReadBack;
        CHECK( fin.ReadAll(&dataReadBack, conv) );

        CHECK_EQ( data, dataReadBack );
    }
}

TEST_CASE("RoundTripUTF8")  { DoRoundTripTest(wxConvUTF8); }
TEST_CASE("RoundTripUTF16") { DoRoundTripTest(wxMBConvUTF16()); }
TEST_CASE("RoundTripUTF32") { DoRoundTripTest(wxMBConvUTF32()); }

TEST_CASE("TempFile")
{
    wxTempFile tmpFile;
    CHECK( tmpFile.Open(wxT("test2")) );
    CHECK( tmpFile.Write(wxT("the answer is 42")) );
    CHECK( tmpFile.Commit() );
    CHECK( wxRemoveFile(wxT("test2")) );
}

#ifdef __LINUX__

// Check that GetSize() works correctly for special files.
TEST_CASE("wxFile::Special", "[file][linux][special-file]")
{
    // LXC containers don't (always) populate /proc and /sys, so skip these
    // tests there.
    if ( IsRunningInLXC() )
        return;

    // We can't test /proc/kcore here, unlike in the similar
    // wxFileName::GetSize() test, as wxFile must be able to open it (at least
    // for reading) and usually we don't have the permissions to do it.

    // This file is not seekable and has 0 size, but can still be read.
    wxFile fileProc("/proc/cpuinfo");
    CHECK( fileProc.IsOpened() );

    wxString s;
    CHECK( fileProc.ReadAll(&s) );
    CHECK( !s.empty() );

    // All files in /sys have the size of one kernel page, even if they don't
    // have that much data in them.
    const long pageSize = sysconf(_SC_PAGESIZE);

    wxFile fileSys("/sys/power/state");
    CHECK( fileSys.Length() == pageSize );
    CHECK( fileSys.IsOpened() );
    CHECK( fileSys.ReadAll(&s) );
    CHECK( !s.empty() );
    CHECK( s.length() < pageSize );
}

#endif // __LINUX__
