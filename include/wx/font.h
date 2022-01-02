/////////////////////////////////////////////////////////////////////////////
// Name:        wx/font.h
// Purpose:     wxFontBase class: the interface of wxFont
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.09.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONT_H_BASE_
#define _WX_FONT_H_BASE_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/gdiobj.h"      // the base class
#include "wx/gdicmn.h"      // for wxGDIObjListBase

import WX.Base.FontEnc;

// ----------------------------------------------------------------------------
// forward declarations
// ----------------------------------------------------------------------------

class wxFont;

// ----------------------------------------------------------------------------
// font constants
// ----------------------------------------------------------------------------

// standard font families: these may be used only for the font creation, it
// doesn't make sense to query an existing font for its font family as,
// especially if the font had been created from a native font description, it
// may be unknown
enum class wxFontFamily
{
    Default,
    Decorative,
    Roman,
    Script,
    Swiss,
    Modern,
    Teletype,
    Max,
    Unknown = Max
};

// font styles
enum class wxFontStyle
{
    Normal,
    Italic,
    Slant,
    Max
};

// font weights
enum wxFontWeight
{
    wxFONTWEIGHT_INVALID = 0,
    wxFONTWEIGHT_THIN = 100,
    wxFONTWEIGHT_EXTRALIGHT = 200,
    wxFONTWEIGHT_LIGHT = 300,
    wxFONTWEIGHT_NORMAL = 400,
    wxFONTWEIGHT_MEDIUM = 500,
    wxFONTWEIGHT_SEMIBOLD = 600,
    wxFONTWEIGHT_BOLD = 700,
    wxFONTWEIGHT_EXTRABOLD = 800,
    wxFONTWEIGHT_HEAVY = 900,
    wxFONTWEIGHT_EXTRAHEAVY = 1000,
    wxFONTWEIGHT_MAX = wxFONTWEIGHT_EXTRAHEAVY
};

// Symbolic font sizes as defined in CSS specification.
enum wxFontSymbolicSize
{
    wxFONTSIZE_XX_SMALL = -3,
    wxFONTSIZE_X_SMALL,
    wxFONTSIZE_SMALL,
    wxFONTSIZE_MEDIUM,
    wxFONTSIZE_LARGE,
    wxFONTSIZE_X_LARGE,
    wxFONTSIZE_XX_LARGE
};

// the font flag bits for the new font ctor accepting one combined flags word
enum class wxFontFlags
{
    Default,
    Italic,
    Slant,
    Light,
    Bold,
    Antialiased,
    NotAntialiased,
    Underlined,
    Strikethrough,
    _max_size
};

using FontFlags = InclBitfield<wxFontFlags>;

// ----------------------------------------------------------------------------
// wxFontInfo describes a wxFont
// ----------------------------------------------------------------------------

class wxFontInfo
{
public:
    // Default ctor uses the default font size appropriate for the current
    // platform.
    wxFontInfo()
    {    
        m_family = wxFontFamily::Default;
        m_weight = wxFONTWEIGHT_NORMAL;
        m_encoding = wxFONTENCODING_DEFAULT;
    
    }

    // These ctors specify the font size, either in points or in pixels.
    explicit wxFontInfo(double pointSize)
        : m_pointSize(pointSize >= 0.0 ? pointSize : -1.0)
    {
        
        m_family = wxFontFamily::Default;
        m_weight = wxFONTWEIGHT_NORMAL;
        m_encoding = wxFONTENCODING_DEFAULT;

        // FIXME: Double equality
        if (!(m_pointSize == pointSize))
        {
            wxFAIL_MSG("Invalid font point size");
        }
    }
    explicit wxFontInfo(const wxSize& pixelSize)
        : m_pointSize(-1.0)
        , m_pixelSize(pixelSize)
    {
        
        m_family = wxFontFamily::Default;
        m_weight = wxFONTWEIGHT_NORMAL;
        m_encoding = wxFONTENCODING_DEFAULT;
    
    }
    // Default copy ctor, assignment operator and dtor are OK

