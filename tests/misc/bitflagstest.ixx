///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/bitflagstest.cpp
// Purpose:     Test Bitfield class
// Author:      Thomas Figueroa
// Created:     2021-10-07
// Copyright:   (c) 2021 Thomas Figueroa
///////////////////////////////////////////////////////////////////////////////

export module WX.Test.Bitfields;

import Utils.Bitfield;
import WX.MetaTest;

namespace ut = boost::ut;

ut::suite BitfieldTests = []
{
    // Exclusive bitflag example
    enum class Borders
    {
        Single,
        Double,
        Triple,
        Raised,
        _max_size
    };

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
    
    using namespace ut;

    "Multiple value construction"_test = []
    {
        StyleFlags newStyles{Styles::Bold, Styles::Italic, Styles::Underline};

        constexpr auto newStylesSetFlags = StyleFlags::bitmask(Styles::Italic, Styles::Bold, Styles::Underline);

        expect(newStyles.as_value() == newStylesSetFlags);
    };

    "Toggle On / Off"_test = []
    {
        StyleFlags styles;

        styles.toggle(Styles::Italic);

        expect(styles.as_value() == StyleFlags::bitmask(Styles::Italic));

        styles.toggle(Styles::Italic);

        expect(styles.as_value() == 0);
    };

    "Set / Reset"_test = []
    {
        StyleFlags styles;

        styles.set(Styles::Bold);

        expect(styles.as_value() == StyleFlags::bitmask(Styles::Bold));

        styles.reset(Styles::Bold);

        expect(styles.as_value() == 0);
    };

    "Set All / Clear"_test = []
    {
        StyleFlags styles;

        styles.set_all();

        expect(styles.as_value() == StyleFlags::AllFlagsSet);

        styles.clear();

        expect(styles.as_value() == 0);
    };

    "Binary operators"_test = []
    {
        "&= operator"_test = [styles]
        {
            StyleFlags styles;

            styles.set(Styles::Strikethrough);

            styles &= Styles::Strikethrough;

            expect(styles.as_value() == StyleFlags::bitmask(Styles::Strikethrough));

            styles.set(Styles::Italic);

            styles &= Styles::Bold;

            expect(styles.as_value() == 0);
        };
    
        "|= operator"_test = [styles]
        {
            StyleFlags styles;

            styles |= Styles::Bold;
            styles |= Styles::Strikethrough;

            expect(styles.as_value() == StyleFlags::bitmask(Styles::Bold, Styles::Strikethrough));
        };
    };

    "Unary operators"_test = []
    {
        StyleFlags styles;

        "& operator"_test = [styles]
        {
            styles.set(Styles::Strikethrough, Styles::Bold);

            auto resultStyle = styles & Styles::Bold;

            expect(resultStyle.as_value() == StyleFlags::bitmask(Styles::Bold));
        };

        // SUBCASE("| operator")
        // {

        // }
    };

    "Comparison operators"_test = []
    {
        StyleFlags newStyles;
        StyleFlags styles;

        expect(styles == newStyles);

        newStyles.set(Styles::Bold);
        expect(styles != newStyles);
    };
};
