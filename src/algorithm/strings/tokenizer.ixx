module;

export module Utils.Strings.Tokenizer;

import <string_view>;

export namespace wx::utils
{

enum class StringTokenizerMode
{
    Invalid,   // set by def ctor until SetString() is called
    Default,        // strtok() for whitespace delims, RET_EMPTY else
    RetEmpty,      // return empty token in the middle of the string
    RetEmptyAll,  // return trailing empty tokens too
    RetDelims,     // return the delim with token (implies RET_EMPTY)
    StrTok          // behave exactly like strtok(3)
};

inline constexpr char DEFAULT_DELIMITERS[] = "\t\r\n";

class StringTokenizer
{
public:
    using iter = std::string_view::iterator;

    constexpr StringTokenizer() {}

    constexpr StringTokenizer(std::string_view str,
                              const std::string& delims = DEFAULT_DELIMITERS,
                              StringTokenizerMode mode = StringTokenizerMode::Default)
        : m_delims{delims}
    {
    }

    constexpr std::string_view Remaining() const
    {
        return {m_pos, m_str.end()};
    }


private:
    std::string m_delims;

};

} // export namespace wx::utils
