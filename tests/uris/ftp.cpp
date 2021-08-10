///////////////////////////////////////////////////////////////////////////////
// Name:        tests/uris/ftp.cpp
// Purpose:     wxFTP unit test
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-05-23
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include <wx/protocol/ftp.h>

// For this to run, the following environment variables need to be defined:
//
//  - WX_FTP_TEST_HOST: the host to use for testing (e.g. ftp.example.com)
//  - WX_FTP_TEST_DIR: the directory in which to perform most of the tests
//  - WX_FTP_TEST_FILE: name of an existing file in this directory
//
// Optionally, WX_FTP_TEST_USER and WX_FTP_TEST_PASS may also be defined,
// otherwise anonymous FTP is used.
TEST_CASE("FTP")
{
    std::string hostname{ wxGetEnv("WX_FTP_TEST_HOST") };
    std::string directory{ wxGetEnv("WX_FTP_TEST_DIR") };
    std::string valid_filename{ wxGetEnv("WX_FTP_TEST_FILE") };

    if ( hostname.empty() || directory.empty() || valid_filename.empty() )
    {
        WARN("Skipping FTPTestCase because required WX_FTP_TEST_XXX "
             "environment variables are not defined.");
        return;
    }

    const wxString user = wxGetenv("WX_FTP_TEST_USER");
    const wxString password = wxGetenv("WX_FTP_TEST_PASS");

    class SocketInit
    {
    public:
        SocketInit() { wxSocketBase::Initialize(); }
        ~SocketInit() { wxSocketBase::Shutdown(); }
    } socketInit;

    // wxFTP cannot be a static variable as its ctor needs to access
    // wxWidgets internals after it has been initialized
    wxFTP ftp;

    if ( !user.empty() )
    {
        ftp.SetUser(user);
        ftp.SetPassword(password);
    }

    REQUIRE( ftp.Connect(hostname) );

    SUBCASE("List")
    {
        // test CWD
        CHECK( ftp.ChDir(directory) );

        // test NLIST and LIST
        std::vector<std::string> files;
        CHECK( ftp.GetFilesList(files) );
        CHECK( ftp.GetDirList(files) );

        CHECK( ftp.ChDir("..") );
    }

    SUBCASE("Download")
    {
        CHECK( ftp.ChDir(directory) );

        // test RETR
        wxInputStream *in1 = ftp.GetInputStream("bloordyblop");
        CHECK( in1 == NULL );
        delete in1;

        wxInputStream *in2 = ftp.GetInputStream(valid_filename);
        CHECK( in2 != NULL );

        size_t size = in2->GetSize();
        wxChar *data = new wxChar[size];
        CHECK( in2->Read(data, size).GetLastError() == wxSTREAM_NO_ERROR );

        delete [] data;
        delete in2;
    }

    SUBCASE("FileSize")
    {
        CHECK( ftp.ChDir(directory) );

        CHECK( ftp.FileExists(valid_filename) );

        int size = ftp.GetFileSize(valid_filename);
        CHECK( size != -1 );
    }

    SUBCASE("Pwd")
    {
        CHECK_EQ( "/", ftp.Pwd() );

        CHECK( ftp.ChDir(directory) );
        CHECK_EQ( directory, ftp.Pwd() );
    }

    SUBCASE("Misc")
    {
        CHECK( ftp.SendCommand("STAT") == '2' );
        CHECK( ftp.SendCommand("HELP SITE") == '2' );
    }

    SUBCASE("Upload")
    {
        if ( user.empty() )
        {
            WARN("Skipping upload test when using anonymous FTP.");
            return;
        }

        // upload a file
        static const std::string file1 = "test1";
        wxOutputStream *out = ftp.GetOutputStream(file1);
        CHECK( out != nullptr );
        CHECK( out->Write("First hello", 11).GetLastError() == wxSTREAM_NO_ERROR );
        delete out;

        // send a command to check the remote file
        CHECK( ftp.SendCommand("STAT " + file1) == '2' );
        CHECK( ftp.GetLastResult() == "11" );
    }
}
