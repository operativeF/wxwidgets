///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/bitflagstest.cpp
// Purpose:     Test Bitfield class
// Author:      Thomas Figueroa
// Created:     2021-10-07
// Copyright:   (c) 2021 Thomas Figueroa
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/bitflags.h"

TEST_CASE("Bitflags")
{
    // Combinable bitflag example
    enum class Styles
    {
        Italic,
        Bold,
        Underline,
        Strikethrough,
        _max_size
    };

    using StyleFlags = InclBitfield<Styles>;

    StyleFlags styles;

    SUBCASE("Multiple value construction")
    {
        StyleFlags newStyles{Styles::Bold, Styles::Italic, Styles::Underline};

        constexpr auto newStylesSetFlags = StyleFlags::bitmask(Styles::Italic, Styles::Bold, Styles::Underline);

        CHECK(newStyles.as_value() == newStylesSetFlags);
    }

    SUBCASE("Toggle On / Off")
    {
        styles.toggle(Styles::Italic);

        CHECK(styles.as_value() == StyleFlags::bitmask(Styles::Italic));

        styles.toggle(Styles::Italic);

        CHECK(styles.as_value() == 0);
    }

    SUBCASE("Set / Reset")
    {
        styles.set(Styles::Bold);

        CHECK(styles.as_value() == StyleFlags::bitmask(Styles::Bold));

        styles.reset(Styles::Bold);

        CHECK(styles.as_value() == 0);
    }

    SUBCASE("Set All / Clear")
    {
        styles.set_all();

        CHECK(styles.as_value() == StyleFlags::AllFlagsSet);

        styles.clear();

        CHECK(styles.as_value() == 0);
    }

    SUBCASE("Binary operators")
    {
        SUBCASE("&= operator")
        {
            styles.set(Styles::Strikethrough);

            styles &= Styles::Strikethrough;

            CHECK(styles.as_value() == StyleFlags::bitmask(Styles::Strikethrough));

            styles.set(Styles::Italic);

            styles &= Styles::Bold;

            CHECK(styles.as_value() == 0);
        }
    
        SUBCASE("|= operator")
        {
            styles |= Styles::Bold;
            styles |= Styles::Strikethrough;

            CHECK(styles.as_value() == StyleFlags::bitmask(Styles::Bold, Styles::Strikethrough));

            styles.clear();
        }
    }

    SUBCASE("Unary operators")
    {
        SUBCASE("& operator")
        {
            styles.set(Styles::Strikethrough, Styles::Bold);

            auto resultStyle = styles & Styles::Bold;

            CHECK(resultStyle.as_value() == StyleFlags::bitmask(Styles::Bold));
        }

        SUBCASE("| operator")
        {

        }
    }

    SUBCASE("Comparison operators")
    {
        StyleFlags newStyles;
        CHECK(styles == newStyles);

        newStyles.set(Styles::Bold);
        CHECK(styles != newStyles);
    }
}

TEST_CASE("Combinable Bitfields")
{
    enum class Borders
    {
        Single,
        Double,
        Triple,
        Raised,
        _max_size
    };

    enum class Alignment
    {
        Top,
        Left,
        Bottom,
        Right,
        _max_size
    };

    CombineBitfield<Borders, Alignment> cb_field{};

    SUBCASE("Toggle All")
    {
        CHECK(cb_field.empty());

        cb_field.toggle_all();

        CHECK(cb_field.as_value() == 0xFF);

        cb_field.clear();
    }

    SUBCASE("Set All")
    {
        CHECK(cb_field.empty());

        cb_field.set_all();

        CHECK(cb_field.as_value() == 0xFF);

        cb_field.clear();
    }
}
