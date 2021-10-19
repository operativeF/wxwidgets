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
#include "wx/string.h"
#include "wx/stringutils.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <span>

class WXDLLIMPEXP_FWD_CORE wxColour;

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

constexpr unsigned char wxALPHA_TRANSPARENT = 0;
constexpr unsigned char wxALPHA_OPAQUE = 0xff;

// a valid but fully transparent colour
#define wxTransparentColour wxColour(0, 0, 0, wxALPHA_TRANSPARENT)
#define wxTransparentColor wxTransparentColour

// ----------------------------------------------------------------------------
// wxVariant support
// ----------------------------------------------------------------------------

#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT_EXPORTED(wxColour,WXDLLIMPEXP_CORE)
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
    constexpr std::array<char, HexLength> ToUpperCase(std::string_view str)
    {
        std::array<char, HexLength> strarr{};
        std::transform(str.begin(), str.end(), strarr.begin(), wx::utils::ToUpperCh);

        return strarr;
    }

    constexpr color8_t XXAsByte(std::span<char> hexValue)
    {
        return color8_t{static_cast<unsigned int>(hexValue[0] << 4) | static_cast<unsigned int>(hexValue[1])};
    }

} // namespace detail

struct ColorRGBA
{
    constexpr ColorRGBA() noexcept {}

    constexpr ColorRGBA(color8_t red, color8_t green, color8_t blue, color8_t alpha = color8_t{wxALPHA_OPAQUE}) noexcept
        : r{red},
          g{green},
          b{blue},
          a{alpha}
    {
    }

    constexpr ColorRGBA(std::uint32_t rgb) noexcept
        : r{static_cast<color8_t>(rgb & 0xFF)},
          g{static_cast<color8_t>((rgb >> 8) & 0xFF)},
          b{static_cast<color8_t>((rgb >> 16) & 0xFF)},
          a{static_cast<color8_t>((rgb >> 24) & 0xFF)}
    {
    }

    constexpr std::uint32_t ToInt() noexcept
    {
        return (static_cast<std::uint32_t>(r)        |
               (static_cast<std::uint32_t>(g) << 8)  |
               (static_cast<std::uint32_t>(b) << 16) |
               (static_cast<std::uint32_t>(a) << 24));
    }

    color8_t r{};
    color8_t g{};
    color8_t b{};
    color8_t a{};
};

// FIXME: WIP
// constexpr std::optional<ColorRGBA> FromString(std::string_view rgba_str)
// {
//     if(rgba_str.starts_with('#'))
//     {
//         auto rgba_sans_hash = rgba_str.substr(1, 8);

//         switch(rgba_sans_hash.length())
//         {
//             case 8: // #RRGGBBAA
//                 if(detail::isValidHexSeq(rgba_sans_hash))
//                 {
//                     auto RGBA_AsArray = detail::ToUpperCase<8>(rgba_sans_hash);

//                     auto RGBA_Span = std::span{RGBA_AsArray};

//                     return ColorRGBA{detail::XXAsByte(RGBA_Span.subspan(0, 2)),
//                                      detail::XXAsByte(RGBA_Span.subspan(2, 2)),
//                                      detail::XXAsByte(RGBA_Span.subspan(4, 2)),
//                                      detail::XXAsByte(RGBA_Span.subspan(6, 2))
//                                     };
//                 }
//                 return std::nullopt;

//             default:
//                 return std::nullopt;
//         }
//     }
// }



} // namespace wx::color

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

class WXDLLIMPEXP_CORE wxColourBase : public
#if wxCOLOUR_IS_GDIOBJECT
    wxGDIObject
#else
    wxObject
#endif
{
public:
    // type of a single colour component
    using ChannelType = unsigned char;

    wxColourBase() = default;
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
WXDLLIMPEXP_CORE wxString wxToString(const wxColourBase& col);
WXDLLIMPEXP_CORE bool wxFromString(const wxString& str, wxColourBase* col);



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
