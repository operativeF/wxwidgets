/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontcmn.cpp
// Purpose:     implementation of wxFontBase methods
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.09.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/font.h"
#include "wx/dc.h"
#include "wx/intl.h"
#include "wx/dcscreen.h"
#include "wx/log.h"
#include "wx/gdicmn.h"

#if defined(__WXMSW__)
    #include  "wx/msw/private.h"  // includes windows.h for LOGFONTW
#endif

#include "wx/fontutil.h" // for wxNativeFontInfo
#include "wx/fontmap.h"
#include "wx/fontenum.h"

#include "wx/tokenzr.h"

import Utils.Geometry;

import <charconv>;

#include <cassert>

// debugger helper: this function can be called from a debugger to show what
// the date really is
extern const std::string wxDumpFont(const wxFont *font)
{
    return fmt::format("%s-%d-%s-%.2f-%d",
             font->GetFaceName(),
             font->GetNumericWeight(),
             font->GetStyle() == wxFontStyle::Normal ? "regular" : "italic",
             font->GetFractionalPointSize(),
             font->GetEncoding());
}

// ----------------------------------------------------------------------------
// XTI
// ----------------------------------------------------------------------------

wxBEGIN_ENUM( wxFontFamily )
wxENUM_MEMBER( wxFontFamily::Default )
wxENUM_MEMBER( wxFontFamily::Decorative )
wxENUM_MEMBER( wxFontFamily::Roman )
wxENUM_MEMBER( wxFontFamily::Script )
wxENUM_MEMBER( wxFontFamily::Swiss )
wxENUM_MEMBER( wxFontFamily::Modern )
wxENUM_MEMBER( wxFontFamily::Teletype )
wxEND_ENUM( wxFontFamily )

wxBEGIN_ENUM( wxFontStyle )
wxENUM_MEMBER( wxFontStyle::Normal )
wxENUM_MEMBER( wxFontStyle::Italic )
wxENUM_MEMBER( wxFontStyle::Slant )
wxEND_ENUM( wxFontStyle )

wxBEGIN_ENUM( wxFontWeight )
wxENUM_MEMBER( wxFONTWEIGHT_THIN )
wxENUM_MEMBER( wxFONTWEIGHT_EXTRALIGHT )
wxENUM_MEMBER( wxFONTWEIGHT_LIGHT )
wxENUM_MEMBER( wxFONTWEIGHT_NORMAL )
wxENUM_MEMBER( wxFONTWEIGHT_MEDIUM )
wxENUM_MEMBER( wxFONTWEIGHT_SEMIBOLD )
wxENUM_MEMBER( wxFONTWEIGHT_BOLD )
wxENUM_MEMBER( wxFONTWEIGHT_EXTRABOLD )
wxENUM_MEMBER( wxFONTWEIGHT_HEAVY )
wxENUM_MEMBER( wxFONTWEIGHT_EXTRAHEAVY )
wxEND_ENUM( wxFontWeight )

// ----------------------------------------------------------------------------
// wxFontBase
// ----------------------------------------------------------------------------

/* static */
void wxFontBase::SetDefaultEncoding(wxFontEncoding encoding)
{
    // GetDefaultEncoding() should return something != wxFONTENCODING_DEFAULT
    // and, besides, using this value here doesn't make any sense
    wxCHECK_RET( encoding != wxFONTENCODING_DEFAULT,
                 "can't set default encoding to wxFONTENCODING_DEFAULT" );

    ms_encodingDefault = encoding;
}

/* static */
wxFont *wxFontBase::New(int size,
                        wxFontFamily family,
                        wxFontStyle style,
                        wxFontWeight weight,
                        bool underlined,
                        const std::string& face,
                        wxFontEncoding encoding)
{
    return new wxFont(size, family, style, weight, underlined, face, encoding);
}

/* static */
wxFont *wxFontBase::New(const wxSize& pixelSize,
                        wxFontFamily family,
                        wxFontStyle style,
                        wxFontWeight weight,
                        bool underlined,
                        const std::string& face,
                        wxFontEncoding encoding)
{
    return new wxFont(pixelSize, family, style, weight, underlined,
                      face, encoding);
}

/* static */
wxFont *wxFontBase::New(int pointSize,
                        wxFontFamily family,
                        FontFlags flags,
                        const std::string& face,
                        wxFontEncoding encoding)
{
    return New(pointSize, family,
               GetStyleFromFlags(flags),
               GetWeightFromFlags(flags),
               GetUnderlinedFromFlags(flags),
               face, encoding);
}

/* static */
wxFont *wxFontBase::New(const wxSize& pixelSize,
                        wxFontFamily family,
                        FontFlags flags,
                        const std::string& face,
                        wxFontEncoding encoding)
{
    return New(pixelSize, family,
               GetStyleFromFlags(flags),
               GetWeightFromFlags(flags),
               GetUnderlinedFromFlags(flags),
               face, encoding);
}

