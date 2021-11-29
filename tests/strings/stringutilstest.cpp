#include "doctest.h"

#include "testprec.h"

import Utils.Strings;

import <array>;

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
        auto isPunctuation = [=](auto&& c){ return std::ranges::find(punctuationMarks, c) != punctuationMarks.end(); };

        std::string excessivePunctuation{"Not. A. Chance? Not a... Chance?! NOT. A. CHANCE!!!"};

        EraseIf(excessivePunctuation, isPunctuation);

        CHECK_EQ(excessivePunctuation, "Not A Chance Not a Chance NOT A CHANCE");
    }

    TEST_CASE("ToUpper: Modify a string to uppercase (non-unicode).")
    {
        std::string convertToYelling{"The quick brown fox jumped over the lazy dog!"};

        ToUpper(convertToYelling);

        CHECK_EQ(convertToYelling, "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG!");
    }

    TEST_CASE("ToLower: Modify a string to lowercase (non-unicode).")
    {
        std::string stopYelling("YOU CAN'T TELL ME WHAT TO DO!");

        ToLower(stopYelling);

        CHECK_EQ(stopYelling, "you can't tell me what to do!");
    }

    TEST_CASE("CmpNoCase: Compare two strings, case insensitive.")
    {
        std::string originalStr{"The quick brown fox jumped over the lazy dog."};
        std::string strCaps("THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG.");
        std::string strDiffSize("The quick brown fox.");

        CHECK(CmpNoCase(originalStr, strCaps) == 0);
        CHECK_FALSE(CmpNoCase(originalStr, strDiffSize) == 0);
    }

    TEST_CASE("Contains: Check if a substring is contained inside of another string.")
    {
        std::string containsFox{"fox"};
        std::string doesNotContainBog{"bog"};

        CHECK(Contains(str, containsFox));
        CHECK_FALSE(Contains(str, doesNotContainBog));
    }

    TEST_CASE("ContainsNoCase: Check if a substring is contained inside of another string, ignoring case.")
    {
        std::string containsFox{"fox"};
        std::string containsCapsFox{"FOX"};
        std::string doesNotContainCapsBog{"BOG"};

        CHECK(ContainsNoCase(str, containsFox));
        CHECK(ContainsNoCase(str, containsCapsFox));
        CHECK_FALSE(ContainsNoCase(str, doesNotContainCapsBog));
    }

    TEST_CASE("StrSplit: A string splitting function that splits on a single character delimiter.")
    {
        std::string csvCat{"Cat,,cat, cat, cat;, cat."};

        static const std::vector<std::string> catSplit {
            "Cat",
            "cat",
            " cat",
            " cat;",
            " cat."
        };

        CHECK_EQ(StrSplit(csvCat, ','), catSplit);
    }

    TEST_CASE("StrSplitEscape: A splitting function that provides a way to escape delimiters.")
    {
        std::string csvPuppy{"Puppy,puppy,\\,puppy,puppy\\,puppy."};

        static const std::vector<std::string> puppyWithEscape {
            "Puppy",
            "puppy",
            ",puppy",
            "puppy,puppy."
        };

        CHECK_EQ(StrSplitEscape(csvPuppy, ',', '\\'), puppyWithEscape);
    }
}
