export module Utils.Strings.Unsafe;

import <string_view>;
import <vector>;

// Generally unsafe utilities to be used in circumstances where
// speed is important and / or lifetimes are a certainty.
export namespace wx::unsafe
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

} // namespace wx::unsafe
