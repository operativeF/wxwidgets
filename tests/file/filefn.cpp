///////////////////////////////////////////////////////////////////////////////
// Name:        tests/file/filefn.cpp
// Purpose:     generic file functions unit test
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-06-13
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "wx/ffile.h"
#include "wx/filefn.h"
#include "wx/textfile.h"
#include "wx/filesys.h"

#include "testfile.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// tests implementation
// ----------------------------------------------------------------------------

static void DoConcatFile(const wxString& filePath1,
    const wxString& filePath2,
    const wxString& destFilePath)
{
    const std::string msg =
        wxString::Format("File 1: %s  File 2:%s  File 3: %s",
            filePath1, filePath2, destFilePath)
        .ToStdString();

    // Prepare source data
    wxFFile f1(filePath1, wxT("wb")),
        f2(filePath2, wxT("wb"));
    CHECK_MESSAGE(f1.IsOpened(), msg);
    CHECK_MESSAGE(f2.IsOpened(), msg);

    wxString s1(wxT("1234567890"));
    wxString s2(wxT("abcdefghij"));
    CHECK_MESSAGE(f1.Write(s1), msg);
    CHECK_MESSAGE(f2.Write(s2), msg);

    CHECK_MESSAGE(f1.Close(), msg);
    CHECK_MESSAGE(f2.Close(), msg);

    // Concatenate source files
    CHECK_MESSAGE(wxConcatFiles(filePath1, filePath2, destFilePath), msg);

    // Verify content of destination file
    CHECK_MESSAGE(wxFileExists(destFilePath), msg);
    wxString sSrc = s1 + s2;
    wxString s3;
    wxFFile f3(destFilePath, wxT("rb"));
    CHECK_MESSAGE(f3.ReadAll(&s3), msg);
    CHECK_MESSAGE(sSrc.length() == s3.length(), msg);
    CHECK_MESSAGE(memcmp(sSrc.c_str(), s3.c_str(), sSrc.length()) == 0, msg);
    CHECK_MESSAGE(f3.Close(), msg);

    CHECK_MESSAGE(wxRemoveFile(filePath1), msg);
    CHECK_MESSAGE(wxRemoveFile(filePath2), msg);
    CHECK_MESSAGE(wxRemoveFile(destFilePath), msg);
}

static void DoCreateFile(const wxString& filePath)
{
    const std::string msg = wxString::Format("File: %s", filePath).ToStdString();

    // Create temporary file.
    wxTextFile file;
    CHECK_MESSAGE(file.Create(filePath), msg);
    CHECK_MESSAGE(file.Close(), msg);

    wxRemoveFile(filePath);
}

static void DoFileExists(const wxString& filePath)
{
    const std::string msg = wxString::Format("File: %s", filePath).ToStdString();

    // Create temporary file.
    wxTextFile file;
    CHECK_MESSAGE(file.Create(filePath), msg);
    CHECK_MESSAGE(file.Close(), msg);

    // Verify that file exists with 2 methods.
    CHECK_MESSAGE(file.Exists(), msg);
    CHECK_MESSAGE(wxFileExists(filePath), msg);

    wxRemoveFile(filePath);
}

static void DoFindFile(const wxString& filePath)
{
    const std::string msg = wxString::Format("File: %s", filePath).ToStdString();

    // Create temporary file.
    wxTextFile file;
    CHECK_MESSAGE(file.Create(filePath), msg);
    CHECK_MESSAGE(file.Close(), msg);

    // Check if file can be found (method 1).
    wxString foundFile = wxFindFirstFile(filePath, wxFILE);
    CHECK_MESSAGE(foundFile == filePath, msg);

    // Check if file can be found (method 2).
    wxFileSystem fs;
    wxString furl = fs.FindFirst(filePath, wxFILE);
    wxFileName fname = wxFileName::URLToFileName(furl);
    foundFile = fname.GetFullPath();
    CHECK_MESSAGE(foundFile == filePath, msg);

    wxRemoveFile(filePath);
}

static void DoRemoveFile(const wxString& filePath)
{
    const std::string msg = wxString::Format("File: %s", filePath).ToStdString();

    // Create temporary file.
    wxTextFile file;
    CHECK_MESSAGE(file.Create(filePath), msg);
    CHECK_MESSAGE(file.Close(), msg);

    CHECK_MESSAGE(file.Exists(), msg);
    CHECK_MESSAGE(wxRemoveFile(filePath), msg);
    CHECK_MESSAGE(!file.Exists(), msg);
}

