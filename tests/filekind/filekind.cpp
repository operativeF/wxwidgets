///////////////////////////////////////////////////////////////////////////////
// Name:        tests/filetype/filetype.cpp
// Purpose:     Test wxGetFileKind and wxStreamBase::IsSeekable
// Author:      Mike Wetherell
// Copyright:   (c) 2005 Mike Wetherell
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////


#include "doctest.h"

#if wxUSE_STREAMS

#ifdef __UNIX__
    #include <sys/socket.h>
#endif

#include "wx/socket.h"
#include "wx/sckstrm.h"

#ifdef __VISUALC__
    #define isatty _isatty
    #define fdopen _fdopen
    #define fileno _fileno
#endif

#include "testfile.h"

import WX.Cmn.FFile;
import WX.Cmn.WFStream;
import WX.Cmn.MemStream;
import WX.File.Filename;
import WX.File.File;

///////////////////////////////////////////////////////////////////////////////
// The test case

// test a wxFFile and wxFFileInput/OutputStreams of a known type
//
static void TestFILE(wxFFile& file, bool expected)
{
    CHECK(file.IsOpened());
    CHECK((wxGetFileKind(file.fp()) == wxFileKind::Disk) == expected);
    CHECK((file.GetKind() == wxFileKind::Disk) == expected);

    wxFFileInputStream inStream(file);
    CHECK(inStream.IsSeekable() == expected);

    wxFFileOutputStream outStream(file);
    CHECK(outStream.IsSeekable() == expected);
}

// test a wxFile and wxFileInput/OutputStreams of a known type
//
static void TestFd(wxFile& file, bool expected)
{
    CHECK(file.IsOpened());
    CHECK((wxGetFileKind(file.fd()) == wxFileKind::Disk) == expected);
    CHECK((file.GetKind() == wxFileKind::Disk) == expected);

    wxFileInputStream inStream(file);
    CHECK(inStream.IsSeekable() == expected);

    wxFileOutputStream outStream(file);
    CHECK(outStream.IsSeekable() == expected);
}

// test with an ordinary file
//
TEST_CASE("File")
{
    TempFile tmp; // put first
    wxFile file;
    tmp.Assign(wxFileName::CreateTempFileName(wxT("wxft"), &file));
    TestFd(file, true);
    file.Close();

    wxFFile ffile(tmp.GetName());
    TestFILE(ffile, true);
}

// test with a pipe
//
#if defined __UNIX__ || defined _MSC_VER || defined __MINGW32__
TEST_CASE("Pipe")
{
    int afd[2];
    int rc;
#ifdef __UNIX__
    rc = pipe(afd);
#else
    rc = _pipe(afd, 256, O_BINARY);
#endif
    CHECK_MESSAGE(0 == rc, "Failed to create pipe");

    wxFile file0(afd[0]);
    wxFile file1(afd[1]);
    TestFd(file0, false);
    file0.Detach();

    wxFFile ffile(fdopen(afd[0], "r"));
    TestFILE(ffile, false);
}
#endif

// test with a socket
//
#if defined __UNIX__
void FileKindTestCase::Socket()
{
    int s = socket(PF_INET, SOCK_STREAM, 0);

    wxFile file(s);
    TestFd(file, false);
    file.Detach();

    wxFFile ffile(fdopen(s, "r"));
    TestFILE(ffile, false);
}
#endif

// Socket streams should be non-seekable
//
#if wxUSE_SOCKETS

TEST_CASE("SocketStream")
{
    wxSocketClient client;
    wxSocketInputStream inStream(client);
    CHECK(!inStream.IsSeekable());
    wxSocketOutputStream outStream(client);
    CHECK(!outStream.IsSeekable());

    wxBufferedInputStream nonSeekableBufferedInput(inStream);
    CHECK(!nonSeekableBufferedInput.IsSeekable());
    wxBufferedOutputStream nonSeekableBufferedOutput(outStream);
    CHECK(!nonSeekableBufferedOutput.IsSeekable());
}
#endif

// Memory streams should be seekable
//
TEST_CASE("MemoryStream")
{
    char buf[20];
    wxMemoryInputStream inStream(buf, sizeof(buf));
    CHECK(inStream.IsSeekable());
    wxMemoryOutputStream outStream(buf, sizeof(buf));
    CHECK(outStream.IsSeekable());

    wxBufferedInputStream seekableBufferedInput(inStream);
    CHECK(seekableBufferedInput.IsSeekable());
    wxBufferedOutputStream seekableBufferedOutput(outStream);
    CHECK(seekableBufferedOutput.IsSeekable());
}

// Stdin will usually be a terminal, if so then test it
//
TEST_CASE("Stdin")
{
    if (isatty(0))
        CHECK(wxGetFileKind(0) == wxFileKind::Terminal);
    if (isatty(fileno(stdin)))
        CHECK(wxGetFileKind(stdin) == wxFileKind::Terminal);
}

#endif // wxUSE_STREAMS