/* static */
wxFont *wxFontBase::New(const wxNativeFontInfo& info)
{
    return new wxFont(info);
}

/* static */
wxFont *wxFontBase::New(const std::string& strNativeFontDesc)
{
    wxNativeFontInfo fontInfo;
    if ( !fontInfo.FromString(strNativeFontDesc) )
        return new wxFont(*wxNORMAL_FONT);

    return New(fontInfo);
}

bool wxFontBase::IsFixedWidth() const
{
    return GetFamily() == wxFontFamily::Teletype;
}


// Convert to/from wxFontWeight enum elements and numeric weight values.

/* static */
int wxFontBase::ConvertFromLegacyWeightIfNecessary(int weight)
{
    switch ( weight )
    {
        case 90: return wxFONTWEIGHT_NORMAL;
        case 91: return wxFONTWEIGHT_LIGHT;
        case 92: return wxFONTWEIGHT_BOLD;
        default: return weight;
    }
}

/* static */
int wxFontBase::GetNumericWeightOf(wxFontWeight weight_)
{
    const int weight = ConvertFromLegacyWeightIfNecessary(weight_);

    wxASSERT(weight > wxFONTWEIGHT_INVALID);
    wxASSERT(weight <= wxFONTWEIGHT_MAX);
    wxASSERT(weight % 100 == 0);

    return weight;
}

int wxFontBase::GetPointSize() const
{
    return std::lround(GetFractionalPointSize());
}


wxSize wxFontBase::GetPixelSize() const
{
    wxScreenDC dc;
    dc.SetFont(*dynamic_cast<const wxFont*>(this));
    return wxSize(dc.wxGetCharWidth(), dc.GetCharHeight());
}

wxFontWeight wxFontBase::GetWeight() const
{
    wxCHECK_MSG( IsOk(), wxFONTWEIGHT_MAX, "invalid font" );

    return wxFontInfo::GetWeightClosestToNumericValue(GetNumericWeight());
}

bool wxFontBase::IsUsingSizeInPixels() const
{
    return false;
}

void wxFontBase::SetPointSize(int pointSize)
{
    SetFractionalPointSize(pointSize);
}

void wxFontBase::SetPixelSize( const wxSize& pixelSize )
{
    wxCHECK_RET( pixelSize.x >= 0 && pixelSize.y > 0,
                 "Negative values for the pixel size or zero pixel height are not allowed" );

    wxScreenDC dc;

    // NOTE: this algorithm for adjusting the font size is used by all
    //       implementations of wxFont except under wxMSW and wxGTK where
    //       native support to font creation using pixel-size is provided.

    int largestGood = 0;
    int smallestBad = 0;

    bool initialGoodFound = false;
    bool initialBadFound = false;

    // NB: this assignment was separated from the variable definition
    // in order to fix a gcc v3.3.3 compiler crash
    int currentSize = GetPointSize();
    while (currentSize > 0)
    {
        dc.SetFont(*dynamic_cast<wxFont*>(this));

        // if currentSize (in points) results in a font that is smaller
        // than required by pixelSize it is considered a good size
        // NOTE: the pixel size width may be zero
        if (dc.GetCharHeight() <= pixelSize.y &&
                (pixelSize.x == 0 ||
                 dc.wxGetCharWidth() <= pixelSize.x))
        {
            largestGood = currentSize;
            initialGoodFound = true;
        }
        else
        {
            smallestBad = currentSize;
            initialBadFound = true;
        }
        if (!initialGoodFound)
        {
            currentSize /= 2;
        }
        else if (!initialBadFound)
        {
            currentSize *= 2;
        }
        else
        {
            const int distance = smallestBad - largestGood;
            if (distance == 1)
                break;

            currentSize = largestGood + distance / 2;
        }

        SetPointSize(currentSize);
    }

    if (currentSize != largestGood)
        SetPointSize(largestGood);
}

void wxFontBase::SetWeight(wxFontWeight weight)
{
    SetNumericWeight(GetNumericWeightOf(weight));
}

void wxFontBase::DoSetNativeFontInfo(const wxNativeFontInfo& info)
{
#ifdef wxNO_NATIVE_FONTINFO
    SetFractionalPointSize(info.pointSize);
    SetFamily(info.family);
    SetStyle(info.style);
    SetNumericWeight(info.weight);
    SetUnderlined(info.underlined);
    SetStrikethrough(info.strikethrough);
    SetFaceName(info.faceName);
    SetEncoding(info.encoding);
#else
    (void)info;
#endif
}