static void DoRenameFile(const wxString& oldFilePath,
    const wxString& newFilePath,
    bool overwrite,
    bool withNew)
{
    const std::string msg =
        wxString::Format(wxT("File 1: %s  File 2:%s"), oldFilePath, newFilePath)
        .ToStdString();

    // Create temporary source file.
    wxTextFile file;
    CHECK_MESSAGE(file.Create(oldFilePath), msg);
    CHECK_MESSAGE(file.Close(), msg);

    if (withNew)
    {
        // Create destination file to test overwriting.
        wxTextFile file2;
        CHECK_MESSAGE(file2.Create(newFilePath), msg);
        CHECK_MESSAGE(file2.Close(), msg);

        CHECK_MESSAGE(wxFileExists(newFilePath), msg);
    }
    else
    {
        // Remove destination file
        if (wxFileExists(newFilePath))
        {
            wxRemoveFile(newFilePath);
        }

        CHECK_MESSAGE(!wxFileExists(newFilePath), msg);
    }

    CHECK_MESSAGE(wxFileExists(oldFilePath), msg);
    CHECK_MESSAGE(wxFileExists(oldFilePath), msg);
    CHECK_MESSAGE(wxFileExists(oldFilePath), msg);
    bool shouldFail = !overwrite && withNew;
    if (shouldFail)
    {
        CHECK_MESSAGE(!wxRenameFile(oldFilePath, newFilePath, overwrite), msg);
        // Verify that file has not been renamed.
        CHECK_MESSAGE(wxFileExists(oldFilePath), msg);
        CHECK_MESSAGE(wxFileExists(newFilePath), msg);

        // Cleanup.
        wxRemoveFile(oldFilePath);
    }
    else
    {
        CHECK_MESSAGE(wxRenameFile(oldFilePath, newFilePath, overwrite), msg);
        // Verify that file has been renamed.
        CHECK_MESSAGE(!wxFileExists(oldFilePath), msg);
        CHECK_MESSAGE(wxFileExists(newFilePath), msg);
    }

    // Cleanup.
    wxRemoveFile(newFilePath);
}

