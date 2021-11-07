/////////////////////////////////////////////////////////////////////////////
// Name:        wx/colour.h
// Purpose:     wxColourBase definition
// Author:      Julian Smart
// Modified by: Francesco Montorsi
// Created:
// Copyright:   Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLOUR_H_BASE_
#define _WX_COLOUR_H_BASE_

#include "wx/defs.h"

#include "wx/gdiobj.h"
#include "wx/colorspace.h"
#include "wx/string.h"
#include "wx/stringutils.h"

#include <boost/tmp.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>

class wxColour;

// A macro to define the standard wxColour constructors:
//
// It avoids the need to repeat these lines across all colour.h files, since
// Set() is a virtual function and thus cannot be called by wxColourBase ctors
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
#define wxWXCOLOUR_CTOR_FROM_CHAR \
    wxColour(const char *colourName) { Init(); Set(colourName); }
#else // wxNO_IMPLICIT_WXSTRING_ENCODING
#define wxWXCOLOUR_CTOR_FROM_CHAR
#endif
#define DEFINE_STD_WXCOLOUR_CONSTRUCTORS                                      \
    wxColour() { Init(); }                                                    \
    wxColour(ChannelType red,                                                 \
             ChannelType green,                                               \
             ChannelType blue,                                                \
             ChannelType alpha = wxALPHA_OPAQUE)                              \
        { Init(); Set(red, green, blue, alpha); }                             \
    wxColour(unsigned long colRGB) { Init(); Set(colRGB    ); }               \
    wxColour(const wxString& colourName) { Init(); Set(colourName); }         \
    wxWXCOLOUR_CTOR_FROM_CHAR                                                 \
    wxColour(const wchar_t *colourName) { Init(); Set(colourName); }


// flags for wxColour -> wxString conversion (see wxColour::GetAsString)
enum {
    wxC2S_NAME             = 1,   // return colour name, when possible
    wxC2S_CSS_SYNTAX       = 2,   // return colour in rgb(r,g,b) syntax
    wxC2S_HTML_SYNTAX      = 4    // return colour in #rrggbb syntax
};

inline constexpr unsigned char wxALPHA_TRANSPARENT = 0;
inline constexpr unsigned char wxALPHA_OPAQUE = 0xff;

// a valid but fully transparent colour
#define wxTransparentColour wxColour(0, 0, 0, wxALPHA_TRANSPARENT)
#define wxTransparentColor wxTransparentColour

// ----------------------------------------------------------------------------
// wxVariant support
// ----------------------------------------------------------------------------

#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT_EXPORTED(wxColour)
#endif

namespace wx::color
{

using color8_t = std::byte;

namespace detail
{
    constexpr bool isValidHexSeq(std::span<const char> str)
    {
        return std::all_of(str.begin(), str.end(), wx::utils::isHex);
    }

    template<std::size_t HexLength>
    constexpr std::array<color8_t, HexLength> ToUpperCase(std::string_view str)
    {
        std::array<color8_t, HexLength> strarr;

        auto strview = str | std::views::transform(wx::utils::ToUpperCh) | std::views::transform([](char c) { return static_cast<color8_t>(c); });

        std::ranges::copy(strview, strarr.begin());

        return strarr;
    }

    constexpr color8_t XXAsByte(unsigned int hex)
    {
        return wx::utils::HexCharToDec(static_cast<color8_t>(hex));
    }

    constexpr color8_t XXAsByte(color8_t hexA, color8_t hexB)
    {
        return (wx::utils::HexCharToDec(hexA) << 4) | wx::utils::HexCharToDec(hexB);
    }

} // namespace detail

using RRGGBBAA_Format = std::array<color8_t, 8>;
using RGBA_Format     = std::array<color8_t, 4>;
using RGBA_IntFormat  = std::uint32_t;

template<typename DataRed,
         typename DataGreen = DataRed,
         typename DataBlue  = DataRed,
         colorspace::rgb::RGBColorSpaceable ColorSpace = colorspace::rgb::sRGB>
class RGB
{
    using color_space = ColorSpace;
    using linear_color = boost::tmp::false_;

    DataRed   r{};
    DataGreen g{};
    DataBlue  b{};
};

template<typename DataRed,
         typename DataGreen = DataRed,
         typename DataBlue  = DataRed,
         typename DataAlpha = DataRed,
         colorspace::rgb::RGBColorSpaceable ColorSpace = colorspace::rgb::sRGB>
class RGBA
{
    using color_space = ColorSpace;
    using linear_color = boost::tmp::false_;

    DataRed   r{};
    DataGreen g{};
    DataBlue  b{};
    DataAlpha a{};
};

template<typename DataRed,
         typename DataGreen = DataRed,
         typename DataBlue  = DataRed,
         colorspace::rgb::RGBColorSpaceable ColorSpace = colorspace::rgb::sRGB>
class LinearRGB
{
    using color_space = ColorSpace;
    using linear_color = boost::tmp::true_;

