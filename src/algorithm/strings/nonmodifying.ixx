module;

#include <fmt/core.h>
#include <fmt/format.h>

export module Utils.Strings.Nonmodifying;

import Utils.Chars;

import <algorithm>;
import <span>;
import <string>;
import <string_view>;
import <vector>;

export namespace wx::utils
{

// FIXME: Not valid for unicode strings.
constexpr std::string ToUpperCopy(std::string_view str)
{
    std::string out;
    out.resize(str.size());

    std::transform(str.begin(), str.end(), out.begin(), [](auto c) noexcept { return ToUpperCh(c); });

    return out;
}

// FIXME: Not valid for unicode strings.
constexpr std::string ToLowerCopy(std::string_view str)
{
    std::string out;
    out.resize(str.size());

    std::transform(str.begin(), str.end(), out.begin(), [](unsigned char c) noexcept { return ToLowerCh(c); });

    return out;
}

constexpr std::vector<std::string> StrSplit(std::string_view strView, char delim)
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
constexpr std::vector<std::string> StrSplitEscape(std::string_view strView, char delim, char escape)
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

// Join a '\0' delimited array of chars into a vector of strings.
std::vector<std::string> JoinChArray(std::span<const char> chSpan)
{
    std::vector<std::string> vec;

    for(auto chIter = chSpan.begin(); chIter != chSpan.end(); chIter = std::next(chIter))
    {
        auto chBeginIter = chIter;

        while(*chIter != '\0')
        {
            chIter = std::next(chIter);
        }

        vec.emplace_back(chBeginIter, chIter);
    }

    return vec;
}

// Delimit strings from a span of them.
std::string JoinStrings(std::span<const std::string> strSpan, char delim)
{
    return fmt::format("{}", fmt::join(strSpan, std::string{delim}));
}

// Like JoinStrings, except there is an
// escape character inserted before a delimiter that is found in a string.
std::string JoinStringsEsc(std::span<const std::string> strSpan, char delim, std::string_view esc)
{
    std::string str;

    const auto escBack = fmt::format("{}{}", esc, delim);

    for(auto&& name : strSpan)
    {
        for(auto&& ch : name)
        {
            if(ch != delim)
            {
                str += ch;
            }
            else
            {
                str += escBack;
            }
        }

        str += delim;
    }

    str.pop_back(); // Remove extra delimiter
    return str;
}

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
int CmpNoCase(const std::string& strViewA, const std::string& strViewB)
{
    const auto nA = strViewA.size();
    const auto nB = strViewB.size();

    std::string strA(nA, '0');
    std::string strB(nB, '0');

    std::transform(strViewA.begin(), strViewA.end(), strA.begin(), [](auto c) noexcept { return wx::utils::ToLowerCh(c); });
    std::transform(strViewB.begin(), strViewB.end(), strB.begin(), [](auto c) noexcept { return wx::utils::ToLowerCh(c); });

    return strA.compare(strB);
}

constexpr bool IsSameAsCase(std::string_view strViewA, std::string_view strViewB) noexcept
{
    return strViewA == strViewB;
}

// FIXME: Wrong (for Unicode), and temporary implementation of a case insensitive string comparison
bool IsSameAsNoCase(const std::string& strViewA, const std::string& strViewB) noexcept
{
    return CmpNoCase(strViewA, strViewB) == 0;
}

[[nodiscard]] constexpr std::string StripAllSpace(const std::string& str)
{
    std::string tmpStr{str};

    std::erase_if(tmpStr, isWhitespace);

    return tmpStr;
}

constexpr std::string StripLeadingSpace(const std::string& str)
{
    std::string tmpStr{str};
    auto it = std::find_if_not(tmpStr.begin(), tmpStr.end(), isWhitespace);

    tmpStr.erase(tmpStr.begin(), it);

    return tmpStr;
}

constexpr std::string StripTrailingSpace(const std::string& str)
{
    std::string tmpStr{str};

    auto it = std::find_if_not(tmpStr.rbegin(), tmpStr.rend(), isWhitespace);

    tmpStr.erase(it.base(), str.end());

    return tmpStr;
}

bool IsSameAs(const std::string& strViewA, const std::string& strViewB, bool bCase) noexcept
{
    if(bCase)
    {
        return IsSameAsCase(strViewA, strViewB);
    }
    else
    {
        return IsSameAsNoCase(strViewA, strViewB);
    }
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
    std::string str = ToLowerCopy(strView);
    std::string substrToFind = ToLowerCopy(strToFind);

    return str.find(substrToFind) != std::string::npos;
}

[[nodiscard]] constexpr bool StartsWith(const std::string& strView, std::string_view prefix, std::string& afterPrefix)
{
    bool prefixFound = strView.rfind(prefix, 0) == 0;

    if(prefixFound)
    {
        afterPrefix = strView.substr(prefix.length());

        return true;
    }

    return false;
}

} // export namespace wx::utils