std::string wxFontBase::GetNativeFontInfoDesc() const
{
    std::string fontDesc;

    wxCHECK_MSG(IsOk(), fontDesc, "invalid font");

    const wxNativeFontInfo *fontInfo = GetNativeFontInfo();
    if ( fontInfo )
    {
        fontDesc = fontInfo->ToString();
        wxASSERT_MSG(!fontDesc.empty(), "This should be a non-empty string!");
    }
    else
    {
        wxFAIL_MSG("Derived class should have created the wxNativeFontInfo!");
    }

    return fontDesc;
}

std::string wxFontBase::GetNativeFontInfoUserDesc() const
{
    std::string fontDesc;

    wxCHECK_MSG(IsOk(), fontDesc, "invalid font");

    const wxNativeFontInfo *fontInfo = GetNativeFontInfo();
    if ( fontInfo )
    {
        fontDesc = fontInfo->ToUserString();
        wxASSERT_MSG(!fontDesc.empty(), "This should be a non-empty string!");
    }
    else
    {
        wxFAIL_MSG("Derived class should have created the wxNativeFontInfo!");
    }

    return fontDesc;
}

bool wxFontBase::SetNativeFontInfo(const std::string& info)
{
    wxNativeFontInfo fontInfo;
    if ( !info.empty() && fontInfo.FromString(info) )
    {
        SetNativeFontInfo(fontInfo);
        return true;
    }

    return false;
}

bool wxFontBase::SetNativeFontInfoUserDesc(const std::string& info)
{
    wxNativeFontInfo fontInfo;
    if ( !info.empty() && fontInfo.FromUserString(info) )
    {
        SetNativeFontInfo(fontInfo);
        return true;
    }

    return false;
}

bool wxFontBase::operator==(const wxFontBase& font) const
{
    // either it is the same font, i.e. they share the same common data or they
    // have different ref datas but still describe the same font
    return IsSameAs(font) ||
           (
            IsOk() == font.IsOk() &&
            GetPointSize() == font.GetPointSize() &&
            // in wxGTK1 GetPixelSize() calls GetInternalFont() which uses
            // operator==() resulting in infinite recursion so we can't use it
            // in that port
#if (!defined(__WXGTK__) || defined(__WXGTK20__))
            GetPixelSize() == font.GetPixelSize() &&
#endif
            GetFamily() == font.GetFamily() &&
            GetStyle() == font.GetStyle() &&
            GetNumericWeight() == font.GetNumericWeight() &&
            GetUnderlined() == font.GetUnderlined() &&
            GetStrikethrough() == font.GetStrikethrough() &&
            wx::utils::IsSameAsNoCase(GetFaceName(), font.GetFaceName()) &&
            GetEncoding() == font.GetEncoding()
           );
}

wxFontFamily wxFontBase::GetFamily() const
{
    wxCHECK_MSG( IsOk(), wxFontFamily::Unknown, "invalid font" );

    // Don't return wxFontFamily::Unknown from here because it prevents the code
    // like wxFont(size, wxNORMAL_FONT->GetFamily(), ...) from working (see
    // #12330). This is really just a hack but it allows to keep compatibility
    // and doesn't really have any bad drawbacks so do this until someone comes
    // up with a better idea.
    const wxFontFamily family = DoGetFamily();

    return family == wxFontFamily::Unknown ? wxFontFamily::Default : family;
}

std::string wxFontBase::GetFamilyString() const
{
    wxCHECK_MSG( IsOk(), "wxFontFamily::Default", "invalid font" );

    switch ( GetFamily() )
    {
        case wxFontFamily::Decorative:   return "wxFontFamily::Decorative";
        case wxFontFamily::Roman:        return "wxFontFamily::Roman";
        case wxFontFamily::Script:       return "wxFontFamily::Script";
        case wxFontFamily::Swiss:        return "wxFontFamily::Swiss";
        case wxFontFamily::Modern:       return "wxFontFamily::Modern";
        case wxFontFamily::Teletype:     return "wxFontFamily::Teletype";
        case wxFontFamily::Unknown:      return "wxFontFamily::Unknown";
        default:                        return "wxFontFamily::Default";
    }
}

std::string wxFontBase::GetStyleString() const
{
    wxCHECK_MSG( IsOk(), "wxFONTSTYLE_DEFAULT", "invalid font" );

    switch ( GetStyle() )
    {
        case wxFontStyle::Normal:   return "wxFontStyle::Normal";
        case wxFontStyle::Slant:    return "wxFontStyle::Slant";
        case wxFontStyle::Italic:   return "wxFontStyle::Italic";
        default:                   return "wxFONTSTYLE_DEFAULT";
    }
}