TEST_SUITE("File Functions")
{
    // Initialize local data
    wxFileName fn1(wxFileName::GetTempDir(), wxT("wx_file_mask.txt"));

    // This file name is 'wx_file_mask.txt' in Russian.
    wxFileName fn2(wxFileName::GetTempDir(),
        wxT("wx_\u043C\u0430\u0441\u043A\u0430_\u0444\u0430\u0439\u043B\u0430.txt"));

    wxFileName fn3(wxFileName::GetTempDir(), wxT("wx_test_copy"));

    wxString m_fileNameASCII = fn1.GetFullPath();
    wxString m_fileNameNonASCII = fn2.GetFullPath();
    wxString m_fileNameWork = fn3.GetFullPath();

    TEST_CASE("Get temporary folder")
    {
        // Verify that obtained temporary folder is not empty.
        wxString tmpDir = wxFileName::GetTempDir();

        CHECK( !tmpDir.IsEmpty() );
    }

    TEST_CASE("Copy file")
    {
        const wxString filename1(wxS("horse.xpm"));
        const wxString& filename2 = m_fileNameWork;

        const std::string msg =
            wxString::Format("File 1: %s  File 2:%s", filename1, filename2)
                .ToStdString();

        CHECK_MESSAGE(wxCopyFile(filename1, filename2), msg);

        // verify that the two files have the same contents!
        wxFFile f1(filename1, wxT("rb")),
                f2(filename2, wxT("rb"));

        CHECK_MESSAGE(f1.IsOpened(), msg);
        CHECK_MESSAGE(f2.IsOpened(), msg);

        wxString s1, s2;
        CHECK_MESSAGE(f1.ReadAll(&s1), msg);
        CHECK_MESSAGE(f2.ReadAll(&s2), msg);
        CHECK_MESSAGE( s1 == s2, msg );

        CHECK_MESSAGE(f1.Close(), msg);
        CHECK_MESSAGE(f2.Close(), msg);
        CHECK_MESSAGE(wxRemoveFile(filename2), msg);
    }

    TEST_CASE("Create file")
    {
        // Create file name containing ASCII characters only.
        DoCreateFile(m_fileNameASCII);
        // Create file name containing non-ASCII characters.
        DoCreateFile(m_fileNameNonASCII);
    }

    TEST_CASE("File exists")
    {
        CHECK( wxFileExists(wxT("horse.png")) );
        CHECK( !wxFileExists(wxT("horse.___")) );

        // Check file name containing ASCII characters only.
        DoFileExists(m_fileNameASCII);
        // Check file name containing non-ASCII characters.
        DoFileExists(m_fileNameNonASCII);
    }

    TEST_CASE("Find file")
    {
        // Find file name containing ASCII characters only.
        DoFindFile(m_fileNameASCII);
        // Find file name containing non-ASCII characters.
        DoFindFile(m_fileNameNonASCII);
    }

    TEST_CASE("Find next file")
    {
        // Construct file name containing ASCII characters only.
        const wxString fileMask(wxT("horse.*"));

        // Check using method 1.
        wxString foundFile1 = wxFindFirstFile(fileMask, wxFILE);
        wxString foundFile2 = wxFindNextFile();
        wxFileName fn1(foundFile1);
        wxFileName fn2(foundFile2);
        // Full names must be different.
        CHECK( foundFile1 != foundFile2 );
        // Base names must be the same.
        CHECK( fn1.GetName() == fn2.GetName() );

        // Check using method 2.
        wxFileSystem fs;
        wxString furl = fs.FindFirst(fileMask, wxFILE);
        fn1 = wxFileName::URLToFileName(furl);
        foundFile1 = fn1.GetFullPath();
        furl = fs.FindNext();
        fn2 = wxFileName::URLToFileName(furl);
        foundFile2 = fn2.GetFullPath();
        // Full names must be different.
        CHECK( fn1.GetFullPath() != fn2.GetFullPath() );
        // Base names must be the same.
        CHECK( fn1.GetName() == fn2.GetName() );
    }

    TEST_CASE("Remove file")
    {
        // Create & remove file with name containing ASCII characters only.
        DoRemoveFile(m_fileNameASCII);
        // Create & remove file with name containing non-ASCII characters.
        DoRemoveFile(m_fileNameNonASCII);
    }

    TEST_CASE("Rename file")
    {
        // Verify renaming file with/without overwriting
        // when new file already exist/don't exist.
        DoRenameFile(m_fileNameASCII, m_fileNameNonASCII, false, false);
        DoRenameFile(m_fileNameASCII, m_fileNameNonASCII, false, true);
        DoRenameFile(m_fileNameASCII, m_fileNameNonASCII, true, false);
        DoRenameFile(m_fileNameASCII, m_fileNameNonASCII, true, true);
        DoRenameFile(m_fileNameNonASCII, m_fileNameASCII, false, false);
        DoRenameFile(m_fileNameNonASCII, m_fileNameASCII, false, true);
        DoRenameFile(m_fileNameNonASCII, m_fileNameASCII, true, false);
        DoRenameFile(m_fileNameNonASCII, m_fileNameASCII, true, true);
    }

    TEST_CASE("Concatenate files")
    {
        DoConcatFile(m_fileNameASCII, m_fileNameNonASCII, m_fileNameWork);
        DoConcatFile(m_fileNameNonASCII, m_fileNameASCII, m_fileNameWork);
    }

    TEST_CASE("Get current working directory")
    {
        // Verify that obtained working directory is not empty.
        wxString cwd = wxGetCwd();

        CHECK( !cwd.IsEmpty() );
    }

    TEST_CASE("Get file EOF")
    {
        const wxString filename(wxT("horse.bmp"));
        const std::string msg = wxString::Format("File: %s", filename).ToStdString();

        wxFFile file(filename, wxT("r"));
        // wxFFile::Eof must be false at start
        CHECK_MESSAGE(!file.Eof(), msg);
        CHECK_MESSAGE(file.SeekEnd(), msg);
        // wxFFile::Eof returns true only after attempt to read last byte
        char array[1];
        CHECK_MESSAGE( file.Read(array, 1) == 0, msg );
        CHECK_MESSAGE( file.Eof(), msg );

        CHECK_MESSAGE(file.Close(), msg );
        // wxFFile::Eof after close should not cause crash but fail instead
    }

    TEST_CASE("File error")
    {
        const wxString filename(wxT("horse.bmp"));
        const std::string msg = wxString::Format("File: %s", filename).ToStdString();

        wxFFile file(filename, wxT("r"));
        // wxFFile::Error must be false at start assuming file "horse.bmp" exists.
        CHECK_MESSAGE(!file.Error(), msg);
        // Attempt to write to file opened in readonly mode should cause error
        CHECK_MESSAGE(!file.Write(filename), msg);
        CHECK_MESSAGE(file.Error(), msg);

        CHECK_MESSAGE(file.Close(), msg);
    }

    TEST_CASE("Check directory existence")
    {
        wxString cwd = wxGetCwd();
        const std::string msg = wxString::Format("CWD: %s", cwd).ToStdString();

        // Current working directory must exist
        CHECK_MESSAGE(wxDirExists(cwd), msg);
    }

    TEST_CASE("Check absolute paths")
    {
        wxString name = wxT("horse.bmp");
        const std::string msg = wxString::Format("File: %s", name).ToStdString();

        // File name is given as relative path
        CHECK_MESSAGE(!wxIsAbsolutePath(name), msg);

        wxFileName filename(name);
        CHECK( filename.MakeAbsolute() );
        // wxFileName::GetFullPath returns absolute path
        CHECK_MESSAGE(wxIsAbsolutePath(filename.GetFullPath()), msg);

    #ifdef __WINDOWS__
        CHECK( wxIsAbsolutePath("\\"));
        CHECK( wxIsAbsolutePath("c:"));
        CHECK( !wxIsAbsolutePath("c"));
    #endif
    }

    TEST_CASE("Check path only")
    {
        wxString name = wxT("horse.bmp");
        // Get absolute path to horse.bmp
        wxFileName filename(name);
        CHECK( filename.MakeAbsolute() );

        wxString pathOnly = wxPathOnly(filename.GetFullPath());
        if ( !wxDirExists(pathOnly) )
            CHECK( pathOnly == wxString() );
    }

    // Unit tests for Mkdir and Rmdir doesn't cover non-ASCII directory names.
    // Rmdir fails on them on Linux. See ticket #17644.
    TEST_CASE("Check making directories")
    {
        wxString dirname = wxString::FromUTF8("__wxMkdir_test_dir_with_\xc3\xb6");
        const std::string msg = wxString::Format("Dir: %s", dirname).ToStdString();
        CHECK_MESSAGE(wxMkdir(dirname), msg);
        CHECK_MESSAGE(wxDirExists(dirname), msg);
        CHECK_MESSAGE(wxRmdir(dirname), msg);
    }

    TEST_CASE("Check removing directories.")
    {
        wxString dirname = wxString::FromUTF8("__wxRmdir_test_dir_with_\xc3\xb6");
        const std::string msg = wxString::Format("Dir: %s", dirname).ToStdString();

        CHECK_MESSAGE(wxMkdir(dirname), msg);
        CHECK_MESSAGE(wxRmdir(dirname), msg);
        CHECK_MESSAGE(!wxDirExists(dirname), msg);
    }

    TEST_CASE("Remove temp files")
    {
        // Remove all remaining temporary files
        if (wxFileExists(m_fileNameASCII))
        {
            wxRemoveFile(m_fileNameASCII);
        }
        if (wxFileExists(m_fileNameNonASCII))
        {
            wxRemoveFile(m_fileNameNonASCII);
        }
        if (wxFileExists(m_fileNameWork))
        {
            wxRemoveFile(m_fileNameWork);
        }
    }
}
/*
    TODO: other file functions to test:

wxChar* wxFileNameFromPath(wxChar *path);
wxString wxFileNameFromPath(const wxString& path);

bool wxIsWild(const wxString& pattern);

bool wxMatchWild(const wxString& pattern,  const wxString& text, bool dot_special = true);

bool wxSetWorkingDirectory(const wxString& d);

wxFileKind wxGetFileKind(int fd);
wxFileKind wxGetFileKind(FILE *fp);

bool wxIsWritable(const wxString &path);
bool wxIsReadable(const wxString &path);
bool wxIsExecutable(const wxString &path);
*/