    DataRed   r{};
    DataGreen g{};  
    DataBlue  b{};
};

template<typename DataRed   = float,
         typename DataGreen = float,
         typename DataBlue  = float,
         typename DataAlpha = float,
         colorspace::rgb::RGBColorSpaceable ColorSpace = colorspace::rgb::sRGB>
class LinearRGBA
{
    using color_space = ColorSpace;
    using linear_color = boost::tmp::true_;
    
    DataRed   r{};
    DataGreen g{};
    DataBlue  b{};
    DataAlpha a{};
};

class ColorRGBA
{
public:
    constexpr ColorRGBA() noexcept {}

    constexpr ColorRGBA(color8_t red,
                        color8_t green,
                        color8_t blue,
                        color8_t alpha = color8_t{wxALPHA_OPAQUE}) noexcept
    {
        value = { red, green, blue, alpha };
    }

    // Constructor takes 8 hex values (0x0 - 0xF) in the format of RRGGBBAA.
    constexpr ColorRGBA(const RRGGBBAA_Format& colorArr)
        : value{ detail::XXAsByte(colorArr[0], colorArr[1]),
                 detail::XXAsByte(colorArr[2], colorArr[3]),
                 detail::XXAsByte(colorArr[4], colorArr[5]),
                 detail::XXAsByte(colorArr[6], colorArr[7]) }
    {
    }

    // Constructor takes 4 hex values (0x00 - 0xFF) in the format of RGBA.
    constexpr ColorRGBA(const RGBA_Format& colorArr)
        : value{colorArr}
    {
    }

    // Constructor takes a std::uint32_t hex value of the form RRGGBBAA.
    constexpr ColorRGBA(RGBA_IntFormat rgb) noexcept
        : value{ detail::XXAsByte(rgb & 0xFF),
                 detail::XXAsByte((rgb >> 8) & 0xFF),
                 detail::XXAsByte((rgb >> 16) & 0xFF),
                 detail::XXAsByte((rgb >> 24) & 0xFF)
               }
    {
    }

    constexpr RGBA_IntFormat AsInt32() const
    {
        return (static_cast<RGBA_IntFormat>(red()))         |
               (static_cast<RGBA_IntFormat>(green()) << 8)  |
               (static_cast<RGBA_IntFormat>(blue())  << 16) |
               (static_cast<RGBA_IntFormat>(alpha()) << 24);
    }

    constexpr bool operator==(const ColorRGBA& rgba) noexcept
    {
        return value == rgba.value;
    }

    constexpr bool operator!=(const ColorRGBA& rgba) noexcept
    {
        return !(*this == rgba);
    }

    constexpr color8_t red() const noexcept
    {
        return value[0];
    }

    constexpr color8_t& red() noexcept
    {
        return value[0];
    }

    constexpr color8_t green() const noexcept
    {
        return value[1];
    }

    constexpr color8_t& green() noexcept
    {
        return value[1];
    }

    constexpr color8_t blue() const noexcept
    {
        return value[2];
    }

    constexpr color8_t& blue() noexcept
    {
        return value[2];
    }

    constexpr color8_t alpha() const noexcept
    {
        return value[3];
    }

