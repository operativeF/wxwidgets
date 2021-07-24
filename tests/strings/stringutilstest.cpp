#include "doctest.h"

#include "testprec.h"

#ifndef WX_PRECOMP
#include "wx/stringutils.h"
#endif // WX_PRECOMP

#include "wx/stringutils.h"

#include <array>

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

    TEST_CASE("Erase: Remove all designated characters from a string.")
    {
        std::string commaDelimitedDogsAndCats{"Dog, dog, dog, cat, cat."};
        Erase(commaDelimitedDogsAndCats, ',');

        CHECK_EQ(commaDelimitedDogsAndCats, "Dog dog dog cat cat.");
    }

    TEST_CASE("EraseIf: Remove all designated characters if the input functor is satisifed.")
    {
        constexpr std::array<char, 3> punctuationMarks {'?', '.', '!'};
        auto isPunctuation = [=](auto&& c){ return std::find(punctuationMarks.begin(), punctuationMarks.end(), c) != punctuationMarks.end(); };

        std::string excessivePunctuation{"Not. A. Chance? Not a... Chance?! NOT. A. CHANCE!!!"};

        EraseIf(excessivePunctuation, isPunctuation);

        CHECK_EQ(excessivePunctuation, "Not A Chance Not a Chance NOT A CHANCE");
    }
}
