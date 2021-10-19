#ifndef _WX_WXSTRINGUTILS_H__
#define _WX_WXSTRINGUTILS_H__

#include "wx/charutils.h"

#include <gsl/gsl>

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace wx::utils
{

// Modifying string functions

[[maybe_unused]] constexpr std::size_t ReplaceAll(std::string& instr, std::string_view candidate, std::string_view replacement)
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

[[maybe_unused]] constexpr std::size_t ReplaceAll(std::wstring& instr, std::wstring_view candidate, std::wstring_view replacement)
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

// FIXME: Not valid for unicode strings.
[[nodiscard]] constexpr std::string ToUpperCopy(std::string_view str)
{
    std::string out;
    out.resize(str.size());

    std::transform(str.begin(), str.end(), out.begin(), [](auto c) noexcept { return ToUpperCh(c); });

    return out;
}

// FIXME: Not valid for unicode strings.
[[nodiscard]] constexpr std::string ToLowerCopy(std::string_view str)
{
    std::string out;
    out.resize(str.size());

    std::transform(str.begin(), str.end(), out.begin(), [](unsigned char c) noexcept { return ToLowerCh(c); });

    return out;
}

[[nodiscard]] constexpr std::vector<std::string> StrSplit(std::string_view strView, char delim)
{
    std::vector<std::string> output;

    size_t first{0};

    while (first < strView.size())
    {
        const auto second = strView.find_first_of(delim, first);

        if (first != second)
            output.emplace_back(strView.substr(first, second - first));

        if (second == std::string_view::npos)
            break;

        first = second + 1;
    }

    return output;
}

// FIXME: could be improved, but is sufficient for string conversion.
[[nodiscard]] constexpr std::vector<std::string> StrSplitEscape(std::string_view strView, char delim, char escape)
{
    std::vector<std::string> output;

    std::string s;

    for(std::string_view::iterator i = strView.begin(); i != strView.end();++i)
    {
        const auto ch = *i;

        if(ch != delim && ch != escape)
        {
            s += ch;
        }
        
        if(ch == escape)
        {
            if(*(i + 1) == delim)
            {
                std::advance(i, 1);
                s += delim;
            }
            else
            {
                s += ch;
            }
        }
        
        if(ch == delim || i == std::prev(strView.end(), 1))
        {
            output.emplace_back(s);
            s.clear();

            // special case of empty delimiter at end
            if(ch == delim && i == (strView.end() - 1))
            {
                output.emplace_back("");
            }
        }
    }

    return output;
}

// Non-modifying string functions

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
[[nodiscard]] constexpr int CmpNoCase(std::string_view strViewA, std::string_view strViewB)
{
    const auto nA = strViewA.size();

    std::string strA(nA, '0');
    std::string strB(nA, '0');

    std::transform(strViewA.begin(), strViewA.end(), strA.begin(), [](auto c) noexcept { return ToLowerCh(c); });
    std::transform(strViewB.begin(), strViewB.end(), strB.begin(), [](auto c) noexcept { return ToLowerCh(c); });

    return strA.compare(strB);
}

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
[[nodiscard]] constexpr int CmpNoCase(const char* const chsA, const char* const chsB)
{
    return CmpNoCase(std::string_view(chsA), std::string_view(chsB));
}

[[nodiscard]] constexpr bool IsSameAsCase(std::string_view strViewA, std::string_view strViewB) noexcept
{
    return strViewA == strViewB;
}

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
[[nodiscard]] constexpr bool IsSameAsNoCase(std::string_view strViewA, std::string_view strViewB) 
{
    return CmpNoCase(strViewA, strViewB) == 0;
}

[[nodiscard]] constexpr std::string BeforeFirst(std::string_view strView, std::string_view strFirst, size_t pos = 0)
{
    const auto n = strView.find(strFirst, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));
    
    return std::string(strView);
}