    constexpr color8_t& alpha() noexcept
    {
        return value[3];
    }

private:
    RGBA_Format value{};
};

constexpr std::optional<ColorRGBA> RGBAFromString(std::string_view rgba_str)
{
    if(rgba_str.starts_with('#'))
    {
        auto rgba_sans_hash = rgba_str.substr(1, 8);

        switch(rgba_sans_hash.length())
        {
            case 8: // #RRGGBBAA
            {
                if(detail::isValidHexSeq(rgba_sans_hash))
                {
                    auto RGBA_AsArray = detail::ToUpperCase<8>(rgba_sans_hash);

                    return ColorRGBA{RGBA_AsArray};
                }

                return std::nullopt;
            }
            case 4: // #RGBA
            {
                if(detail::isValidHexSeq(rgba_sans_hash))
                {
                    auto RGBA_AsArray = detail::ToUpperCase<4>(rgba_sans_hash);
                    
                    return ColorRGBA{RGBA_AsArray};
                }

                return std::nullopt;
            }
            default:
                return std::nullopt;
        }
    }
}

namespace
{

// Default Colors: HTML5 color names; RGBA 100% Opaqueness
inline constexpr ColorRGBA AliceBlue            = {0xF0F8FFFF};
inline constexpr ColorRGBA AntiqueWhite         = {0xFAEBD7FF};
inline constexpr ColorRGBA Aqua                 = {0x00FFFFFF};
inline constexpr ColorRGBA Aquamarine           = {0x7FFFD4FF};
inline constexpr ColorRGBA Azure                = {0xF0FFFFFF};
inline constexpr ColorRGBA Beige                = {0xF5F5DCFF};
inline constexpr ColorRGBA Bisque               = {0xFFE4C4FF};
inline constexpr ColorRGBA Black                = {0x000000FF};
inline constexpr ColorRGBA BlancedAlmond        = {0xFFEBCDFF};
inline constexpr ColorRGBA Blue                 = {0x0000FFFF};
inline constexpr ColorRGBA BlueViolet           = {0x8A2BE2FF};
inline constexpr ColorRGBA Brown                = {0xA52A2AFF};
inline constexpr ColorRGBA BurlyWood            = {0xDE8887FF};
inline constexpr ColorRGBA CadetBlue            = {0x5F9EA0FF};
inline constexpr ColorRGBA Chartreuse           = {0x7FFF00FF};
inline constexpr ColorRGBA Chocolate            = {0xD2691EFF};
inline constexpr ColorRGBA Coral                = {0xFF7F50FF};
inline constexpr ColorRGBA CornflowerBlue       = {0x6495EDFF};
inline constexpr ColorRGBA Cornsilk             = {0xFFF8DCFF};
inline constexpr ColorRGBA Crimson              = {0xDC143CFF};
inline constexpr ColorRGBA Cyan                 = Aqua;
inline constexpr ColorRGBA DarkBlue             = {0x00008BFF};
inline constexpr ColorRGBA DarkCyan             = {0x008B8BFF};
inline constexpr ColorRGBA DarkGoldenRod        = {0xB8860BFF};
inline constexpr ColorRGBA DarkGray             = {0xA9A9A9FF};
inline constexpr ColorRGBA DarkGrey             = DarkGray;
inline constexpr ColorRGBA DarkGreen            = {0x006400FF};
inline constexpr ColorRGBA DarkKhaki            = {0xBDB76BFF};
inline constexpr ColorRGBA DarkMagenta          = {0x8B008BFF};
inline constexpr ColorRGBA DarkOliveGreen       = {0x556B2FFF};
inline constexpr ColorRGBA DarkOrange           = {0xFF8C00FF};
inline constexpr ColorRGBA DarkOrchid           = {0x9932CCFF};
inline constexpr ColorRGBA DarkRed              = {0x8B0000FF};
inline constexpr ColorRGBA DarkSalmon           = {0xE9967AFF};
inline constexpr ColorRGBA DarkSeaGreen         = {0x8FBC8FFF};
inline constexpr ColorRGBA DarkSlateBlue        = {0x483D8BFF};
inline constexpr ColorRGBA DarkSlateGray        = {0x2F4F4FFF};
inline constexpr ColorRGBA DarkSlateGrey        = DarkSlateGray;
inline constexpr ColorRGBA DarkTurquoise        = {0x00CED1FF};
inline constexpr ColorRGBA DarkViolet           = {0x9400D3FF};
inline constexpr ColorRGBA DeepPink             = {0xFF1493FF};
inline constexpr ColorRGBA DeepSkyBlue          = {0x00BFFFFF};
inline constexpr ColorRGBA DimGray              = {0x696969FF};
inline constexpr ColorRGBA DimGrey              = DimGray;
inline constexpr ColorRGBA DodgerBlue           = {0x1E90FFFF};
inline constexpr ColorRGBA FireBrick            = {0xB22222FF};
inline constexpr ColorRGBA FloralWhite          = {0xFFFAF0FF};
inline constexpr ColorRGBA ForestGreen          = {0x228B22FF};
inline constexpr ColorRGBA Fuschsia             = {0xFF00FFFF};
inline constexpr ColorRGBA Gainsboro            = {0xDCDCDCFF};
inline constexpr ColorRGBA GhostWhite           = {0xF8F8FFFF};
inline constexpr ColorRGBA Gold                 = {0xFFD700FF};
inline constexpr ColorRGBA GoldenRod            = {0xDAA520FF};
inline constexpr ColorRGBA Gray                 = {0x808080FF};
inline constexpr ColorRGBA Grey                 = Gray;
inline constexpr ColorRGBA Green                = {0x00FF00FF};
inline constexpr ColorRGBA GreenYellow          = {0xADFF2FFF};
inline constexpr ColorRGBA HoneyDew             = {0xF0FFF0FF};
inline constexpr ColorRGBA HotPink              = {0xFF69B4FF};
inline constexpr ColorRGBA IndianRed            = {0xCD5C5CFF};
inline constexpr ColorRGBA Indigo               = {0x4B0082FF};
inline constexpr ColorRGBA Ivory                = {0xFFFFF0FF};
inline constexpr ColorRGBA Khaki                = {0xF0E68CFF};
inline constexpr ColorRGBA Lavendar             = {0xE6E6FAFF};
inline constexpr ColorRGBA LavenderBlush        = {0xFFF0F5FF};
inline constexpr ColorRGBA LawnGreen            = {0x7CFC00FF};
inline constexpr ColorRGBA LemonChiffon         = {0xFFFACDFF};
inline constexpr ColorRGBA LightBlue            = {0xADD8E6FF};
inline constexpr ColorRGBA LightCoral           = {0xF08080FF};
inline constexpr ColorRGBA LightCyan            = {0xE0FFFFFF};
inline constexpr ColorRGBA LightGoldenRodYellow = {0xFAFAD2FF};
inline constexpr ColorRGBA LightGray            = {0xD3D3D3FF};
inline constexpr ColorRGBA LightGrey            = LightGray;
inline constexpr ColorRGBA LightGreen           = {0x90EE90FF};
inline constexpr ColorRGBA LightPink            = {0xFFB6C1FF};
inline constexpr ColorRGBA LightSalmon          = {0xFFA07AFF};
inline constexpr ColorRGBA LightSeaGreen        = {0x20B2AAFF};
inline constexpr ColorRGBA LightSkyBlue         = {0x87CEFAFF};
inline constexpr ColorRGBA LightSlateGray       = {0x778899FF};
inline constexpr ColorRGBA LightSlateGrey       = LightSlateGray;
inline constexpr ColorRGBA LightSteelBlue       = {0xB0C4DEFF};
inline constexpr ColorRGBA LightYellow          = {0xFFFFE0FF};
inline constexpr ColorRGBA Lime                 = {0x00FF00FF};
inline constexpr ColorRGBA LimeGreen            = {0x32CD32FF};
inline constexpr ColorRGBA Linen                = {0xFAF0E6FF};
inline constexpr ColorRGBA Magenta              = {0xFF00FFFF};
inline constexpr ColorRGBA Maroon               = {0x800000FF};
inline constexpr ColorRGBA MediumAquaMarine     = {0x66CDAAFF};
inline constexpr ColorRGBA MediumBlue           = {0x0000CDFF};
inline constexpr ColorRGBA MediumOrchid         = {0xBA55D3FF};
inline constexpr ColorRGBA MediumPurple         = {0x9370DBFF};
inline constexpr ColorRGBA MediumSeaGreen       = {0x3CB371FF};
inline constexpr ColorRGBA MediumSlateBlue      = {0x7B68EEFF};
inline constexpr ColorRGBA MediumSpringGreen    = {0x00FA9AFF};
inline constexpr ColorRGBA MediumTurquoise      = {0x48D1CCFF};
inline constexpr ColorRGBA MediumVioletRed      = {0xC71585FF};
inline constexpr ColorRGBA MidnightBlue         = {0x191970FF};
inline constexpr ColorRGBA MintCream            = {0xF5FFFAFF};
inline constexpr ColorRGBA MistyRose            = {0xFFE4E1FF};
inline constexpr ColorRGBA Moccasin             = {0xFFE4B5FF};
inline constexpr ColorRGBA NavajoWhite          = {0xFFDEADFF};
inline constexpr ColorRGBA Navy                 = {0x000080FF};
inline constexpr ColorRGBA OldLace              = {0xFDF5E6FF};
inline constexpr ColorRGBA Olive                = {0x808000FF};
inline constexpr ColorRGBA OliveDrab            = {0x6B8E23FF};
inline constexpr ColorRGBA Orange               = {0xFFA500FF};
inline constexpr ColorRGBA OrangeRed            = {0xFF4500FF};
inline constexpr ColorRGBA Orchid               = {0xDA70D6FF};
inline constexpr ColorRGBA PaleGoldenRod        = {0xEEE8AAFF};
inline constexpr ColorRGBA PaleGreen            = {0x98FB98FF};
inline constexpr ColorRGBA PaleTurquoise        = {0xAFEEEEFF};
inline constexpr ColorRGBA PaleVioletRed        = {0xDB7093FF};
inline constexpr ColorRGBA PapayaWhip           = {0xFFEFD5FF};
inline constexpr ColorRGBA PeachPuff            = {0xFFDAB9FF};
inline constexpr ColorRGBA Peru                 = {0xCD853FFF};
inline constexpr ColorRGBA Pink                 = {0xFFC0CBFF};
inline constexpr ColorRGBA Plum                 = {0xDDA0DDFF};
inline constexpr ColorRGBA PowderBlue           = {0xB0E0E6FF};
inline constexpr ColorRGBA Purple               = {0x800080FF};
inline constexpr ColorRGBA Red                  = {0xFF0000FF};
inline constexpr ColorRGBA RosyBrown            = {0xBC8F8FFF};
inline constexpr ColorRGBA RoyalBlue            = {0x4169E1FF};
inline constexpr ColorRGBA SaddleBrown          = {0x8B4513FF};
inline constexpr ColorRGBA Salmon               = {0xFA8072FF};
inline constexpr ColorRGBA SandyBrown           = {0xF4A460FF};
inline constexpr ColorRGBA SeaGreen             = {0x2E8B57FF};
inline constexpr ColorRGBA SeaShell             = {0xFFF5EEFF};
inline constexpr ColorRGBA Sienna               = {0xA0522DFF};
inline constexpr ColorRGBA Silver               = {0xC0C0C0FF};
inline constexpr ColorRGBA SkyBlue              = {0x87CEEBFF};
inline constexpr ColorRGBA SlateBlue            = {0x6A5ACDFF};
inline constexpr ColorRGBA SlateGray            = {0x708090FF};
inline constexpr ColorRGBA SlateGrey            = SlateGray;
inline constexpr ColorRGBA Snow                 = {0xFFFAFAFF};
inline constexpr ColorRGBA SpringGreen          = {0x00FF7FFF};
inline constexpr ColorRGBA SteelBlue            = {0x4682B4FF};
inline constexpr ColorRGBA Tan                  = {0xD2B48CFF};
inline constexpr ColorRGBA Teal                 = {0x008080FF};
inline constexpr ColorRGBA Thistle              = {0xD8BFD8FF};
inline constexpr ColorRGBA Tomato               = {0xFF6347FF};
inline constexpr ColorRGBA Turquoise            = {0x40E0D0FF};
inline constexpr ColorRGBA Violet               = {0xEE82EEFF};
inline constexpr ColorRGBA Wheat                = {0xF5DEB3FF};
inline constexpr ColorRGBA White                = {0xFFFFFFFF};
inline constexpr ColorRGBA WhiteSmoke           = {0xF5F5F5FF};
inline constexpr ColorRGBA Yellow               = {0xFFFF00FF};
inline constexpr ColorRGBA YellowGreen          = {0x9ACD32FF};

struct NameColor
{
    std::string_view name;
    ColorRGBA color;
};

inline constexpr std::array<NameColor, 147> StandardColorsLUT
{{
    { "AliceBlue", AliceBlue },
    { "AntiqueWhite", AntiqueWhite },
    { "Aqua", Aqua },
    { "Aquamarine", Aquamarine },
    { "Azure", Azure },
    { "Beige", Beige },
    { "Bisque", Bisque },
    { "Black", Black },
    { "BlancedAlmond", BlancedAlmond },
    { "Blue", Blue },
    { "BlueViolet", BlueViolet },
    { "Brown", Brown },
    { "BurlyWood", BurlyWood },
    { "CadetBlue", CadetBlue },
    { "Chartreuse", Chartreuse },
    { "Chocolate", Chocolate },
    { "Coral", Coral },
    { "CornflowerBlue", CornflowerBlue },
    { "Cornsilk", Cornsilk },
    { "Crimson", Crimson },
    { "Cyan", Cyan },
    { "DarkBlue", DarkBlue },
    { "DarkCyan", DarkCyan },
    { "DarkGoldenRod", DarkGoldenRod },
    { "DarkGray", DarkGray },
    { "DarkGrey", DarkGrey },
    { "DarkGreen", DarkGreen },
    { "DarkKhaki", DarkKhaki },
    { "DarkMagenta", DarkMagenta },
    { "DarkOliveGreen", DarkOliveGreen },
    { "DarkOrange", DarkOrange },
    { "DarkOrchid", DarkOrchid },
    { "DarkRed", DarkRed },
    { "DarkSalmon", DarkSalmon },
    { "DarkSeaGreen", DarkSeaGreen },
    { "DarkSlateBlue", DarkSlateBlue },
    { "DarkSlateGray", DarkSlateGray },
    { "DarkSlateGrey", DarkSlateGrey },
    { "DarkTurquoise", DarkTurquoise },
    { "DarkViolet", DarkViolet },
    { "DeepPink", DeepPink },
    { "DeepSkyBlue", DeepSkyBlue },
    { "DimGray", DimGray },
    { "DimGrey", DimGrey },
    { "DodgerBlue", DodgerBlue },
    { "FireBrick", FireBrick },
    { "FloralWhite", FloralWhite },
    { "ForestGreen", ForestGreen },
    { "Fuschsia", Fuschsia },
    { "Gainsboro", Gainsboro },
    { "GhostWhite", GhostWhite },
    { "Gold", Gold },
    { "GoldenRod", GoldenRod },
    { "Gray", Gray },
    { "Grey", Grey },
    { "Green", Green },
    { "GreenYellow", GreenYellow },
    { "HoneyDew", HoneyDew },
    { "HotPink", HotPink },
    { "IndianRed", IndianRed },
    { "Indigo", Indigo },
    { "Ivory", Ivory },
    { "Khaki", Khaki },
    { "Lavendar", Lavendar },
    { "LavenderBlush", LavenderBlush },
    { "LawnGreen", LawnGreen },
    { "LemonChiffon", LemonChiffon },
    { "LightBlue", LightBlue },
    { "LightCoral", LightCoral },
    { "LightCyan", LightCyan },
    { "LightGoldenRodYellow", LightGoldenRodYellow },
    { "LightGray", LightGray },
    { "LightGrey", LightGrey },
    { "LightGreen", LightGreen },
    { "LightPink", LightPink },
    { "LightSalmon", LightSalmon },
    { "LightSeaGreen", LightSeaGreen },
    { "LightSkyBlue", LightSkyBlue },
    { "LightSlateGray", LightSlateGray },
    { "LightSlateGrey", LightSlateGrey },
    { "LightSteelBlue", LightSteelBlue },
    { "LightYellow", LightYellow },
    { "Lime", Lime },
    { "LimeGreen", LimeGreen },
    { "Linen", Linen },
    { "Magenta", Magenta },
    { "Maroon", Maroon },
    { "MediumAquaMarine", MediumAquaMarine },
    { "MediumBlue", MediumBlue },
    { "MediumOrchid", MediumOrchid },
    { "MediumPurple", MediumPurple },
    { "MediumSeaGreen", MediumSeaGreen },
    { "MediumSlateBlue", MediumSlateBlue },
    { "MediumSpringGreen", MediumSpringGreen },
    { "MediumTurquoise", MediumTurquoise },
    { "MediumVioletRed", MediumVioletRed },
    { "MidnightBlue", MidnightBlue },
    { "MintCream", MintCream },
    { "MistyRose", MistyRose },
    { "Moccasin", Moccasin },
    { "NavajoWhite", NavajoWhite },
    { "Navy", Navy },
    { "OldLace", OldLace },
    { "Olive", Olive },
    { "OliveDrab", OliveDrab },
    { "Orange", Orange },
    { "OrangeRed", OrangeRed },
    { "Orchid", Orchid },
    { "PaleGoldenRod", PaleGoldenRod },
    { "PaleGreen", PaleGreen },
    { "PaleTurquoise", PaleTurquoise },
    { "PaleVioletRed", PaleVioletRed },
    { "PapayaWhip", PapayaWhip },
    { "PeachPuff", PeachPuff },
    { "Peru", Peru },
    { "Pink", Pink },
    { "Plum", Plum },
    { "PowderBlue", PowderBlue },
    { "Purple", Purple },
    { "Red", Red },
    { "RosyBrown", RosyBrown },
    { "RoyalBlue", RoyalBlue },
    { "SaddleBrown", SaddleBrown },
    { "Salmon", Salmon },
    { "SandyBrown", SandyBrown },
    { "SeaGreen", SeaGreen },
    { "SeaShell", SeaShell },
    { "Sienna", Sienna },
    { "Silver", Silver },
    { "SkyBlue", SkyBlue },
    { "SlateBlue", SlateBlue },
    { "SlateGray", SlateGray },
    { "SlateGrey", SlateGrey },
    { "Snow", Snow },
    { "SpringGreen", SpringGreen },
    { "SteelBlue", SteelBlue },
    { "Tan", Tan },
    { "Teal", Teal },
    { "Thistle", Thistle },
    { "Tomato", Tomato },
    { "Turquoise", Turquoise },
    { "Violet", Violet },
    { "Wheat", Wheat },
    { "White", White },
    { "WhiteSmoke", WhiteSmoke },
    { "Yellow", Yellow },
    { "YellowGreen", YellowGreen }
}};

constexpr std::optional<ColorRGBA> GetColorByName(std::string_view name)
{
    auto possibleColorMatch = std::find_if(StandardColorsLUT.begin(), StandardColorsLUT.end(),
        [name](auto namecolor)
        {
            return name == namecolor.name;
        });

    if(possibleColorMatch != StandardColorsLUT.end())
    {
        return std::make_optional(possibleColorMatch->color);
    }

    return std::nullopt;
}

constexpr std::string_view GetNameByColor(const ColorRGBA& rgba)
{
    auto possibleNameMatch = std::find_if(StandardColorsLUT.begin(), StandardColorsLUT.end(),
        [rgba](auto namecolor)
        {
            return rgba == namecolor.color;
        });

    if(possibleNameMatch != StandardColorsLUT.end())
    {
        return possibleNameMatch->name;
    }

    return {};
}

} // namespace anonymous

template<typename T> // Offset +1 for prec.
constexpr T sqrt_approx(T value, unsigned int prec = 8)
{
    if(prec > 1)
    {
        --prec;
        return (sqrt_approx(value, prec) + value / sqrt_approx(value, prec)) / 2;
    }
    else
    {
        return (1 + value) / 2;
    }
}

constexpr float LinearToRGB(float lincolor)
{
    if(lincolor <= 0.0031308F)
    {
        return 12.92 * lincolor;
    }
    else
    {
        return 0.585122381F * sqrt_approx(lincolor) +
               0.783140355F * sqrt_approx(sqrt_approx(lincolor)) +
               0.368262736F * sqrt_approx(sqrt_approx(sqrt_approx(lincolor)));
    }
}

constexpr float RGBToLinear(float rgbcolor)
{
    if(rgbcolor < 0.04045F)
    {
        return rgbcolor / 12.92;
    }
    else
    {
        return 0.012522878F * rgbcolor +
               0.682171111F * rgbcolor * rgbcolor +
               0.305306011F * rgbcolor * rgbcolor * rgbcolor;
    }
}

} // namespace wx::color

