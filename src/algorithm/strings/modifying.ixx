export module Utils.Strings.Modifying;

import Utils.Chars;

import <algorithm>;
import <ranges>;
import <string>;
import <string_view>;

export namespace wx::utils
{

constexpr std::size_t ReplaceAll(std::string& instr, std::string_view candidate, std::string_view replacement)
{
    std::size_t count{ 0 };
    for (std::string::size_type pos{};
        instr.npos != (pos = instr.find(candidate.data(), pos, candidate.length()));
        pos += replacement.length(), ++count)
    {
        instr.replace(pos, candidate.length(), replacement.data(), replacement.length());
    }

    return count;
}

constexpr std::size_t ReplaceAll(std::wstring& instr, std::wstring_view candidate, std::wstring_view replacement)
{
    std::size_t count{ 0 };
    for (std::wstring::size_type pos{};
        instr.npos != (pos = instr.find(candidate.data(), pos, candidate.length()));
        pos += replacement.length(), ++count)
    {
        instr.replace(pos, candidate.length(), replacement.data(), replacement.length());
    }

    return count;
}

// FIXME: Not valid for unicode strings.
// Trims all space leading and following, but not in the middle.
template<typename R>
constexpr void TrimAllSpace(R& str)
{
    auto it1 = std::ranges::find_if_not(str, isWhitespace);
    auto it2 = std::ranges::find_if_not(std::ranges::reverse_view(str), isWhitespace);

    str.erase(it2.base(), str.end());
    str.erase(str.begin(), it1);
}

// FIXME: Not valid for unicode strings.
template<typename R>
constexpr void TrimLeadingSpace(R& str)
{
    auto it = std::ranges::find_if_not(str, isWhitespace);

    str.erase(str.begin(), it);
}

// FIXME: Not valid for unicode strings.
template<typename R>
constexpr void TrimTrailingSpace(R& str)
{
    auto it = std::ranges::find_if_not(std::ranges::reverse_view(str), isWhitespace);

    str.erase(it.base(), str.end());
}

// FIXME: Not valid for unicode strings.
template<typename R>
constexpr void ToUpper(R& str)
{
    std::ranges::transform(str, str.begin(), [](auto c) noexcept { return ToUpperCh(c); });
}

// FIXME: Not valid for unicode strings.
template<typename R>
constexpr void ToLower(R& str)
{
    std::ranges::transform(str, str.begin(), [](auto c) noexcept { return ToLowerCh(c); });
}

constexpr void EraseSubstr(std::string& str, std::string_view subToErase)
{
    std::size_t pos{std::string::npos};

    while((pos = str.find(subToErase)) != std::string::npos)
    {
        str.erase(pos, subToErase.length());
    }
}

} // export namespace wx::utils