std::string wxFontBase::GetWeightString() const
{
    wxCHECK_MSG( IsOk(), "wxFONTWEIGHT_DEFAULT", "invalid font" );

    switch ( GetWeight() )
    {
        case wxFONTWEIGHT_THIN:     return "wxFONTWEIGHT_THIN";
        case wxFONTWEIGHT_EXTRALIGHT: return "wxFONTWEIGHT_EXTRALIGHT";
        case wxFONTWEIGHT_LIGHT:    return "wxFONTWEIGHT_LIGHT";
        case wxFONTWEIGHT_NORMAL:   return "wxFONTWEIGHT_NORMAL";
        case wxFONTWEIGHT_MEDIUM:   return "wxFONTWEIGHT_MEDIUM";
        case wxFONTWEIGHT_SEMIBOLD: return "wxFONTWEIGHT_SEMIBOLD";
        case wxFONTWEIGHT_BOLD:     return "wxFONTWEIGHT_BOLD";
        case wxFONTWEIGHT_EXTRABOLD: return "wxFONTWEIGHT_EXTRABOLD";
        case wxFONTWEIGHT_HEAVY:    return "wxFONTWEIGHT_HEAVY";
        case wxFONTWEIGHT_EXTRAHEAVY:    return "wxFONTWEIGHT_EXTRAHEAVY";
        default:                    return "wxFONTWEIGHT_DEFAULT";
    }
}

bool wxFontBase::SetFaceName(const std::string& facename)
{
#if wxUSE_FONTENUM
    if (!wxFontEnumerator::IsValidFacename(facename))
    {
        UnRef();        // make IsOk() return false
        return false;
    }
#else // !wxUSE_FONTENUM
    wxUnusedVar(facename);
#endif // wxUSE_FONTENUM/!wxUSE_FONTENUM

    return true;
}

namespace
{

void InitInfoWithLegacyParams(wxFontInfo& info,
                              wxFontFamily family,
                              wxFontStyle style,
                              wxFontWeight weight,
                              bool underlined,
                              const std::string& face,
                              wxFontEncoding encoding)
{
    info
        .Family(family)
        .Style(style)
        .Weight(wxFontBase::GetNumericWeightOf(weight))
        .Underlined(underlined)
        .FaceName(face)
        .Encoding(encoding);
}

} // anonymous namespace

/* static */
wxFontInfo wxFontBase::InfoFromLegacyParams(int pointSize,
                                            wxFontFamily family,
                                            wxFontStyle style,
                                            wxFontWeight weight,
                                            bool underlined,
                                            const std::string& face,
                                            wxFontEncoding encoding)
{
    // Old code specifies wxDEFAULT instead of -1 or wxNORMAL instead of the
    // new type-safe wxFontStyle::Normal or wxFONTWEIGHT_NORMAL, continue
    // handling this for compatibility.
    // FIXME: Deprecate and replace the Legacy Font functions.
    // static constexpr int wxDEFAULT = 70; // Value taken from old gui macros.
       if ( pointSize == 70 )
           pointSize = -1;

    wxFontInfo info(pointSize);

    InitInfoWithLegacyParams(info,
                             family, style, weight, underlined, face, encoding);

    return info;
}

/* static */
wxFontInfo wxFontBase::InfoFromLegacyParams(const wxSize& pixelSize,
                                            wxFontFamily family,
                                            wxFontStyle style,
                                            wxFontWeight weight,
                                            bool underlined,
                                            const std::string& face,
                                            wxFontEncoding encoding)
{
    wxFontInfo info(pixelSize);

    InitInfoWithLegacyParams(info,
                             family, style, weight, underlined, face, encoding);

    return info;
}

void wxFontBase::SetSymbolicSize(wxFontSymbolicSize size)
{
    SetSymbolicSizeRelativeTo(size, wxNORMAL_FONT->GetPointSize());
}

/* static */
int wxFontBase::AdjustToSymbolicSize(wxFontSymbolicSize size, int base)
{
    // Using a fixed factor (1.2, from CSS2) is a bad idea as explained at
    // http://www.w3.org/TR/CSS21/fonts.html#font-size-props so use the values
    // from http://style.cleverchimp.com/font_size_intervals/altintervals.html
    // instead.
    static constexpr float factors[] = { 0.60f, 0.75f, 0.89f, 1.f, 1.2f, 1.5f, 2.f };

    static_assert
    (
        WXSIZEOF(factors) == wxFONTSIZE_XX_LARGE - wxFONTSIZE_XX_SMALL + 1,
        "Wrong font size / factors size."
    );

    return std::lround(factors[size - wxFONTSIZE_XX_SMALL]*base);
}

wxFont& wxFont::MakeBold()
{
    SetWeight(wxFONTWEIGHT_BOLD);
    return *this;
}

wxFont wxFont::Bold() const
{
    wxFont font(*this);
    font.MakeBold();
    return font;
}

wxFont wxFont::GetBaseFont() const
{
    wxFont font(*this);
    font.SetStyle(wxFontStyle::Normal);
    font.SetWeight(wxFONTWEIGHT_NORMAL );
    font.SetUnderlined(false);
    font.SetStrikethrough(false);
    return font;
}

