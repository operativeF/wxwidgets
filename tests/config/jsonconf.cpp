#include "doctest.h"

#include "testprec.h"

#include "wx/fileconf_json.h"

TEST_SUITE("Test creation and manipulation of JSON settings files.")
{
    namespace cfg = wx::cfg;
    namespace fs  = std::filesystem;

    TEST_CASE("Create a JSON configuration file.")
    {
        fs::path localpath{fs::current_path()};

        localpath /= "properties.json";

        cfg::FileConfigJson local(localpath);

        CHECK(local.ContainsKey("appName"));

        CHECK(local.ContainsSubkey("configurations", "name"));

        CHECK_FALSE(local.ContainsSubkey("configurations", "version"));

        CHECK_FALSE(local.ContainsKey("names"));
    }

    TEST_CASE("Manipulate program configuration settings.")
    {

    }

    TEST_CASE("Save the configuration settings.")
    {
        
    }
}