    // Setters for the various attributes. All of them return the object itself
    // so that the calls to them could be chained.
    wxFontInfo& Family(wxFontFamily family)
        { m_family = family; return *this; }
    wxFontInfo& FaceName(const std::string& faceName)
        { m_faceName = faceName; return *this; }

    wxFontInfo& Weight(int weight)
        { m_weight = weight; return *this; }
    wxFontInfo& Bold(bool bold = true)
        { return Weight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL); }
    wxFontInfo& Light(bool light = true)
        { return Weight(light ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL); }

    wxFontInfo& Italic(bool italic = true)
        { SetFlag(wxFontFlags::Italic, italic); return *this; }
    wxFontInfo& Slant(bool slant = true)
        { SetFlag(wxFontFlags::Slant, slant); return *this; }
    wxFontInfo& Style(wxFontStyle style)
    {
        if ( style == wxFontStyle::Italic )
            return Italic();

        if ( style == wxFontStyle::Slant )
            return Slant();

        return *this;
    }

    wxFontInfo& AntiAliased(bool antiAliased = true)
        { SetFlag(wxFontFlags::Antialiased, antiAliased); return *this; }
    wxFontInfo& Underlined(bool underlined = true)
        { SetFlag(wxFontFlags::Underlined, underlined); return *this; }
    wxFontInfo& Strikethrough(bool strikethrough = true)
        { SetFlag(wxFontFlags::Strikethrough, strikethrough); return *this; }

    wxFontInfo& Encoding(wxFontEncoding encoding)
        { m_encoding = encoding; return *this; }

    // Set all flags at once.
    wxFontInfo& AllFlags(FontFlags flags)
    {
        m_flags = flags;

        m_weight = m_flags & wxFontFlags::Bold
                        ? wxFONTWEIGHT_BOLD
                        : m_flags & wxFontFlags::Light
                            ? wxFONTWEIGHT_LIGHT
                            : wxFONTWEIGHT_NORMAL;

        return *this;
    }

    // Accessors are mostly meant to be used by wxFont itself to extract the
    // various pieces of the font description.

    bool IsUsingSizeInPixels() const { return m_pixelSize != wxDefaultSize; }
    double GetFractionalPointSize() const { return m_pointSize; }
    int GetPointSize() const { return std::lround(m_pointSize); }
    wxSize GetPixelSize() const { return m_pixelSize; }

    // If face name is not empty, it has priority, otherwise use family.
    bool HasFaceName() const { return !m_faceName.empty(); }
    wxFontFamily GetFamily() const { return m_family; }
    const std::string& GetFaceName() const { return m_faceName; }

    wxFontStyle GetStyle() const
    {
        return (m_flags & wxFontFlags::Italic)
                        ? wxFontStyle::Italic
                        : (m_flags & wxFontFlags::Slant)
                            ? wxFontStyle::Slant
                            : wxFontStyle::Normal;
    }

    int GetNumericWeight() const
    {
        return m_weight;
    }

    wxFontWeight GetWeight() const
    {
        return GetWeightClosestToNumericValue(m_weight);
    }

    bool IsAntiAliased() const
    {
        return m_flags.is_set(wxFontFlags::Antialiased);
    }

    bool IsUnderlined() const
    {
        return m_flags.is_set(wxFontFlags::Underlined);
    }

    bool IsStrikethrough() const
    {
        return m_flags.is_set(wxFontFlags::Strikethrough);
    }

    wxFontEncoding GetEncoding() const { return m_encoding; }

    // Another helper for converting arbitrary numeric weight to the closest
    // value of wxFontWeight enum. It should be avoided in the new code (also
    // note that the function for the conversion in the other direction is
    // trivial and so is not provided, we only have GetNumericWeightOf() which
    // contains backwards compatibility hacks, but we don't need it here).
    static wxFontWeight GetWeightClosestToNumericValue(int numWeight)
    {
        wxASSERT(numWeight > 0);
        wxASSERT(numWeight <= 1000);

        // round to nearest hundredth = wxFONTWEIGHT_ constant
        int weight = ((numWeight + 50) / 100) * 100;

        if (weight < wxFONTWEIGHT_THIN)
            weight = wxFONTWEIGHT_THIN;
        if (weight > wxFONTWEIGHT_MAX)
            weight = wxFONTWEIGHT_MAX;

        return static_cast<wxFontWeight>(weight);
    }