wxFont& wxFont::MakeItalic()
{
    SetStyle(wxFontStyle::Italic);
    return *this;
}

wxFont wxFont::Italic() const
{
    wxFont font(*this);
    font.MakeItalic();
    return font;
}

wxFont& wxFont::MakeUnderlined()
{
    SetUnderlined(true);
    return *this;
}

wxFont wxFont::Underlined() const
{
    wxFont font(*this);
    font.MakeUnderlined();
    return font;
}

wxFont wxFont::Strikethrough() const
{
    wxFont font(*this);
    font.MakeStrikethrough();
    return font;
}

wxFont& wxFont::MakeStrikethrough()
{
    SetStrikethrough(true);
    return *this;
}

wxFont& wxFont::Scale(float x)
{
    SetFractionalPointSize(double(x) * GetFractionalPointSize());
    return *this;
}

wxFont wxFont::Scaled(float x) const
{
    wxFont font(*this);
    font.Scale(x);
    return font;
}

// ----------------------------------------------------------------------------
// wxNativeFontInfo
// ----------------------------------------------------------------------------

// Up to now, there are no native implementations of this function:
// FIXME: Case sensitivity, does it matter here?
void wxNativeFontInfo::SetFaceName(const std::vector<std::string>& facenames)
{
#if wxUSE_FONTENUM
    auto possible_name = std::ranges::find_if(facenames,
        [](const auto& face){ return wxFontEnumerator::IsValidFacename(face); });

    if(possible_name != facenames.end())
    {
        SetFaceName(*possible_name);
        return;
    }

    // set the first valid facename we can find on this system
    const std::string validfacename = *wxFontEnumerator::GetFacenames().cbegin();
    wxLogTrace("font", "Falling back to '%s'", validfacename.c_str());
    SetFaceName(validfacename);
#else // !wxUSE_FONTENUM
    SetFaceName(facenames[0]);
#endif // wxUSE_FONTENUM/!wxUSE_FONTENUM
}

int wxNativeFontInfo::GetPointSize() const
{
    return std::lround(GetFractionalPointSize());
}

void wxNativeFontInfo::SetPointSize(int pointsize)
{
    SetFractionalPointSize(pointsize);
}

#ifdef wxNO_NATIVE_FONTINFO

// These are the generic forms of FromString()/ToString.
//
// convert to/from the string representation: the general format is
// "version;the rest..." with currently defined versions being:
//
//      0;pointsize;family;style;weight;underlined;facename;encoding
//      1;pointsize;family;style;weight;underlined;strikethrough;facename;encoding

bool wxNativeFontInfo::FromString(const std::string& s)
{
    long l;
    double d;
    unsigned long version;

    wxStringTokenizer tokenizer(s, ";");

    std::string token = tokenizer.GetNextToken();
    if ( !token.ToULong(&version) || version > 1 )
        return false;

    token = tokenizer.GetNextToken();
    if ( !token.ToCDouble(&d) )
        return false;
    if ( d < 0 )
        return false;
    pointSize = d;

    token = tokenizer.GetNextToken();
    if ( !token.ToLong(&l) )
        return false;
    family = (wxFontFamily)l;

    token = tokenizer.GetNextToken();
    if ( !token.ToLong(&l) )
        return false;
    style = (wxFontStyle)l;

    token = tokenizer.GetNextToken();
    if ( !token.ToLong(&l) )
        return false;
    weight = wxFont::ConvertFromLegacyWeightIfNecessary(l);
    if ( weight <= wxFONTWEIGHT_INVALID || weight > wxFONTWEIGHT_MAX )
        return false;

    token = tokenizer.GetNextToken();
    if ( !token.ToLong(&l) )
        return false;
    underlined = l != 0;

    if ( version == 1 )
    {
        token = tokenizer.GetNextToken();
        if ( !token.ToLong(&l) )
            return false;
        strikethrough = l != 0;
    }

    faceName = tokenizer.GetNextToken();

#ifndef __WXMAC__
    if( !faceName )
        return false;
#endif

    token = tokenizer.GetNextToken();
    if ( !token.ToLong(&l) )
        return false;
    encoding = (wxFontEncoding)l;

    return true;
}

std::string wxNativeFontInfo::ToString() const
{
    return fmt::format("%d;%s;%d;%d;%d;%d;%d;%s;%d",
             1,                                 // version
             GetFractionalPointSize(),
             family,
             (int)style,
             weight,
             underlined,
             strikethrough,
             faceName.GetData(),
             (int)encoding);
}

void wxNativeFontInfo::Init()
{
    pointSize = 0.0f;
    family = wxFontFamily::Default;
    style = wxFontStyle::Normal;
    weight = wxFONTWEIGHT_NORMAL;
    underlined = false;
    strikethrough = false;
    faceName.clear();
    encoding = wxFONTENCODING_DEFAULT;
}

