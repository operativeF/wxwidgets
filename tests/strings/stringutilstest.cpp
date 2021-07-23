#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
#include "wx/stringutils.h"
#endif // WX_PRECOMP

#include "wx/stringutils.h"

TEST_SUITE("Test auxilliary functions that work with strings")
{
    using namespace wx::utils;

    const std::string str{ "The quick; brown fox; jumped over the; lazy; dog." };

    TEST_CASE("BeforeFirst: Get substring before first delimiter")
    {
        const auto quickSubstr = BeforeFirst(str, ';');

        CHECK_EQ(quickSubstr, "The quick");

        SUBCASE("No delimiter found. Return input string.")
        {
            const auto noSubstr = BeforeFirst(str, ',');

            CHECK_EQ(noSubstr, str);
        }
    }

    TEST_CASE("BeforeLast: Get substring before last delimiter")
    {
        const auto strSansDog = BeforeLast(str, ';');

        CHECK_EQ(strSansDog, "The quick; brown fox; jumped over the; lazy");
    }

    TEST_CASE("AfterFirst: Get substring after first delimiter")
    {
        const auto strSansQuick = AfterFirst(str, ';');

        CHECK_EQ(strSansQuick, " brown fox; jumped over the; lazy; dog.");
    }

    TEST_CASE("AfterLast: Get substring after last delimiter")
    {
        const auto strJustDog = AfterLast(str, ';');

        CHECK_EQ(strJustDog, " dog.");
    }

    TEST_CASE("ReplaceAll: Replace all substrings with another substring.")
    {
        std::string replaceFoxWithDog{str};
        ReplaceAll(replaceFoxWithDog, "fox", "dog");

        CHECK_EQ(replaceFoxWithDog, "The quick; brown dog; jumped over the; lazy; dog.");
    }
}