template <>
struct fmt::formatter<wx::color::ColorRGBA>
{
    // Presentation format: 'f' - fixed, 'e' - exponential.
    std::string presentation{"RGBAName"};

    static constexpr std::string_view HtmlHexFormat  = "#{0:X}{0:X}{0:X}{0:X}";
    static constexpr std::string_view HtmlRGBFormat  = "rgb({}, {}, {})";
    static constexpr std::string_view HtmlRGBAFormat = "rgba({}, {}, {}, {})";
    static constexpr std::string_view CSSFormat      = HtmlRGBFormat;
    static constexpr std::string_view RGBAName       = "{}";

    // Parses format specifications of the forms [CSS | HtmlRGB | HtmlHex | RGBAName].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        // [ctx.begin(), ctx.end()) is a character range that contains a part of
        // the format string starting from the format specifications to be parsed,
        // e.g. in
        //
        //   fmt::format("{:RGBAName} - color name", RGBAColor(0, 255, 0, 0));
        //
        // the range will contain "RGBAName} - point of interest". The formatter should
        // parse specifiers until '}' or the end of the range. In this example
        // the formatter should parse the 'f' specifier and return an iterator
        // pointing to '}'.

        // Parse the presentation format and store it in the formatter:
        auto it  = ctx.begin();
        auto end = ctx.end();