double wxNativeFontInfo::GetFractionalPointSize() const
{
    return pointSize;
}

wxFontStyle wxNativeFontInfo::GetStyle() const
{
    return style;
}

int wxNativeFontInfo::GetNumericWeight() const
{
    return weight;
}

bool wxNativeFontInfo::GetUnderlined() const
{
    return underlined;
}

bool wxNativeFontInfo::GetStrikethrough() const
{
    return strikethrough;
}

std::string wxNativeFontInfo::GetFaceName() const
{
    return faceName;
}

wxFontFamily wxNativeFontInfo::GetFamily() const
{
    return family;
}

wxFontEncoding wxNativeFontInfo::GetEncoding() const
{
    return encoding;
}

void wxNativeFontInfo::SetFractionalPointSize(double pointsize)
{
    pointSize = pointsize;
}

void wxNativeFontInfo::SetStyle(wxFontStyle style_)
{
    style = style_;
}

void wxNativeFontInfo::SetNumericWeight(int weight_)
{
    weight = weight_;
}

void wxNativeFontInfo::SetUnderlined(bool underlined_)
{
    underlined = underlined_;
}

void wxNativeFontInfo::SetStrikethrough(bool strikethrough_)
{
    strikethrough = strikethrough_;
}

bool wxNativeFontInfo::SetFaceName(const std::string& facename_)
{
    faceName = facename_;
    return true;
}

void wxNativeFontInfo::SetFamily(wxFontFamily family_)
{
    family = family_;
}

void wxNativeFontInfo::SetEncoding(wxFontEncoding encoding_)
{
    encoding = encoding_;
}

#endif // generic wxNativeFontInfo implementation

// conversion to/from user-readable string: this is used in the generic
// versions and under MSW as well because there is no standard font description
// format there anyhow (but there is a well-defined standard for X11 fonts used
// by wxGTK and wxMotif)

#if defined(wxNO_NATIVE_FONTINFO) || defined(__WXMSW__) || defined(__WXOSX__)

std::string wxNativeFontInfo::ToUserString() const
{
    std::string desc;

    // first put the adjectives, if any - this is English-centric, of course,
    // but what else can we do?
    if ( GetUnderlined() )
    {
        desc += _("underlined").ToStdString();
    }

    if ( GetStrikethrough() )
    {
        desc += _(" strikethrough").ToStdString();
    }

    switch ( GetWeight() )
    {
        default:
            wxFAIL_MSG( "unknown font weight" );
            [[fallthrough]];

        case wxFONTWEIGHT_NORMAL:
            break;

        case wxFONTWEIGHT_THIN:
            desc += _(" thin").ToStdString();
            break;

        case wxFONTWEIGHT_EXTRALIGHT:
            desc += _(" extra light").ToStdString();
            break;

        case wxFONTWEIGHT_LIGHT:
            desc += _(" light").ToStdString();
            break;

        case wxFONTWEIGHT_MEDIUM:
            desc += _(" medium").ToStdString();
            break;

        case wxFONTWEIGHT_SEMIBOLD:
            desc += _(" semi bold").ToStdString();
            break;

        case wxFONTWEIGHT_BOLD:
            desc += _(" bold").ToStdString();
            break;

        case wxFONTWEIGHT_EXTRABOLD:
            desc += _(" extra bold").ToStdString();
            break;

        case wxFONTWEIGHT_HEAVY:
            desc += _(" heavy").ToStdString();
            break;

        case wxFONTWEIGHT_EXTRAHEAVY:
            desc += _(" extra heavy").ToStdString();
            break;
    }

    switch ( GetStyle() )
    {
        default:
            wxFAIL_MSG( "unknown font style" );
            [[fallthrough]];

        case wxFontStyle::Normal:
            break;

            // we don't distinguish between the two for now anyhow...
        case wxFontStyle::Italic:
        case wxFontStyle::Slant:
            desc += _(" italic").ToStdString();
            break;
    }

    std::string face = GetFaceName();
    if ( !face.empty() )
    {
        if(wx::utils::ContainsAnyOf(face, " ", ";", ","))
        {
            std::erase(face, '\'');
            // eventually remove quote characters: most systems do not
            // allow them in a facename anyway so this usually does nothing

            // make it possible for FromUserString() function to understand
            // that the different words which compose this facename are
            // not different adjectives or other data but rather all parts
            // of the facename
            desc += fmt::format(" '{}'", face);
        }
        else
        {
            desc += fmt::format(" {}", face);
        }
    }
    else // no face name specified
    {
        // use the family
        std::string familyStr;
        switch ( GetFamily() )
        {
            case wxFontFamily::Decorative:
                familyStr = "decorative";
                break;

            case wxFontFamily::Roman:
                familyStr = "roman";
                break;

            case wxFontFamily::Script:
                familyStr = "script";
                break;

            case wxFontFamily::Swiss:
                familyStr = "swiss";
                break;

            case wxFontFamily::Modern:
                familyStr = "modern";
                break;

            case wxFontFamily::Teletype:
                familyStr = "teletype";
                break;

            case wxFontFamily::Default:
            case wxFontFamily::Unknown:
                break;

            default:
                wxFAIL_MSG( "unknown font family" );
        }

        if ( !familyStr.empty() )
            desc += fmt::format(" '{} family'", familyStr);
    }

    const int size = GetPointSize();
    if ( size != wxNORMAL_FONT->GetPointSize() )
    {
        desc += fmt::format(" {}", size);
    }

#if wxUSE_FONTMAP
    const wxFontEncoding enc = GetEncoding();
    if ( enc != wxFONTENCODING_DEFAULT && enc != wxFONTENCODING_SYSTEM )
    {
        desc += fmt::format(" {}", wxFontMapper::GetEncodingName(enc));
    }
#endif // wxUSE_FONTMAP

    return wx::utils::ToLowerCopy(wx::utils::StripAllSpace(desc));
}