private:
    // Turn on or off the given bit in m_flags depending on the value of the
    // boolean argument.
    void SetFlag(wxFontFlags flag, bool on)
    {
        if ( on )
            m_flags.set(flag);
        else
            m_flags.reset(flag);
    }

    // The size information: if m_pixelSize is valid (!= wxDefaultSize), then
    // it is used. Otherwise m_pointSize is used, except if it is < 0, which
    // means that the platform dependent font size should be used instead.
    double m_pointSize{-1.0};
    wxSize m_pixelSize{wxDefaultSize};

    wxFontFamily m_family;
    std::string m_faceName;
    FontFlags m_flags{wxFontFlags::Default};
    int m_weight;
    wxFontEncoding m_encoding;
};

// ----------------------------------------------------------------------------
// wxFontBase represents a font object
// ----------------------------------------------------------------------------

class wxNativeFontInfo;

class wxFontBase : public wxGDIObject
{
public:
    /*
        derived classes should provide the following ctors:

    wxFont();
    wxFont(const wxFontInfo& info);
    wxFont(const std::string& nativeFontInfoString);
    wxFont(const wxNativeFontInfo& info);
    wxFont(int size,
           wxFontFamily family,
           wxFontStyle style,
           wxFontWeight weight,
           bool underlined = false,
           const std::string& face = {},
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT);
    wxFont(const wxSize& pixelSize,
           wxFontFamily family,
           wxFontStyle style,
           wxFontWeight weight,
           bool underlined = false,
           const std::string& face = {},
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT);
    */

    // from the font components
    static wxFont *New(
        int pointSize,              // size of the font in points
        wxFontFamily family,        // see wxFontFamily enum
        wxFontStyle style,          // see wxFontStyle enum
        wxFontWeight weight,        // see wxFontWeight enum
        bool underlined = false,    // not underlined by default
        const std::string& face = {},              // facename
        wxFontEncoding encoding = wxFONTENCODING_DEFAULT); // ISO8859-X, ...

    // from the font components
    static wxFont *New(
        const wxSize& pixelSize,    // size of the font in pixels
        wxFontFamily family,        // see wxFontFamily enum
        wxFontStyle style,          // see wxFontStyle enum
        wxFontWeight weight,        // see wxFontWeight enum
        bool underlined = false,    // not underlined by default
        const std::string& face = {},              // facename
        wxFontEncoding encoding = wxFONTENCODING_DEFAULT); // ISO8859-X, ...

    // from the font components but using the font flags instead of separate
    // parameters for each flag
    static wxFont *New(int pointSize,
                       wxFontFamily family,
                       FontFlags flags = wxFontFlags::Default,
                       const std::string& face = {},
                       wxFontEncoding encoding = wxFONTENCODING_DEFAULT);


