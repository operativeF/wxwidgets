#ifndef _BITFLAGS_H_
#define _BITFLAGS_H_

#include <concepts>

template<typename Enum>
concept BitfieldCompatible =  requires
{
    requires(std::is_enum_v<Enum>);
    requires(std::unsigned_integral<std::underlying_type_t<Enum>>);
    {Enum::_max_size};
};

template<typename Enum, typename Tag> requires(BitfieldCompatible<Enum>)
class Bitfield;

template<typename Enum>
using InclBitfield = Bitfield<Enum, struct InclType>;

template<typename Enum>
using MaskBitfield = Bitfield<Enum, struct MaskType>;

template<typename Enum, typename Tag> requires(BitfieldCompatible<Enum>)
class Bitfield
{
public:
    // TODO: Specialize based on the number of bits.
    using value_type = std::underlying_type_t<Enum>;

    explicit constexpr Bitfield(value_type bits) : m_fields{bits}
    {
    }

    explicit constexpr Bitfield(const Enum& e)
    {
        set(e);
    }

    static constexpr value_type bitmask(const Enum& e) noexcept
    {
        return value_type{1} << static_cast<std::size_t>(e);
    }

    constexpr auto clear() noexcept
    {
        m_fields = value_type{0};
    }

    constexpr void set(const Enum& e) noexcept
    {
        m_fields |= bitmask(e);
    }

    constexpr void reset(const Enum& e) noexcept
    {
        m_fields &= ~bitmask(e);
    }

    constexpr void toggle(const Enum& e) noexcept
    {
        m_fields ^= bitmask(e);
    }

    constexpr void set_all() noexcept
    {
        m_fields = (value_type{1} << static_cast<value_type>(Enum::_max_size)) - value_type{1};
    }

    constexpr auto operator|=(const Enum& e) noexcept
    {
        m_fields |= static_cast<std::size_t>(e);
    }

    constexpr auto operator&=(const Enum& e) noexcept
    {
        m_fields &= static_cast<std::size_t>(e);
    }

    constexpr auto as_value() const noexcept
    {
        return m_fields;
    }

    auto operator<=>(const Bitfield&) const noexcept = default;

private:
    value_type m_fields;
};



#endif // _BITFLAGS_H_
