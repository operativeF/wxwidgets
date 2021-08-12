#include "doctest.h"

#include "testprec.h"

#include "wx/msw/config_reg.h"

using namespace wx::cfg;

TEST_SUITE("Test read-write of configuration settings through config_type interface.")
{
    TEST_CASE("Test adding a registry key")
    {
        RegistryContainer new_cont{ "Luckyware", "Test" };

        new_cont.SetString("config0", "nil");

        auto read_value = new_cont.ReadString("config0");

        CHECK_EQ(read_value, "nil");

        new_cont.SetDword("config1", DWORD(0x001));

        auto enumSubkeys = new_cont.EnumerateValues();

        CHECK_EQ(enumSubkeys[0].first, "config0");
        // Cleanup keys
        //new_cont.Clear();
    }
}
