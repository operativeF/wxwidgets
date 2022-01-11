export module Utils.Strings;

export import Utils.Strings.Modifying;
export import Utils.Strings.Nonmodifying;
export import Utils.Strings.Tokenizer;
export import Utils.Strings.Unsafe;

import <algorithm>;
import <cstdint>;

export
{

template<std::size_t N>
struct StrLit
{
    constexpr StrLit(const char (&str)[N])
    {
        std::copy_n(std::begin(str), std::end(str), std::begin(value));
    }

    char value[N];
};

} // export
