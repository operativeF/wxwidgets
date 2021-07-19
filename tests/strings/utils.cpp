#include "doctest.h"

#include "testprec.h"

#include "wx/stringutils.h"

#include <array>

namespace wu = wx::utils;

TEST_SUITE("String utilities")
{
    TEST_CASE("Starts with")
    {
        std::string a{"quick fox"};
        std::string_view b{"lazy dog"};

        CHECK(wu::StartsWith(a, "quick"));
        CHECK(wu::StartsWith(b, "lazy"));
    }

    TEST_CASE("Ends with")
    {
        std::string a{ "quick fox" };
        std::string_view b{ "lazy dog" };

        CHECK(wu::EndsWith(a, "fox"));
        CHECK(wu::EndsWith(b, "dog"));
    }

    TEST_CASE("Before first")
    {
        std::string a{ "quick fox" };
        std::string_view b{ "lazy dog" };

        CHECK_EQ(wu::BeforeFirst(a, " "), "quick");
        CHECK_EQ(wu::BeforeFirst(b, " "), "lazy");
    }

    TEST_CASE("After first")
    {
        std::string a{ "quick fox" };
        std::string_view b{ "lazy dog" };

        CHECK_EQ(wu::AfterFirst(a, " "), "fox");
        CHECK_EQ(wu::AfterFirst(b, " "), "dog");

        SUBCASE("Boundary checking")
        {
            std::string c{"quickfox "};
            std::string d{ " quickfox" };

            CHECK_EQ(wu::AfterFirst(c, " "), "");
            CHECK_EQ(wu::AfterFirst(d, " "), "quickfox");
        }
    }

    TEST_CASE("After last")
    {
        std::string a{ "quick fox" };
        std::string_view b{ "lazy dog" };

        CHECK_EQ(wu::AfterLast(a, " "), "quick fox");
        CHECK_EQ(wu::AfterLast(b, " "), "lazy dog");

        SUBCASE("Boundary checking")
        {
            std::string c{"quickfox "};
            std::string d{ " quickfox" };

            CHECK_EQ(wu::AfterLast(c, " "), "quickfox ");
            CHECK_EQ(wu::AfterLast(d, " "), "quickfox");
        }
    }

    TEST_CASE("Comparison (case insensitive); ASCII only")
    {
        std::string_view a{"nOt A cHaNcE."};
        std::string_view b{"not a chance."};
        std::string_view c{"NOT A CHANCE."};

        CHECK(wu::CmpNoCase(a, b));
        CHECK(wu::CmpNoCase(b, c));
        CHECK(wu::CmpNoCase(a, c));
    }

    TEST_CASE("String erase")
    {
        std::string commasep{"10,20,30,40,50"};
        std::string capremoval{"no MAMES"};
        std::string vowelremoval{"expedite"};

        static constexpr std::array<char, 5> vowels {'a', 'e', 'i', 'o', 'u'};

        auto commasRemoved = wu::Erase(commasep, ',');
        auto capsRemoved   = wu::EraseIf(capremoval, [](char c) { return std::isupper(c); });
        auto vowelsRemoved = wu::EraseIf(vowelremoval, [](char c) {
                return std::find(vowels.cbegin(), vowels.cend(), c) != vowels.cend();
        });

        CHECK_EQ(commasep, "1020304050");
        CHECK_EQ(capremoval, "no ");
        CHECK_EQ(vowelremoval, "xpdt");

        CHECK_EQ(commasRemoved, 4);
        CHECK_EQ(capsRemoved, 5);
        CHECK_EQ(vowelsRemoved, 4);
    }
}
