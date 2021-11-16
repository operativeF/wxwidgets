export module Utils.Strings.Modifying;

import Utils.Chars;

import <algorithm>;
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
constexpr void TrimAllSpace(std::string& str)
{
    std::erase_if(str, isWhitespace);
}

// FIXME: Not valid for unicode strings.
constexpr void TrimLeadingSpace(std::string& str)
{
    auto it = std::find_if_not(str.begin(), str.end(), isWhitespace);

    str.erase(str.begin(), it);
}

// FIXME: Not valid for unicode strings.
constexpr void TrimTrailingSpace(std::string& str)
{
    auto it = std::find_if_not(str.rbegin(), str.rend(), isWhitespace);

    str.erase(it.base(), str.end());
}

// FIXME: Not valid for unicode strings.
constexpr void ToUpper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](auto c) noexcept { return ToUpperCh(c); });
}

// FIXME: Not valid for unicode strings.
constexpr void ToLower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](auto c) noexcept { return ToLowerCh(c); });
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