[[nodiscard]] constexpr std::string BeforeFirst(std::string_view strView, const char ch, size_t pos = 0)
{
    const auto n = strView.find(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));
    
    return std::string(strView);
}

[[nodiscard]] constexpr std::string BeforeFirst(std::string_view strView, const char* const chs, size_t pos = 0)
{
    return BeforeFirst(strView, std::string_view(chs), pos);
}

[[nodiscard]] constexpr std::string AfterFirst(std::string_view strView, std::string_view strAfter, size_t pos = 0)
{
    const auto n = strView.find(strAfter, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(n + 1, strView.size()));
    
    return {};
}

[[nodiscard]] constexpr std::string AfterFirst(std::string_view strView, const char ch, size_t pos = 0)
{
    const auto n = strView.find(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(n + 1, strView.size()));
    
    return {};
}

[[nodiscard]] constexpr std::string AfterFirst(std::string_view strView, const char* const chs, size_t pos = 0)
{
    return AfterFirst(strView, std::string_view(chs), pos);
}

[[nodiscard]] constexpr std::string BeforeLast(std::string_view strView, std::string_view strBefore, size_t pos = std::string_view::npos)
{
    const auto n = strView.rfind(strBefore, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));

    return {};
}

[[nodiscard]] constexpr std::string BeforeLast(std::string_view strView, const char ch, size_t pos = std::string_view::npos)
{
    const auto n = strView.rfind(ch, pos);

    if(n != std::string_view::npos)
        return std::string(strView.substr(0, n));

    return {};
}

[[nodiscard]] constexpr std::string BeforeLast(std::string_view strView, const char* const chs, size_t pos = std::string_view::npos)
{
    return BeforeLast(strView, std::string_view(chs), pos);
}


// TODO: Do we really want to return the whole input string if it fails to find anything?
[[nodiscard]] constexpr std::string AfterLast(std::string_view strView, std::string_view strLast, size_t pos = std::string_view::npos)
{
    const auto n = strView.rfind(strLast, pos);

    if(n != std::string_view::npos)
        return std::string{strView.substr(n + 1, strView.size())};

    return std::string{strView};
}

[[nodiscard]] constexpr std::string AfterLast(std::string_view strView, const char ch, size_t pos = std::string_view::npos)
{
    const auto n = strView.rfind(ch, pos);

    if(n != std::string_view::npos)
        return std::string{strView.substr(n + 1, strView.size())};

    return std::string{strView};
}

[[nodiscard]] constexpr std::string AfterLast(std::string_view strView, const char* const chs, size_t pos = std::string_view::npos)
{
    return AfterLast(strView, std::string_view{chs}, pos);
}

[[nodiscard]] constexpr bool Contains(std::string_view strView, std::string_view strToFind) noexcept
{
    return strView.find(strToFind, 0) != std::string_view::npos;
}

// FIXME: Not valid for unicode strings
[[nodiscard]] constexpr bool ContainsNoCase(std::string_view strView, std::string_view strToFind)
{
    std::string str{strView};
    std::string substrToFind{strToFind};

    ToLower(str);
    ToLower(substrToFind);

    return str.find(substrToFind) != std::string::npos;
}

} // namespace wx::util

// Generally unsafe utilities to be used in circumstances where
// speed is important and / or lifetimes are a certainty.
namespace wx::unsafe
{

// From an input string_view, split the view on each delimiter and store each split in a vector.
// Delimiter not included.
constexpr std::vector<std::string_view> StrViewSplit(std::string_view strView, char delim)
{
    std::vector<std::string_view> output;

    size_t first{0};

    while (first < strView.size())
    {
        const auto second = strView.find_first_of(delim, first);

        if (first != second)
            output.emplace_back(strView.substr(first, second - first));

        if (second == std::string_view::npos)
            break;

        first = second + 1;
    }

    return output;
}

}

#endif _WX_WXSTRINGUTILS_H__
