///////////////////////////////////////////////////////////////////////////////
// Name:        tests/file/dir.cpp
// Purpose:     wxDir unit test
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-06-19
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/dir.h"
#include "wx/filename.h"
#include "wx/stdpaths.h"
#include "wx/stringutils.h"

#include <filesystem>

#include <boost/nowide/convert.hpp>
#include <fmt/core.h>

static const std::string DIRTEST_FOLDER = "dirTest_folder";
static const std::string SEP = "\\"; // FIXME: Change for all systems

// ----------------------------------------------------------------------------
// tests implementation
// ----------------------------------------------------------------------------

static void CreateTempFile(const std::string& path)
{
    wxFile f(path, wxFile::write);
    f.Write("dummy test file");
    f.Close();
}

static void setUp()
{
    // create a test directory hierarchy
    wxDir::Make(DIRTEST_FOLDER + SEP + "folder1" + SEP + "subfolder1", wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    wxDir::Make(DIRTEST_FOLDER + SEP + "folder1" + SEP + "subfolder2", wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    wxDir::Make(DIRTEST_FOLDER + SEP + "folder2", wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    wxDir::Make(DIRTEST_FOLDER + SEP + "folder3" + SEP + "subfolder1", wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

    CreateTempFile(DIRTEST_FOLDER + SEP + "folder1" + SEP + "subfolder2" + SEP + "dummy");
    CreateTempFile(DIRTEST_FOLDER + SEP + "dummy");
    CreateTempFile(DIRTEST_FOLDER + SEP + "folder3" + SEP + "subfolder1" + SEP + "dummy.foo");
    CreateTempFile(DIRTEST_FOLDER + SEP + "folder3" + SEP + "subfolder1" + SEP + "dummy.foo.bar");
}

static void tearDown()
{
    wxRemove(DIRTEST_FOLDER + SEP + "folder1" + SEP + "subfolder2" + SEP + "dummy");
    wxRemove(DIRTEST_FOLDER + SEP + "dummy");
    wxRemove(DIRTEST_FOLDER + SEP + "folder3" + SEP + "subfolder1" + SEP + "dummy.foo");
    wxRemove(DIRTEST_FOLDER + SEP + "folder3" + SEP + "subfolder1" + SEP + "dummy.foo.bar");
    wxDir::Remove(DIRTEST_FOLDER, wxPATH_RMDIR_RECURSIVE);
}

static std::vector<std::string> DirEnumHelper(wxDir& dir,
                                           int flags = wxDIR_DEFAULT,
                                           const std::string& filespec = {})
{
    std::vector<std::string> ret;
    CHECK( dir.IsOpened() );

    std::string filename;
    bool cont = dir.GetFirst(&filename, filespec, flags);
    while ( cont )
    {
        ret.push_back(filename);
        cont = dir.GetNext(&filename);
    }

    return ret;
}

TEST_CASE("Directory Tests")
{
    setUp();

    SUBCASE("Enum")
    {
        wxDir dir(DIRTEST_FOLDER);
        CHECK( dir.IsOpened() );

        // enumerating everything in test directory
        CHECK_EQ(4, DirEnumHelper(dir).size());

        // enumerating really everything in test directory recursively
        CHECK_EQ(6, DirEnumHelper(dir, wxDIR_DEFAULT | wxDIR_DOTDOT).size());

        // enumerating object files in test directory
        CHECK_EQ(0, DirEnumHelper(dir, wxDIR_DEFAULT, "*.o*").size());

        // enumerating directories in test directory
        CHECK_EQ(3, DirEnumHelper(dir, wxDIR_DIRS).size());

        // enumerating files in test directory
        CHECK_EQ(1, DirEnumHelper(dir, wxDIR_FILES).size());

        // enumerating files including hidden in test directory
        CHECK_EQ(1, DirEnumHelper(dir, wxDIR_FILES | wxDIR_HIDDEN).size());

        // enumerating files and folders in test directory
        CHECK_EQ(4, DirEnumHelper(dir, wxDIR_FILES | wxDIR_DIRS).size());
    }

    class TestDirTraverser : public wxDirTraverser
    {
    public:
        std::vector<std::string> dirs;

        wxDirTraverseResult OnFile(const std::string& WXUNUSED(filename)) override
        {
            return wxDIR_CONTINUE;
        }

        wxDirTraverseResult OnDir(const std::string& dirname) override
        {
            dirs.push_back(dirname);
            return wxDIR_CONTINUE;
        }
    };

    SUBCASE("Traverse")
    {
        // enum all files
        std::vector<std::string> files;
        CHECK_EQ(4, wxDir::GetAllFiles(DIRTEST_FOLDER, &files));

        // enum all files according to the filter
        CHECK_EQ(1, wxDir::GetAllFiles(DIRTEST_FOLDER, &files, "*.foo"));

        // enum again with custom traverser
        wxDir dir(DIRTEST_FOLDER);
        TestDirTraverser traverser;
        dir.Traverse(traverser, "", wxDIR_DIRS | wxDIR_HIDDEN);
        CHECK_EQ(6, traverser.dirs.size());
    }

    SUBCASE("DirExists")
    {
        struct
        {
            const char *dirname;
            bool shouldExist;
        } testData[] =
        {
            { ".", true },
            { "..", true },
            { "$USER_DOCS_DIR", true },
    #if defined(__WINDOWS__)
            { "$USER_DOCS_DIR\\", true },
            { "$USER_DOCS_DIR\\\\", true },
            { "..\\..", true },
            { "$MSW_DRIVE", true },
            { "$MSW_DRIVE\\", true },
            { "$MSW_DRIVE\\\\", true },
            { "\\\\non_existent_share\\file", false },
            { "$MSW_DRIVE\\a\\directory\\which\\does\\not\\exist", false },
            { "$MSW_DRIVE\\a\\directory\\which\\does\\not\\exist\\", false },
            { "$MSW_DRIVE\\a\\directory\\which\\does\\not\\exist\\\\", false },
            { "test.exe", false }            // not a directory!
    #elif defined(__UNIX__)
            { "../..", true },
            { "/", true },
            { "//", true },
            { "/usr/bin", true },
            { "/usr//bin", true },
            { "/usr///bin", true },
            { "/tmp/a/directory/which/does/not/exist", false },
            { "/bin/ls", false }             // not a directory!
    #endif
        };

    #ifdef __WINDOWS__
        std::string homedrive = wxGetenv("HOMEDRIVE");
        if ( homedrive.empty() )
            homedrive = "c:";
    #endif // __WINDOWS__

        for ( size_t n = 0; n < WXSIZEOF(testData); n++ )
        {
            std::string dirname = testData[n].dirname;
            wx::utils::ReplaceAll(dirname, "$USER_DOCS_DIR", wxStandardPaths::Get().GetDocumentsDir().ToStdString());

    #ifdef __WINDOWS__
            wx::utils::ReplaceAll(dirname, "$MSW_DRIVE", homedrive);
    #endif // __WINDOWS__

            std::string errDesc = fmt::format("failed on directory '%s'", dirname);
            CHECK_MESSAGE(testData[n].shouldExist == wxDir::Exists(dirname), errDesc);

            wxDir d(dirname);
            CHECK_EQ(testData[n].shouldExist, d.IsOpened());
        }

        CHECK( wxDir::Exists(wxGetCwd()) );
    }

    SUBCASE("GetName")
    {
        wxDir d;

        CHECK( d.Open(".") );
        CHECK( d.GetName().back() != wxFILE_SEP_PATH );
        CHECK( d.GetNameWithSep().back() == wxFILE_SEP_PATH );
        CHECK_EQ( d.GetName() + wxFILE_SEP_PATH,
                              d.GetNameWithSep() );

    #ifdef __UNIX__
        CHECK( d.Open("/") );
        CHECK_EQ( "/", d.GetName() );
        CHECK_EQ( "/", d.GetNameWithSep() );
    #endif
    }

    tearDown();
}