    // from the font components but using the font flags instead of separate
    // parameters for each flag
    static wxFont *New(const wxSize& pixelSize,
                       wxFontFamily family,
                       FontFlags flags = wxFontFlags::Default,
                       const std::string& face = {},
                       wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

    // from the (opaque) native font description object
    static wxFont *New(const wxNativeFontInfo& nativeFontDesc);

    // from the string representation of wxNativeFontInfo
    static wxFont *New(const std::string& strNativeFontDesc);

    // Load the font from the given file and return true on success or false on
    // error (an error message will be logged in this case).
#if wxUSE_PRIVATE_FONTS
    static bool AddPrivateFont(const std::string& filename);
#endif // wxUSE_PRIVATE_FONTS

    bool operator==(const wxFontBase& font) const;
    bool operator!=(const wxFontBase& font) const { return !(*this == font); }

    virtual int GetPointSize() const;
    virtual double GetFractionalPointSize() const = 0;
    virtual wxSize GetPixelSize() const;
    virtual bool IsUsingSizeInPixels() const;
    wxFontFamily GetFamily() const;
    virtual wxFontStyle GetStyle() const = 0;
    virtual int GetNumericWeight() const = 0;
    virtual bool GetUnderlined() const = 0;
    virtual bool GetStrikethrough() const { return false; }
    virtual std::string GetFaceName() const = 0;
    virtual wxFontEncoding GetEncoding() const = 0;
    virtual const wxNativeFontInfo *GetNativeFontInfo() const = 0;

    // Accessors that can be overridden in the platform-specific code but for
    // which we provide a reasonable default implementation in the base class.
    virtual wxFontWeight GetWeight() const;
    virtual bool IsFixedWidth() const;

    std::string GetNativeFontInfoDesc() const;
    std::string GetNativeFontInfoUserDesc() const;

    // change the font characteristics
    virtual void SetPointSize( int pointSize );
    virtual void SetFractionalPointSize( double pointSize ) = 0;
    virtual void SetPixelSize( const wxSize& pixelSize );
    virtual void SetFamily( wxFontFamily family ) = 0;
    virtual void SetStyle( wxFontStyle style ) = 0;
    virtual void SetNumericWeight( int weight ) = 0;

    virtual void SetUnderlined( bool underlined ) = 0;
    virtual void SetStrikethrough( [[maybe_unused]] bool strikethrough ) {}
    virtual void SetEncoding(wxFontEncoding encoding) = 0;
    virtual bool SetFaceName( const std::string& faceName );
    void SetNativeFontInfo(const wxNativeFontInfo& info)
        { DoSetNativeFontInfo(info); }

    // Similarly to the accessors above, the functions in this group have a
    // reasonable default implementation in the base class.
    virtual void SetWeight( wxFontWeight weight );

    bool SetNativeFontInfo(const std::string& info);
    bool SetNativeFontInfoUserDesc(const std::string& info);

    // Symbolic font sizes support: set the font size to "large" or "very
    // small" either absolutely (i.e. compared to the default font size) or
    // relatively to the given font size.
    void SetSymbolicSize(wxFontSymbolicSize size);
    void SetSymbolicSizeRelativeTo(wxFontSymbolicSize size, int base)
    {
        SetPointSize(AdjustToSymbolicSize(size, base));
    }

    // Adjust the base size in points according to symbolic size.
    static int AdjustToSymbolicSize(wxFontSymbolicSize size, int base);


    // translate the fonts into human-readable string (i.e. GetStyleString()
    // will return "wxITALIC" for an italic font, ...)
    std::string GetFamilyString() const;
    std::string GetStyleString() const;
    std::string GetWeightString() const;

    // the default encoding is used for creating all fonts with default
    // encoding parameter
    static wxFontEncoding GetDefaultEncoding() { return ms_encodingDefault; }
    static void SetDefaultEncoding(wxFontEncoding encoding);

    // Account for legacy font weight values: if the argument is one of
    // wxNORMAL, wxLIGHT or wxBOLD, return the corresponding wxFONTWEIGHT_XXX
    // enum value. Otherwise just return it unchanged.
    static int ConvertFromLegacyWeightIfNecessary(int weight);

    // Convert between symbolic and numeric font weights. This function uses
    // ConvertFromLegacyWeightIfNecessary(), so takes legacy values into
    // account as well.
    static int GetNumericWeightOf(wxFontWeight weight);

protected:
    // the function called by both overloads of SetNativeFontInfo()
    virtual void DoSetNativeFontInfo(const wxNativeFontInfo& info);

    // The function called by public GetFamily(): it can return
    // wxFontFamily::Unknown unlike the public method (see comment there).
    virtual wxFontFamily DoGetFamily() const = 0;


    // Helper functions to recover wxFONTSTYLE/wxFONTWEIGHT and underlined flag
    // values from flags containing a combination of wxFontFlags.
    static constexpr wxFontStyle GetStyleFromFlags(FontFlags flags) noexcept
    {
        return (flags & wxFontFlags::Italic)
                        ? wxFontStyle::Italic
                        : (flags & wxFontFlags::Slant)
                            ? wxFontStyle::Slant
                            : wxFontStyle::Normal;
    }

    static constexpr wxFontWeight GetWeightFromFlags(FontFlags flags) noexcept
    {
        return (flags & wxFontFlags::Light)
                        ? wxFONTWEIGHT_LIGHT
                        : (flags & wxFontFlags::Bold)
                            ? wxFONTWEIGHT_BOLD
                            : wxFONTWEIGHT_NORMAL;
    }

    static constexpr bool GetUnderlinedFromFlags(FontFlags flags) noexcept
    {
        return flags.is_set(wxFontFlags::Underlined);
    }

    static constexpr bool GetStrikethroughFromFlags(FontFlags flags) noexcept
    {
        return flags.is_set(wxFontFlags::Strikethrough);
    }

    // Create wxFontInfo object from the parameters passed to the legacy wxFont
    // ctor/Create() overload. This function implements the compatibility hack
    // which interprets wxDEFAULT value of size as meaning -1 and also supports
    // specifying wxNORMAL, wxLIGHT and wxBOLD as weight values.
    static wxFontInfo InfoFromLegacyParams(int pointSize,
                                           wxFontFamily family,
                                           wxFontStyle style,
                                           wxFontWeight weight,
                                           bool underlined,
                                           const std::string& face,
                                           wxFontEncoding encoding);

    static wxFontInfo InfoFromLegacyParams(const wxSize& pixelSize,
                                           wxFontFamily family,
                                           wxFontStyle style,
                                           wxFontWeight weight,
                                           bool underlined,
                                           const std::string& face,
                                           wxFontEncoding encoding);

private:
    // the currently default encoding: by default, it's the default system
    // encoding, but may be changed by the application using
    // SetDefaultEncoding() to make all subsequent fonts created without
    // specifying encoding parameter using this encoding
    inline static wxFontEncoding ms_encodingDefault{wxFONTENCODING_SYSTEM};
};

// wxFontBase <-> std::string utilities, used by wxConfig
std::string wxToString(const wxFontBase& font);
bool wxFromString(const std::string& str, wxFontBase* font);


// this macro must be used in all derived wxFont classes declarations
#define wxDECLARE_COMMON_FONT_METHODS() \
    /* functions for modifying font in place */ \
    wxFont& MakeBold(); \
    wxFont& MakeItalic(); \
    wxFont& MakeUnderlined(); \
    wxFont& MakeStrikethrough(); \
    wxFont& MakeLarger() { return Scale(1.2f); } \
    wxFont& MakeSmaller() { return Scale(1/1.2f); } \
    wxFont& Scale(float x); \
    /* functions for creating fonts based on this one */ \
    wxFont Bold() const; \
    wxFont GetBaseFont() const; \
    wxFont Italic() const; \
    wxFont Underlined() const; \
    wxFont Strikethrough() const; \
    wxFont Larger() const { return Scaled(1.2f); } \
    wxFont Smaller() const { return Scaled(1/1.2f); } \
    wxFont Scaled(float x) const

// include the real class declaration
#if defined(__WXMSW__)
    #include "wx/msw/font.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/font.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/font.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/font.h"
#elif defined(__WXX11__)
    #include "wx/x11/font.h"
#elif defined(__WXDFB__)
    #include "wx/dfb/font.h"
#elif defined(__WXMAC__)
    #include "wx/osx/font.h"
#elif defined(__WXQT__)
    #include "wx/qt/font.h"
#endif

class wxFontList: public wxGDIObjListBase
{
public:
    wxFont *FindOrCreateFont(int pointSize,
                             wxFontFamily family,
                             wxFontStyle style,
                             wxFontWeight weight,
                             bool underline = false,
                             const std::string& face = {},
                             wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

    wxFont *FindOrCreateFont(const wxFontInfo& fontInfo)
        { return FindOrCreateFont(fontInfo.GetPointSize(), fontInfo.GetFamily(),
                                  fontInfo.GetStyle(), fontInfo.GetWeight(),
                                  fontInfo.IsUnderlined(), fontInfo.GetFaceName(),
                                  fontInfo.GetEncoding()); }
};

inline wxFontList*    wxTheFontList;

#endif // _WX_FONT_H_BASE_
