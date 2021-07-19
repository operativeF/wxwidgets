#ifndef _WX_WXSTRINGUTILS_H__
#define _WX_WXSTRINGUTILS_H__

#include <algorithm>
#include <string>
#include <string_view>

#if __cplusplus < 202011L

[[nodiscard]] bool StartsWith(std::string_view strView, std::string_view prefix) noexcept
{
    const auto prefixSize = prefix.size();

    if(strView.size() < prefixSize)
        return false;

    return strView.compare(0, prefix.size(), prefix) == 0;
}

[[nodiscard]] bool StartsWith(std::string_view strView, const char* const prefix) noexcept
{
    return StartsWith(strView, std::string_view(prefix));
}

[[nodiscard]] bool StartsWith(std::string_view strView, const char prefix) noexcept
{
    return !strView.empty() && strView.front() == prefix;
}

[[nodiscard]] bool EndsWith(std::string_view strView, std::string_view suffix) noexcept
{
    const auto suffixSize = suffix.size();

    if(strView.size() < suffixSize)
        return false;

    return strView.compare(strView.size() - suffixSize, suffixSize, suffix) == 0;
}

[[nodiscard]] bool EndsWith(std::string_view strView, const char* const suffix) noexcept
{
    return EndsWith(strView, std::string_view(suffix));
}

[[nodiscard]] bool EndsWith(std::string_view strView, const char suffix) noexcept
{
    return !strView.empty() && strView.back() == suffix;
}

#endif

#endif _WX_WXSTRINGUTILS_H__