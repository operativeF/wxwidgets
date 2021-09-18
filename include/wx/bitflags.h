#ifndef _BITFLAGS_H_
#define _BITFLAGS_H_

#include <concepts>

namespace wx::type
{

template<unsigned int N, typename C>
struct Flag : C
{
    using base = C;
    static constexpr unsigned int value = N;
};

namespace detail
{

template<typename... Flags>
struct BitFlagsImpl : Flags... {};

} // namespace detail

template<typename BF, typename... Flags>
concept SameBase = requires
{
    requires(std::derived_from<BF, Flags> && ...);
};

template<typename... Flags>
class InclBitflags
{
public:
    using BF = detail::BitFlagsImpl<Flags...>;

    constexpr InclBitflags() noexcept {}

    constexpr InclBitflags(auto... flags) noexcept requires(SameBase<BF, decltype(flags)...>)
    {
        SetFlags(flags...);
    }

    constexpr void SetFlag(auto flag) noexcept requires(SameBase<BF, decltype(flag)>)
    {
        m_flags |= decltype(flag)::value;
    }

    constexpr void SetFlags(auto... flags) noexcept requires(SameBase<BF, decltype(flags)...>)
    {
        m_flags |= (decltype(flags)::value | ...);
    }

    constexpr void ClearFlag(auto flag) noexcept requires(SameBase<BF, decltype(flag)>)
    {
        m_flags &= ~decltype(flag)::value;
    }

    constexpr void ClearFlags(auto... flags) noexcept requires(SameBase<BF, decltype(flags)...>)
    {
        m_flags &= ~(decltype(flags)::value | ...);
    }

    constexpr void Reset() noexcept
    {
        m_flags = 0;
    }

    constexpr auto GetFlags() const noexcept
    {
        return m_flags;
    }

    constexpr bool IsFlagSet(auto flag) const noexcept requires(SameBase<BF, decltype(flag)>)
    {
        return m_flags & decltype(flag)::value;
    }

    constexpr auto operator|=(auto flag) noexcept requires(SameBase<BF, decltype(flag)>)
    {
        m_flags |= decltype(flag)::value;
        return this;
    }

private:
    unsigned int m_flags{};
};

template<typename... Flags>
class ExclBitflags
{
public:
    using BF = detail::BitFlagsImpl<Flags...>;

    constexpr ExclBitflags() {}

    constexpr ExclBitflags(auto flag) noexcept requires(SameBase<BF, decltype(flag)>)
    {
        SetFlag(flag);
    }

    constexpr void SetFlag(auto flag) noexcept requires(SameBase<BF, decltype(flag)>)
    {
        m_flag = decltype(flag)::value;
    }

    constexpr void Reset() noexcept
    {
        m_flag = 0;
    }

    constexpr auto GetFlag() const noexcept
    {
        return m_flag;
    }

    constexpr auto GetFlagAsType() const noexcept
    {
        return Flag<m_flag>{};
    }

    constexpr bool IsFlagSet(auto flag) const noexcept requires(SameBase<BF, decltype(flag)>)
    {
        return m_flag & decltype(flag)::value;
    }

private:
    unsigned int m_flag{};
};


} // namespace wx::type

#endif // _BITFLAGS_H_