bool wxNativeFontInfo::FromUserString(const std::string& s)
{
    // reset to the default state
    Init();

    // ToUserString() will quote the facename if it contains spaces, commas
    // or semicolons: we must be able to understand that quoted text is
    // a single token:

    // parse a more or less free form string
    wxStringTokenizer tokenizer(s, ";, ", wxStringTokenizerMode::StrTok);

    std::string face;
    unsigned long size;
    bool weightfound = false, pointsizefound = false;
#if wxUSE_FONTMAP
    bool encodingfound = false;
#endif
    bool insideQuotes = false;
    bool extraQualifierFound = false;
    bool semiQualifierFound = false;

    while ( tokenizer.HasMoreTokens() )
    {
        // normalize it
        std::string token = wx::utils::ToLowerCopy(wx::utils::StripAllSpace(tokenizer.GetNextToken()));

        if (insideQuotes)
        {
            if (token.starts_with('\'') ||
                token.ends_with('\''))
            {
                insideQuotes = false;

                // add this last token to the facename:
                face += " " + token;

                // normalize facename:
                wx::utils::TrimAllSpace(face);

                std::erase(face, '\'');

                continue;
            }
        }
        else
        {
            if (token.starts_with('\''))
                insideQuotes = true;
        }

        // look for the known tokens
        if ( insideQuotes )
        {
            // only the facename may be quoted:
            face += " " + token;
            continue;
        }

        auto [p, ec] = std::from_chars(token.data(), token.data() + token.size(), size);

        if ( token == "underlined" || token == _("underlined").ToStdString() )
        {
            SetUnderlined(true);
        }
        else if ( token == "strikethrough" || token == _("strikethrough").ToStdString() )
        {
            SetStrikethrough(true);
        }
        else if ( token == "underlinedstrikethrough" )
        {
            SetUnderlined(true);
            SetStrikethrough(true);
        }
        else if ( token == "thin" || token == _("thin").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_THIN);
            weightfound = true;
        }
        else if ( token == "extra" || token == "ultra" )
        {
            extraQualifierFound = true;
        }
        else if ( token == "semi" || token == "demi" )
        {
            semiQualifierFound = true;
        }
       else if ( token == "extralight" || token == _("extralight").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_EXTRALIGHT);
            weightfound = true;
        }
        else if ( token == "light" || token == _("light").ToStdString() )
        {
            if ( extraQualifierFound )
                SetWeight(wxFONTWEIGHT_EXTRALIGHT);
            else
                SetWeight(wxFONTWEIGHT_LIGHT);
            weightfound = true;
        }
        else if ( token == "normal" || token == _("normal").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_NORMAL);
            weightfound = true;
        }
        else if ( token == "medium" || token == _("medium").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_MEDIUM);
            weightfound = true;
        }
        else if ( token == "semibold" || token == _("semibold").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_SEMIBOLD);
            weightfound = true;
        }
        else if ( token == "bold" || token == _("bold").ToStdString() )
        {
            if ( extraQualifierFound )
                SetWeight(wxFONTWEIGHT_EXTRABOLD);
            else if ( semiQualifierFound )
                SetWeight(wxFONTWEIGHT_SEMIBOLD);
            else
                SetWeight(wxFONTWEIGHT_BOLD);
            weightfound = true;
        }
        else if ( token == "extrabold" || token == _("extrabold").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_EXTRABOLD);
            weightfound = true;
        }
        else if ( token == "semibold" || token == _("semibold").ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_SEMIBOLD);
            weightfound = true;
        }
        else if ( token == "heavy" || token == _("heavy").ToStdString() )
        {
            if ( extraQualifierFound )
                SetWeight(wxFONTWEIGHT_EXTRAHEAVY);
            else
                SetWeight(wxFONTWEIGHT_HEAVY);
            weightfound = true;
        }
        else if ( token == "extraheavy" || token == _("extraheavy" ).ToStdString() )
        {
            SetWeight(wxFONTWEIGHT_EXTRAHEAVY);
            weightfound = true;
        }
        else if ( token == "italic" || token == _("italic").ToStdString() )
        {
            SetStyle(wxFontStyle::Italic);
        }
        else if ( ec == std::errc() )
        {
            SetPointSize(size);
            pointsizefound = true;
        }
        else
        {
#if wxUSE_FONTMAP
            // try to interpret this as an encoding
            const wxFontEncoding encoding = wxFontMapper::Get()->CharsetToEncoding(token, false);
            if ( encoding != wxFONTENCODING_DEFAULT &&
                 encoding != wxFONTENCODING_SYSTEM )    // returned when the recognition failed
        {
            SetEncoding(encoding);
                encodingfound = true;
        }
            else
        {
#endif // wxUSE_FONTMAP

                // assume it is the face name
            if ( !face.empty() )
            {
                face += ' ';
            }

            face += token;

            // skip the code which resets face below
            continue;

#if wxUSE_FONTMAP
        }
#endif // wxUSE_FONTMAP
        }

        // if we had had the facename, we shouldn't continue appending tokens
        // to it (i.e. "foo bold bar" shouldn't result in the facename "foo
        // bar")
        if ( !face.empty() )
        {
            auto familyPos = face.rfind(" family");
            if ( familyPos != std::string::npos )
            {
                std::string familyStr{face.substr(0, familyPos)};
                // it's not a facename but rather a font family
                wxFontFamily family;
                if ( familyStr == "decorative" )
                    family = wxFontFamily::Decorative;
                else if ( familyStr == "roman" )
                    family = wxFontFamily::Roman;
                else if ( familyStr == "script" )
                    family = wxFontFamily::Script;
                else if ( familyStr == "swiss" )
                    family = wxFontFamily::Swiss;
                else if ( familyStr == "modern" )
                    family = wxFontFamily::Modern;
                else if ( familyStr == "teletype" )
                    family = wxFontFamily::Teletype;
                else
                    return false;

                SetFamily(family);
            }
            // NB: the check on the facename is implemented in wxFontBase::SetFaceName
            //     and not in wxNativeFontInfo::SetFaceName thus we need to explicitly
            //     call here wxFontEnumerator::IsValidFacename
            else if (
#if wxUSE_FONTENUM
                    !wxFontEnumerator::IsValidFacename(face) ||
#endif // wxUSE_FONTENUM
                    !SetFaceName(face) )
            {
                SetFaceName(wxNORMAL_FONT->GetFaceName());
            }

            face.clear();
        }
    }

    // we might not have flushed it inside the loop
    if ( !face.empty() )
    {
        // NB: the check on the facename is implemented in wxFontBase::SetFaceName
        //     and not in wxNativeFontInfo::SetFaceName thus we need to explicitly
        //     call here wxFontEnumerator::IsValidFacename
        if (
#if wxUSE_FONTENUM
                !wxFontEnumerator::IsValidFacename(face) ||
#endif // wxUSE_FONTENUM
                !SetFaceName(face) )
            {
                SetFaceName(wxNORMAL_FONT->GetFaceName());
            }
    }

    // set point size to default value if size was not given
    if ( !pointsizefound )
        SetFractionalPointSize(wxNORMAL_FONT->GetFractionalPointSize());

    // set font weight to default value if weight was not given
    if ( !weightfound )
        SetWeight(wxFONTWEIGHT_NORMAL);

#if wxUSE_FONTMAP
    // set font encoding to default value if encoding was not given
    if ( !encodingfound )
        SetEncoding(wxFONTENCODING_SYSTEM);
#endif // wxUSE_FONTMAP

    return true;
}

#endif // generic or wxMSW

// compatibility functions using old API implemented using numeric weight values

wxFontWeight wxNativeFontInfo::GetWeight() const
{
    return wxFontInfo::GetWeightClosestToNumericValue(GetNumericWeight());
}

void wxNativeFontInfo::SetWeight(wxFontWeight weight)
{
    const int numWeight = wxFontBase::GetNumericWeightOf(weight);
    if ( numWeight != GetNumericWeight() )
        SetNumericWeight(numWeight);
}

// wxFont <-> std::string utilities, used by wxConfig
std::string wxToString(const wxFontBase& font)
{
    return font.IsOk() ? font.GetNativeFontInfoDesc()
                       : std::string();
}

bool wxFromString(const std::string& str, wxFontBase *font)
{
    wxCHECK_MSG( font, false, "NULL output parameter" );

    if ( str.empty() )
    {
        *font = wxNullFont;
        return true;
    }

    return font->SetNativeFontInfo(str);
}