        std::string_view fullCtx{ctx.begin(), ctx.end()};

        if(it != end)
        {
            if(fullCtx.starts_with("CSS"))
            {
                presentation = "CSS";
                std::advance(it, 3);
            }
            else if(fullCtx.starts_with("HtmlRGB"))
            {
                presentation = "HtmlRGB";
                std::advance(it, 7);
            }
            else if(fullCtx.starts_with("HtmlHex"))
            {
                presentation = "HtmlHex";
                std::advance(it, 7);
            }
            else if(fullCtx.starts_with("RGBAName"))
            {
                presentation = "RGBAName";
                std::advance(it, 8);
            }
        }

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw fmt::format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const wx::color::ColorRGBA& rgba, FormatContext& ctx) -> decltype(ctx.out())
    {
        std::string_view presentation_fmt{};

        switch(presentation)
        {
            case "HtmlHexFormat":
                presentation_fmt = HtmlHexFormat;
                break;
            case "HtmlRGBFormat":
                presentation_fmt = HtmlRGBFormat;
                break;
            case "HtmlRGBAFormat":
                presentation_fmt = HtmlRGBAFormat;
                break;
            case "CSS":
                presentation_fmt = CSSFormat;
                break;
            case "name":
                return format_to(
                    ctx.out(),
                    "{}",
                    wx::color::GetNameByColor(rgba)
                );
            default:
                presentation_fmt = HtmlRGBAFormat; // TODO: Is this right?
                break;                
        }

        // ctx.out() is an output iterator to write to.
        return format_to(
            ctx.out(),
            presentation_fmt,
            rgba.red(), rgba.green(), rgba.blue(), rgba.alpha());
    }
};

//-----------------------------------------------------------------------------
// wxColourBase: this class has no data members, just some functions to avoid
//               code redundancy in all native wxColour implementations
//-----------------------------------------------------------------------------

/*  Transition from wxGDIObject to wxObject is incomplete.  If your port does
    not need the wxGDIObject machinery to handle colors, please add it to the
    list of ports which do not need it.
 */
#if defined( __WXMSW__ ) || defined( __WXQT__ )
#define wxCOLOUR_IS_GDIOBJECT 0
#else
#define wxCOLOUR_IS_GDIOBJECT 1
#endif

class wxColourBase : public
#if wxCOLOUR_IS_GDIOBJECT
    wxGDIObject
#else
    wxObject
#endif
{
public:
    // type of a single colour component
    using ChannelType = unsigned char;

    ~wxColourBase() = default;

    // Set() functions
    // ---------------

    void Set(ChannelType red,
             ChannelType green,
             ChannelType blue,
             ChannelType alpha = wxALPHA_OPAQUE)
        { InitRGBA(red, green, blue, alpha); }

    // implemented in colourcmn.cpp
    bool Set(const wxString &str)
        { return FromString(str); }

    void Set(unsigned long colRGB)
    {
        // we don't need to know sizeof(long) here because we assume that the three
        // least significant bytes contain the R, G and B values
        Set((ChannelType)(0xFF & colRGB),
            (ChannelType)(0xFF & (colRGB >> 8)),
            (ChannelType)(0xFF & (colRGB >> 16)));
    }

    virtual ChannelType Red() const = 0;
    virtual ChannelType Green() const = 0;
    virtual ChannelType Blue() const = 0;
    virtual ChannelType Alpha() const
        { return wxALPHA_OPAQUE ; }

    virtual bool IsSolid() const
        { return true; }

    // implemented in colourcmn.cpp
    virtual wxString GetAsString(unsigned int flags = wxC2S_NAME | wxC2S_CSS_SYNTAX) const;

    void SetRGB(std::uint32_t colRGB)
    {
        Set((ChannelType)(0xFFU & colRGB),
            (ChannelType)(0xFFU & (colRGB >> 8)),
            (ChannelType)(0xFFU & (colRGB >> 16)));
    }

    void SetRGBA(std::uint32_t colRGBA)
    {
        Set((ChannelType)(0xFFU & colRGBA),
            (ChannelType)(0xFFU & (colRGBA >> 8)),
            (ChannelType)(0xFFU & (colRGBA >> 16)),
            (ChannelType)(0xFFU & (colRGBA >> 24)));
    }

    std::uint32_t GetRGB() const
        { return Red() | (Green() << 8) | (Blue() << 16); }

    std::uint32_t GetRGBA() const
        { return Red() | (Green() << 8) | (Blue() << 16) | (Alpha() << 24); }

#if !wxCOLOUR_IS_GDIOBJECT
    virtual bool IsOk() const= 0;
#endif

    // Return the perceived brightness of the colour, with 0 for black and 1
    // for white.
    double GetLuminance() const;

    // manipulation
    // ------------

    // These methods are static because they are mostly used
    // within tight loops (where we don't want to instantiate wxColour's)

    static void          MakeMono    (unsigned char* r, unsigned char* g, unsigned char* b, bool on);
    static void          MakeDisabled(unsigned char* r, unsigned char* g, unsigned char* b, unsigned char brightness = 255);
    static void          MakeGrey    (unsigned char* r, unsigned char* g, unsigned char* b); // integer version
    static void          MakeGrey    (unsigned char* r, unsigned char* g, unsigned char* b,
                                      double weight_r, double weight_g, double weight_b); // floating point version
    static unsigned char AlphaBlend  (unsigned char fg, unsigned char bg, double alpha);
    static void          ChangeLightness(unsigned char* r, unsigned char* g, unsigned char* b, int ialpha);

    wxColour ChangeLightness(int ialpha) const;
    wxColour& MakeDisabled(unsigned char brightness = 0xFFU);

protected:
    // Some ports need Init() and while we don't, provide a stub so that the
    // ports which don't need it are not forced to define it
    void Init() { }

    virtual void
    InitRGBA(ChannelType r, ChannelType g, ChannelType b, ChannelType a) = 0;

    virtual bool FromString(const wxString& s);

#if wxCOLOUR_IS_GDIOBJECT
    // wxColour doesn't use reference counted data (at least not in all ports)
    // so provide stubs for the functions which need to be defined if we do use
    // them
    wxGDIRefData *CreateGDIRefData() const override
    {
        wxFAIL_MSG( "must be overridden if used" );

        return NULL;
    }

    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *WXUNUSED(data)) const override
    {
        wxFAIL_MSG( "must be overridden if used" );

        return NULL;
    }
#endif
};


// wxColour <-> wxString utilities, used by wxConfig, defined in colourcmn.cpp
wxString wxToString(const wxColourBase& col);
bool wxFromString(const wxString& str, wxColourBase* col);



#if defined(__WXMSW__)
    #include "wx/msw/colour.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/colour.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/colour.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/colour.h"
#elif defined(__WXDFB__)
    #include "wx/generic/colour.h"
#elif defined(__WXX11__)
    #include "wx/x11/colour.h"
#elif defined(__WXMAC__)
    #include "wx/osx/colour.h"
#elif defined(__WXQT__)
    #include "wx/qt/colour.h"
#endif

#define wxColor wxColour

#endif // _WX_COLOUR_H_BASE_
