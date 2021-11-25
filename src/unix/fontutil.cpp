/////////////////////////////////////////////////////////////////////////////
// Name:        src/unix/fontutil.cpp
// Purpose:     Font helper functions for wxX11, wxGTK, wxMotif
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05.11.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#include "wx/fontutil.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/font.h" // wxFont enums
    #include "wx/hash.h"
    #include "wx/utils.h"       // for wxGetDisplay()
    #include "wx/module.h"
#endif // PCH

#include "wx/encinfo.h"
#include "wx/fontmap.h"
#include "wx/math.h"
#include "wx/tokenzr.h"
#include "wx/fontenum.h"

#if wxUSE_PANGO

#include <pango/pango.h>

PangoContext* wxGetPangoContext();

#ifdef __WXGTK20__
    #include "wx/gtk/private.h"

    #define wxPANGO_CONV wxGTK_CONV_SYS
    #define wxPANGO_CONV_BACK wxGTK_CONV_BACK_SYS
#else
    #include "wx/x11/private.h"
    #include "wx/gtk/private/string.h"

    #define wxPANGO_CONV(s) s.utf8_str()
    #define wxPANGO_CONV_BACK(s) wxString::FromUTF8Unchecked(s)
#endif

// ----------------------------------------------------------------------------
// wxNativeFontInfo
// ----------------------------------------------------------------------------

void wxNativeFontInfo::Init()
{
    description = NULL;
    m_underlined = false;
    m_strikethrough = false;
}

void wxNativeFontInfo::Init(const wxNativeFontInfo& info)
{
    if (info.description)
    {
        description = pango_font_description_copy(info.description);
        m_underlined = info.GetUnderlined();
        m_strikethrough = info.GetStrikethrough();
    }
    else
    {
        description = NULL;
        m_underlined = false;
        m_strikethrough = false;
    }
}

void wxNativeFontInfo::Free()
{
    if (description)
        pango_font_description_free(description);
}

double wxNativeFontInfo::GetFractionalPointSize() const
{
    return double(pango_font_description_get_size(description)) / PANGO_SCALE;
}

wxFontStyle wxNativeFontInfo::GetStyle() const
{
    wxFontStyle m_style = wxFontStyle::Normal;

    switch (pango_font_description_get_style( description ))
    {
        case PANGO_STYLE_NORMAL:
            m_style = wxFontStyle::Normal;
            break;
        case PANGO_STYLE_ITALIC:
            m_style = wxFontStyle::Italic;
            break;
        case PANGO_STYLE_OBLIQUE:
            m_style = wxFontStyle::Slant;
            break;
    }

    return m_style;
}

int wxNativeFontInfo::GetNumericWeight() const
{
    // We seem to currently initialize only by string.
    // In that case PANGO_FONT_MASK_WEIGHT is always set.
    // if (!(pango_font_description_get_set_fields(description) & PANGO_FONT_MASK_WEIGHT))
    //    return wxFONTWEIGHT_NORMAL;

    PangoWeight pango_weight = pango_font_description_get_weight( description );
    return pango_weight;
}

bool wxNativeFontInfo::GetUnderlined() const
{
    return m_underlined;
}

bool wxNativeFontInfo::GetStrikethrough() const
{
    return m_strikethrough;
}

wxString wxNativeFontInfo::GetFaceName() const
{
    // the Pango "family" is the wx "face name"
    return wxPANGO_CONV_BACK(pango_font_description_get_family(description));
}

wxFontFamily wxNativeFontInfo::GetFamily() const
{
    wxFontFamily ret = wxFontFamily::Unknown;

    const char *family_name = pango_font_description_get_family( description );

    // note: not passing -1 as the 2nd parameter to g_ascii_strdown to work
    // around a bug in the 64-bit glib shipped with solaris 10, -1 causes it
    // to try to allocate 2^32 bytes.
    if ( !family_name )
        return ret;
    wxGtkString family_text(g_ascii_strdown(family_name, strlen(family_name)));

    // Check for some common fonts, to salvage what we can from the current
    // win32 centric wxFont API:
    if (wxStrnicmp( family_text, "monospace", 9 ) == 0)
        ret = wxFontFamily::Teletype;    // begins with "Monospace"
    else if (wxStrnicmp( family_text, "courier", 7 ) == 0)
        ret = wxFontFamily::Teletype;    // begins with "Courier"
#if defined(__WXGTK20__) || defined(HAVE_PANGO_FONT_FAMILY_IS_MONOSPACE)
    else
    {
        PangoFontFamily **families;
        PangoFontFamily  *family = NULL;
        int n_families;
        PangoContext* context = wxGetPangoContext();
        pango_context_list_families(context, &families, &n_families);

        for (int i = 0; i < n_families; ++i)
        {
            if (g_ascii_strcasecmp(pango_font_family_get_name( families[i] ),
                                   pango_font_description_get_family( description )) == 0 )
            {
                family = families[i];
                break;
            }
        }

        g_free(families);
        g_object_unref(context);

        // Some gtk+ systems might query for a non-existing font from
        // wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT) on initialization,
        // don't assert until wxSystemSettings::GetFont is checked for this - MR
        // wxASSERT_MSG( family, "No appropriate PangoFontFamily found for ::description" );

        if (family != NULL && pango_font_family_is_monospace( family ))
            ret = wxFontFamily::Teletype; // is deemed a monospace font by pango
    }
#endif // GTK+ 2 || HAVE_PANGO_FONT_FAMILY_IS_MONOSPACE

    if (ret == wxFontFamily::Unknown)
    {
        if (strstr( family_text, "sans" ) != NULL || strstr( family_text, "Sans" ) != NULL)
            // checked before serif, so that "* Sans Serif" fonts are detected correctly
            ret = wxFontFamily::Swiss;       // contains "Sans"
        else if (strstr( family_text, "serif" ) != NULL || strstr( family_text, "Serif" ) != NULL)
            ret = wxFontFamily::Roman;       // contains "Serif"
        else if (wxStrnicmp( family_text, "times", 5 ) == 0)
            ret = wxFontFamily::Roman;       // begins with "Times"
        else if (wxStrnicmp( family_text, "old", 3 ) == 0)
            ret = wxFontFamily::Decorative;  // begins with "Old" - "Old English", "Old Town"
    }

    return ret;
}

wxFontEncoding wxNativeFontInfo::GetEncoding() const
{
    return wxFONTENCODING_SYSTEM;
}

void wxNativeFontInfo::SetFractionalPointSize(double pointsize)
{
    pango_font_description_set_size( description, wxRound(pointsize * PANGO_SCALE) );
}

void wxNativeFontInfo::SetStyle(wxFontStyle style)
{
    switch (style)
    {
        case wxFontStyle::Italic:
            pango_font_description_set_style( description, PANGO_STYLE_ITALIC );
            break;
        case wxFontStyle::Slant:
            pango_font_description_set_style( description, PANGO_STYLE_OBLIQUE );
            break;
        default:
            wxFAIL_MSG( "unknown font style" );
            [[fallthrough]];
        case wxFontStyle::Normal:
            pango_font_description_set_style( description, PANGO_STYLE_NORMAL );
            break;
    }
}

void wxNativeFontInfo::SetNumericWeight(int weight)
{
    pango_font_description_set_weight(description, (PangoWeight) weight);
}

void wxNativeFontInfo::SetUnderlined(bool underlined)
{
    // Pango doesn't have the underlined attribute so we store it separately
    // (and handle it specially in wxWindowDCImpl::DoDrawText()).
    m_underlined = underlined;
}

void wxNativeFontInfo::SetStrikethrough(bool strikethrough)
{
    // As with the underlined attribute above, we handle this one separately as
    // Pango doesn't support it as part of the font description.
    m_strikethrough = strikethrough;
}

bool wxNativeFontInfo::SetFaceName(const wxString& facename)
{
    pango_font_description_set_family(description, wxPANGO_CONV(facename));

    // we return true because Pango doesn't tell us if the call failed or not;
    // instead on wxGTK wxFont::SetFaceName() will call wxFontBase::SetFaceName()
    // which does the check
    return true;
}

void wxNativeFontInfo::SetFamily(wxFontFamily family)
{
    wxArrayString facename;

    // the list of fonts associated with a family was partially
    // taken from http://www.codestyle.org/css/font-family

    switch ( family )
    {
        case wxFontFamily::Script:
            // corresponds to the cursive font family in the page linked above
            facename.Add("URW Chancery L");
            facename.Add("Comic Sans MS");
            break;

        case wxFontFamily::Decorative:
            // corresponds to the fantasy font family in the page linked above
            facename.Add("Impact");
            break;

        case wxFontFamily::Roman:
            // corresponds to the serif font family in the page linked above
            facename.Add("Serif");
            facename.Add("DejaVu Serif");
            facename.Add("DejaVu LGC Serif");
            facename.Add("Bitstream Vera Serif");
            facename.Add("Liberation Serif");
            facename.Add("FreeSerif");
            facename.Add("Luxi Serif");
            facename.Add("Times New Roman");
            facename.Add("Century Schoolbook L");
            facename.Add("URW Bookman L");
            facename.Add("URW Palladio L");
            facename.Add("Times");
            break;

        case wxFontFamily::Teletype:
        case wxFontFamily::Modern:
            // corresponds to the monospace font family in the page linked above
            facename.Add("Monospace");
            facename.Add("DejaVu Sans Mono");
            facename.Add("DejaVu LGC Sans Mono");
            facename.Add("Bitstream Vera Sans Mono");
            facename.Add("Liberation Mono");
            facename.Add("FreeMono");
            facename.Add("Luxi Mono");
            facename.Add("Courier New");
            facename.Add("Lucida Sans Typewriter");
            facename.Add("Nimbus Mono L");
            facename.Add("Andale Mono");
            facename.Add("Courier");
            break;

        case wxFontFamily::Swiss:
        case wxFontFamily::Default:
        default:
            // corresponds to the sans-serif font family in the page linked above
            facename.Add("Sans");
            facename.Add("DejaVu Sans");
            facename.Add("DejaVu LGC Sans");
            facename.Add("Bitstream Vera Sans");
            facename.Add("Liberation Sans");
            facename.Add("FreeSans");
            facename.Add("Luxi Sans");
            facename.Add("Arial");
            facename.Add("Lucida Sans");
            facename.Add("Nimbus Sans L");
            facename.Add("URW Gothic L");
            break;
    }

    SetFaceName(facename);
}

void wxNativeFontInfo::SetEncoding([[maybe_unused]] wxFontEncoding encoding)
{
    wxFAIL_MSG( "not implemented: Pango encoding is always UTF8" );
}

bool wxNativeFontInfo::FromString(const wxString& s)
{
    wxString str(s);

    // Pango font description doesn't have 'underlined' or 'strikethrough'
    // attributes, so we handle them specially by extracting them from the
    // string before passing it to Pango.
    m_underlined = str.StartsWith("underlined ", &str);
    m_strikethrough = str.StartsWith("strikethrough ", &str);

    if (description)
        pango_font_description_free( description );

    // there is a bug in at least pango <= 1.13 which makes it (or its backends)
    // segfault for very big point sizes and for negative point sizes.
    // To workaround that bug for pango <= 1.13
    // (see http://bugzilla.gnome.org/show_bug.cgi?id=340229)
    // we do the check on the size here using same (arbitrary) limits used by
    // pango > 1.13. Note that the segfault could happen also for pointsize
    // smaller than this limit !!
    const size_t pos = str.find_last_of(" ");
    double size;
    if ( pos != wxString::npos && wxString(str, pos + 1).ToDouble(&size) )
    {
        wxString sizeStr;
        if ( size < 1 )
            sizeStr = "1";
        else if ( size >= 1E6 )
            sizeStr = "1E6";

        if ( !sizeStr.empty() )
        {
            // replace the old size with the adjusted one
            str = wxString(s, 0, pos) + sizeStr;
        }
    }

    description = pango_font_description_from_string(wxPANGO_CONV(str));

#if wxUSE_FONTENUM
    // ensure a valid facename is selected
    if (!wxFontEnumerator::IsValidFacename(GetFaceName()))
        SetFaceName(wxNORMAL_FONT->GetFaceName());
#endif // wxUSE_FONTENUM

    return true;
}

wxString wxNativeFontInfo::ToString() const
{
    wxGtkString str(pango_font_description_to_string( description ));
    wxString desc = wxPANGO_CONV_BACK(str);

    // Augment the string with the attributes not handled by Pango.
    //
    // Notice that we must add them in the same order they are extracted in
    // FromString() above.
    if (m_strikethrough)
        desc.insert(0, "strikethrough ");
    if (m_underlined)
        desc.insert(0, "underlined ");

    return desc;
}

bool wxNativeFontInfo::FromUserString(const wxString& s)
{
    return FromString( s );
}

wxString wxNativeFontInfo::ToUserString() const
{
    return ToString();
}

#else // GTK+ 1.x

#ifdef __X__
    #ifdef __VMS__
        #pragma message disable nosimpint
    #endif

    #include <X11/Xlib.h>

    #ifdef __VMS__
        #pragma message enable nosimpint
    #endif

#elif defined(__WXGTK__)
    // we have to declare struct tm to avoid problems with first forward
    // declaring it in C code (glib.h included from gdk.h does it) and then
    // defining it when time.h is included from the headers below - this is
    // known not to work at least with Sun CC 6.01
    #include <time.h>

    #include <gdk/gdk.h>
#endif


// ----------------------------------------------------------------------------
// private data
// ----------------------------------------------------------------------------

static wxHashTable *g_fontHash = NULL;

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// define the functions to create and destroy native fonts for this toolkit
#ifdef __X__
    wxNativeFont wxLoadFont(const wxString& fontSpec)
    {
        return XLoadQueryFont((Display *)wxGetDisplay(), fontSpec);
    }

    inline void wxFreeFont(wxNativeFont font)
    {
        XFreeFont((Display *)wxGetDisplay(), (XFontStruct *)font);
    }
#elif defined(__WXGTK__)
    wxNativeFont wxLoadFont(const wxString& fontSpec)
    {
        // VZ: we should use gdk_fontset_load() instead of gdk_font_load()
        //     here to be able to display Japanese fonts correctly (at least
        //     this is what people report) but unfortunately doing it results
        //     in tons of warnings when using GTK with "normal" European
        //     languages and so we can't always do it and I don't know enough
        //     to determine when should this be done... (FIXME)
        return gdk_font_load( wxConvertWX2MB(fontSpec) );
    }

    inline void wxFreeFont(wxNativeFont font)
    {
        gdk_font_unref(font);
    }
#else
    #error "Unknown GUI toolkit"
#endif

static bool wxTestFontSpec(const wxString& fontspec);

static wxNativeFont wxLoadQueryFont(double pointSize,
                                    wxFontFamily family,
                                    wxFontStyle style,
                                    int weight,
                                    bool underlined,
                                    const wxString& facename,
                                    const wxString& xregistry,
                                    const wxString& xencoding,
                                    wxString* xFontName);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxNativeEncodingInfo
// ----------------------------------------------------------------------------

// convert to/from the string representation: format is
//      encodingid;registry;encoding[;facename]
bool wxNativeEncodingInfo::FromString(const wxString& s)
{
    // use ";", not "-" because it may be part of encoding name
    wxStringTokenizer tokenizer(s, ";");

    wxString encid = tokenizer.GetNextToken();
    long enc;
    if ( !encid.ToLong(&enc) )
        return false;
    encoding = (wxFontEncoding)enc;

    xregistry = tokenizer.GetNextToken();
    if ( !xregistry )
        return false;

    xencoding = tokenizer.GetNextToken();
    if ( !xencoding )
        return false;

    // ok even if empty
    facename = tokenizer.GetNextToken();

    return true;
}

wxString wxNativeEncodingInfo::ToString() const
{
    wxString s;
    s << (long)encoding << wxT(';') << xregistry << wxT(';') << xencoding;
    if ( !facename.empty() )
    {
        s << wxT(';') << facename;
    }

    return s;
}

// ----------------------------------------------------------------------------
// wxNativeFontInfo
// ----------------------------------------------------------------------------

void wxNativeFontInfo::Init()
{
    m_isDefault = true;
}

bool wxNativeFontInfo::FromString(const wxString& s)
{
    wxStringTokenizer tokenizer(s, ";");

    // check the version
    wxString token = tokenizer.GetNextToken();
    if ( token != wxT('0') )
        return false;

    xFontName = tokenizer.GetNextToken();

    // this should be the end
    if ( tokenizer.HasMoreTokens() )
        return false;

    return FromXFontName(xFontName);
}

wxString wxNativeFontInfo::ToString() const
{
    // 0 is the version
    return wxString::Format("%d;%s", 0, GetXFontName().c_str());
}

bool wxNativeFontInfo::FromUserString(const wxString& s)
{
    return FromXFontName(s);
}

wxString wxNativeFontInfo::ToUserString() const
{
    return GetXFontName();
}

bool wxNativeFontInfo::HasElements() const
{
    // we suppose that the foundry is never empty, so if it is it means that we
    // had never parsed the XLFD
    return !fontElements[0].empty();
}

wxString wxNativeFontInfo::GetXFontComponent(wxXLFDField field) const
{
    wxCHECK_MSG( field < wxXLFD_MAX, {}, "invalid XLFD field" );

    if ( !HasElements() )
    {
        if ( !const_cast<wxNativeFontInfo *>(this)->FromXFontName(xFontName) )
            return {};
    }

    return fontElements[field];
}

bool wxNativeFontInfo::FromXFontName(const wxString& fontname)
{
    // TODO: we should be able to handle the font aliases here, but how?
    wxStringTokenizer tokenizer(fontname, "-");

    // skip the leading, usually empty field (font name registry)
    if ( !tokenizer.HasMoreTokens() )
        return false;

    (void)tokenizer.GetNextToken();

    for ( size_t n = 0; n < WXSIZEOF(fontElements); n++ )
    {
        if ( !tokenizer.HasMoreTokens() )
        {
            // not enough elements in the XLFD - or maybe an alias
            return false;
        }

        wxString field = tokenizer.GetNextToken();
        if ( !field.empty() && field != wxT('*') )
        {
            // we're really initialized now
            m_isDefault = false;
        }

        fontElements[n] = field;
    }

    // this should be all
    if ( tokenizer.HasMoreTokens() )
        return false;

    return true;
}

wxString wxNativeFontInfo::GetXFontName() const
{
    if ( xFontName.empty() )
    {
        for ( size_t n = 0; n < WXSIZEOF(fontElements); n++ )
        {
            // replace the non specified elements with '*' except for the
            // additional style which is usually just omitted
            wxString elt = fontElements[n];
            if ( elt.empty() && n != wxXLFD_ADDSTYLE )
            {
                elt = wxT('*');
            }

            const_cast<wxNativeFontInfo *>(this)->xFontName << wxT('-') << elt;
        }
    }

    return xFontName;
}

void
wxNativeFontInfo::SetXFontComponent(wxXLFDField field, const wxString& value)
{
    wxCHECK_RET( field < wxXLFD_MAX, "invalid XLFD field" );

    if ( !HasElements() )
    {
        for ( int field = 0; field < wxXLFD_MAX; field++ )
            fontElements[field] = '*';
    }

    fontElements[field] = value;

    // invalidate the XFLD, it doesn't correspond to the font elements any more
    xFontName.clear();
}

void wxNativeFontInfo::SetXFontName(const wxString& xFontName_)
{
    // invalidate the font elements, GetXFontComponent() will reparse the XLFD
    fontElements[0].clear();

    xFontName = xFontName_;

    m_isDefault = false;
}

double wxNativeFontInfo::GetFractionalPointSize() const
{
    const wxString s = GetXFontComponent(wxXLFD_POINTSIZE);

    // return -1 to indicate that the size is unknown
    long l;
    return s.ToLong(&l) ? l : -1;
}

wxFontStyle wxNativeFontInfo::GetStyle() const
{
    const wxString s = GetXFontComponent(wxXLFD_SLANT);

    if ( s.length() != 1 )
    {
        // it is really unknown but we don't have any way to return it from
        // here
        return wxFontStyle::Normal;
    }

    switch ( s[0].GetValue() )
    {
        default:
            // again, unknown but consider normal by default

        case wxT('r'):
            return wxFontStyle::Normal;

        case wxT('i'):
            return wxFontStyle::Italic;

        case wxT('o'):
            return wxFontStyle::Slant;
    }
}

int wxNativeFontInfo::GetNumericWeight() const
{
    const wxString weight = GetXFontComponent(wxXLFD_WEIGHT).MakeLower();
    if (weight == "thin") || weight == wxT("ultralight")
        return wxFONTWEIGHT_THIN;
    else if (weight == "extralight")
        return wxFONTWEIGHT_EXTRALIGHT;
    else if (weight == "light")
        return wxFONTWEIGHT_LIGHT;
    else if (weight == "book") || weight == wxT("semilight") || weight == wxT("demilight")
        return 350;
    else if (weight == "medium")
        return wxFONTWEIGHT_MEDIUM;
    else if (weight == "semibold") || weight == wxT("demibold")
        return wxFONTWEIGHT_SEMIBOLD;
    else if (weight == "bold")
        return wxFONTWEIGHT_BOLD;
    else if (weight == "extrabold")
        return wxFONTWEIGHT_EXTRABOLD;
    else if (weight == "heavy")
        return wxFONTWEIGHT_HEAVY;
    else if (weight == "extraheavy") || weight == wxT("black") || weight == wxT("ultrabold")
        return wxFONTWEIGHT_EXTRAHEAVY;

    return wxFONTWEIGHT_NORMAL;
}

bool wxNativeFontInfo::GetUnderlined() const
{
    // X fonts are never underlined
    return false;
}

wxString wxNativeFontInfo::GetFaceName() const
{
    // wxWidgets facename probably more accurately corresponds to X family
    return GetXFontComponent(wxXLFD_FAMILY);
}

wxFontFamily wxNativeFontInfo::GetFamily() const
{
    // and wxWidgets family -- to X foundry, but we have to translate it to
    // wxFontFamily somehow...
    wxFAIL_MSG("not implemented"); // GetXFontComponent(wxXLFD_FOUNDRY);

    return wxFontFamily::Default;
}

wxFontEncoding wxNativeFontInfo::GetEncoding() const
{
    // we already have the code for this but need to refactor it first
    wxFAIL_MSG( "not implemented" );

    return wxFONTENCODING_MAX;
}

void wxNativeFontInfo::SetFractionalPointSize(double pointsize)
{
    wxString s;
    if ( pointsize < 0 )
        s = '*';
    else
        s.Printf("%d", wxRound(10*pointsize));

    SetXFontComponent(wxXLFD_POINTSIZE, s);
}

void wxNativeFontInfo::SetStyle(wxFontStyle style)
{
    wxString s;
    switch ( style )
    {
        case wxFontStyle::Italic:
            s = wxT('i');
            break;

        case wxFontStyle::Slant:
            s = wxT('o');
            break;

        case wxFontStyle::Normal:
            s = wxT('r');
            break;

        default:
            wxFAIL_MSG( "unknown wxFontStyle in wxNativeFontInfo::SetStyle" );
            return;
    }

    SetXFontComponent(wxXLFD_SLANT, s);
}

void wxNativeFontInfo::SetNumericWeight(int weight)
{
    wxString s;
    switch ( wxFontInfo::GetWeightClosestToNumericValue(weight) )
    {
        case wxFONTWEIGHT_THIN:
            s = "thin";
            break;

        case wxFONTWEIGHT_EXTRALIGHT:
            s = "extralight";
            break;

        case wxFONTWEIGHT_LIGHT:
            s = "light";
            break;

        case wxFONTWEIGHT_NORMAL:
            s = "normal";
            break;

        case wxFONTWEIGHT_MEDIUM:
            s = "medium";
            break;

        case wxFONTWEIGHT_SEMIBOLD:
            s = "semibold";
            break;

        case wxFONTWEIGHT_BOLD:
            s = "bold";
            break;

        case wxFONTWEIGHT_EXTRABOLD:
            s = "extrabold";
            break;

        case wxFONTWEIGHT_HEAVY:
            s = "heavy";
            break;

        case wxFONTWEIGHT_EXTRAHEAVY:
            s = "extraheavy";
            break;

        case wxFONTWEIGHT_INVALID:
            wxFAIL_MSG( "Invalid font weight" );
            break;
    }

    wxCHECK_RET( !s.empty(), "unknown weight value" );

    SetXFontComponent(wxXLFD_WEIGHT, s);
}

void wxNativeFontInfo::SetUnderlined([[maybe_unused]] bool underlined)
{
    // can't do this under X
}

void wxNativeFontInfo::SetStrikethrough([[maybe_unused]] bool strikethrough)
{
    // this is not supported by Pango fonts neither
}

bool wxNativeFontInfo::SetFaceName(const wxString& facename)
{
    SetXFontComponent(wxXLFD_FAMILY, facename);
    return true;
}

void wxNativeFontInfo::SetFamily(wxFontFamily family)
{
    wxString xfamily;
    switch (family)
    {
        case wxFontFamily::Decorative: xfamily = "lucida"; break;
        case wxFontFamily::Roman:      xfamily = "times";  break;
        case wxFontFamily::Modern:     xfamily = "courier"; break;
        case wxFontFamily::Default:
        case wxFontFamily::Swiss:      xfamily = "helvetica"; break;
        case wxFontFamily::Teletype:   xfamily = "lucidatypewriter"; break;
        case wxFontFamily::Script:     xfamily = "utopia"; break;
        case wxFontFamily::Unknown:    break;
    }

    wxCHECK_RET( !xfamily.empty(), "Unknown wxFontFamily" );

    SetXFontComponent(wxXLFD_FAMILY, xfamily);
}

void wxNativeFontInfo::SetEncoding(wxFontEncoding encoding)
{
    wxNativeEncodingInfo info;
    if ( wxGetNativeFontEncoding(encoding, &info) )
    {
        SetXFontComponent(wxXLFD_ENCODING, info.xencoding);
        SetXFontComponent(wxXLFD_REGISTRY, info.xregistry);
    }
}

// ----------------------------------------------------------------------------
// common functions
// ----------------------------------------------------------------------------

bool wxGetNativeFontEncoding(wxFontEncoding encoding,
                             wxNativeEncodingInfo *info)
{
    wxCHECK_MSG( info, false, "bad pointer in wxGetNativeFontEncoding" );

    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        encoding = wxFont::GetDefaultEncoding();
    }

    switch ( encoding )
    {
        case wxFONTENCODING_ISO8859_1:
        case wxFONTENCODING_ISO8859_2:
        case wxFONTENCODING_ISO8859_3:
        case wxFONTENCODING_ISO8859_4:
        case wxFONTENCODING_ISO8859_5:
        case wxFONTENCODING_ISO8859_6:
        case wxFONTENCODING_ISO8859_7:
        case wxFONTENCODING_ISO8859_8:
        case wxFONTENCODING_ISO8859_9:
        case wxFONTENCODING_ISO8859_10:
        case wxFONTENCODING_ISO8859_11:
        case wxFONTENCODING_ISO8859_12:
        case wxFONTENCODING_ISO8859_13:
        case wxFONTENCODING_ISO8859_14:
        case wxFONTENCODING_ISO8859_15:
            {
                int cp = encoding - wxFONTENCODING_ISO8859_1 + 1;
                info->xregistry = "iso8859";
                info->xencoding.Printf("%d", cp);
            }
            break;

        case wxFONTENCODING_UTF8:
            info->xregistry = "iso10646";
            info->xencoding = "*";
            break;

        case wxFONTENCODING_GB2312:
            info->xregistry = "GB2312";   // or the otherway round?
            info->xencoding = "*";
            break;

        case wxFONTENCODING_KOI8:
        case wxFONTENCODING_KOI8_U:
            info->xregistry = "koi8";

            // we don't make distinction between koi8-r, koi8-u and koi8-ru (so far)
            info->xencoding = "*";
            break;

        case wxFONTENCODING_CP1250:
        case wxFONTENCODING_CP1251:
        case wxFONTENCODING_CP1252:
        case wxFONTENCODING_CP1253:
        case wxFONTENCODING_CP1254:
        case wxFONTENCODING_CP1255:
        case wxFONTENCODING_CP1256:
        case wxFONTENCODING_CP1257:
            {
                int cp = encoding - wxFONTENCODING_CP1250 + 1250;
                info->xregistry = "microsoft";
                info->xencoding.Printf("cp%d", cp);
            }
            break;

        case wxFONTENCODING_EUC_JP:
        case wxFONTENCODING_SHIFT_JIS:
            info->xregistry = "jis*";
            info->xencoding = "*";
            break;

        case wxFONTENCODING_SYSTEM:
            info->xregistry =
            info->xencoding = "*";
            break;

        default:
            // don't know how to translate this encoding into X fontspec
            return false;
    }

    info->encoding = encoding;

    return true;
}

bool wxTestFontEncoding(const wxNativeEncodingInfo& info)
{
    wxString fontspec;
    fontspec.Printf("-*-%s-*-*-*-*-*-*-*-*-*-*-%s-%s",
                    info.facename.empty() ? wxString("*") : info.facename,
                    info.xregistry,
                    info.xencoding);

    return wxTestFontSpec(fontspec);
}

// ----------------------------------------------------------------------------
// X-specific functions
// ----------------------------------------------------------------------------

wxNativeFont wxLoadQueryNearestFont(double pointSize,
                                    wxFontFamily family,
                                    wxFontStyle style,
                                    int weight,
                                    bool underlined,
                                    const wxString &facename,
                                    wxFontEncoding encoding,
                                    wxString* xFontName)
{
    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        encoding = wxFont::GetDefaultEncoding();
    }

    // first determine the encoding - if the font doesn't exist at all in this
    // encoding, it's useless to do all other approximations (i.e. size,
    // family &c don't matter much)
    wxNativeEncodingInfo info;
    if ( encoding == wxFONTENCODING_SYSTEM )
    {
        // This will always work so we don't test to save time
        wxGetNativeFontEncoding(wxFONTENCODING_SYSTEM, &info);
    }
    else
    {
        if ( !wxGetNativeFontEncoding(encoding, &info) ||
             !wxTestFontEncoding(info) )
        {
#if wxUSE_FONTMAP
            if ( !wxFontMapper::Get()->GetAltForEncoding(encoding, &info) )
#endif // wxUSE_FONTMAP
            {
                // unspported encoding - replace it with the default
                //
                // NB: we can't just return 0 from here because wxGTK code doesn't
                //     check for it (i.e. it supposes that we'll always succeed),
                //     so it would provoke a crash
                wxGetNativeFontEncoding(wxFONTENCODING_SYSTEM, &info);
            }
        }
    }

    // OK, we have the correct xregistry/xencoding in info structure
    wxNativeFont font = 0;

    // if we already have the X font name, try to use it
    if( xFontName && !xFontName->empty() )
    {
        //
        //  Make sure point size is correct for scale factor.
        //
        wxStringTokenizer tokenizer(*xFontName, "-", wxTOKEN_RET_DELIMS);
        wxString newFontName;

        for(int i = 0; i < 8; i++)
          newFontName += tokenizer.NextToken();

        (void) tokenizer.NextToken();

        newFontName += wxString::Format("%d-", pointSize);

        while(tokenizer.HasMoreTokens())
          newFontName += tokenizer.GetNextToken();

        font = wxLoadFont(newFontName);

        if(font)
          *xFontName = newFontName;
    }

    if ( !font )
    {
        // search up and down by stepsize 10
        int max_size = pointSize + 20 * (1 + (pointSize/180));
        int min_size = pointSize - 20 * (1 + (pointSize/180));

        int i, round; // counters

        // first round: search for equal, then for smaller and for larger size
        // with the given weight and style
        int testweight = weight;
        wxFontStyle teststyle = style;

        for ( round = 0; round < 3; round++ )
        {
            // second round: use normal weight
            if ( round == 1 )
            {
                if ( testweight != wxFONTWEIGHT_NORMAL )
                {
                    testweight = wxFONTWEIGHT_NORMAL;
                }
                else
                {
                    ++round; // fall through to third round
                }
            }

            // third round: ... and use normal style
            if ( round == 2 )
            {
                if ( teststyle != wxFontStyle::Normal )
                {
                    teststyle = wxFontStyle::Normal;
                }
                else
                {
                    break;
                }
            }
            // Search for equal or smaller size (approx.)
            for ( i = pointSize; !font && i >= 10 && i >= min_size; i -= 10 )
            {
                font = wxLoadQueryFont(i, family, teststyle, testweight, underlined,
                                   facename, info.xregistry, info.xencoding,
                                   xFontName);
            }

            // Search for larger size (approx.)
            for ( i = pointSize + 10; !font && i <= max_size; i += 10 )
            {
                font = wxLoadQueryFont(i, family, teststyle, testweight, underlined,
                                   facename, info.xregistry, info.xencoding,
                                   xFontName);
            }
        }

        // Try default family
        if ( !font && family != wxFontFamily::Default )
        {
            font = wxLoadQueryFont(pointSize, wxFontFamily::Default, style, weight,
                                   underlined, facename,
                                   info.xregistry, info.xencoding,
                                   xFontName );
        }

        // ignore size, family, style and weight but try to find font with the
        // given facename and encoding
        if ( !font )
        {
            font = wxLoadQueryFont(120, wxFontFamily::Default,
                                   wxFontStyle::Normal, wxFONTWEIGHT_NORMAL,
                                   underlined, facename,
                                   info.xregistry, info.xencoding,
                                   xFontName);

            // ignore family as well
            if ( !font )
            {
                font = wxLoadQueryFont(120, wxFontFamily::Default,
                                       wxFontStyle::Normal, wxFONTWEIGHT_NORMAL,
                                       underlined, {},
                                       info.xregistry, info.xencoding,
                                       xFontName);

                // if it still failed, try to get the font of any size but
                // with the requested encoding: this can happen if the
                // encoding is only available in one size which happens to be
                // different from 120
                if ( !font )
                {
                    font = wxLoadQueryFont(-1, wxFontFamily::Default,
                                           wxFontStyle::Normal, wxFONTWEIGHT_NORMAL,
                                           false, {},
                                           info.xregistry, info.xencoding,
                                           xFontName);

                    // this should never happen as we had tested for it in the
                    // very beginning, but if it does, do return something non
                    // NULL or we'd crash in wxFont code
                    if ( !font )
                    {
                        wxFAIL_MSG( "this encoding should be available!" );

                        font = wxLoadQueryFont(-1, wxFontFamily::Default,
                                               wxFontStyle::Normal, wxFONTWEIGHT_NORMAL,
                                               false, {},
                                               "*"), wxT("*",
                                               xFontName);
                    }
                }
            }
        }
    }

    return font;
}

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// returns true if there are any fonts matching this font spec
static bool wxTestFontSpec(const wxString& fontspec)
{
    // some X servers will fail to load this font because there are too many
    // matches so we must test explicitly for this
    if ( fontspec == "-*-*-*-*-*-*-*-*-*-*-*-*-*-*" )
    {
        return true;
    }

    wxNativeFont test = (wxNativeFont) g_fontHash->Get( fontspec );
    if (test)
    {
        return true;
    }

    test = wxLoadFont(fontspec);
    g_fontHash->Put( fontspec, (wxObject*) test );

    if ( test )
    {
        wxFreeFont(test);

        return true;
    }
    else
    {
        return false;
    }
}

static wxNativeFont wxLoadQueryFont(double pointSize,
                                    wxFontFamily family,
                                    wxFontStyle style,
                                    int weight,
                                    [[maybe_unused]] bool underlined,
                                    const wxString& facename,
                                    const wxString& xregistry,
                                    const wxString& xencoding,
                                    wxString* xFontName)
{
#if wxUSE_NANOX
    int xweight;
    switch (weight)
    {
         case wxFONTWEIGHT_BOLD:
             {
                 xweight = MWLF_WEIGHT_BOLD;
                 break;
             }
        case wxFONTWEIGHT_LIGHT:
             {
                 xweight = MWLF_WEIGHT_LIGHT;
                 break;
             }
         case wxFONTWEIGHT_NORMAL:
             {
                 xweight = MWLF_WEIGHT_NORMAL;
                 break;
             }

     default:
             {
                 xweight = MWLF_WEIGHT_DEFAULT;
                 break;
             }
    }
    GR_SCREEN_INFO screenInfo;
    GrGetScreenInfo(& screenInfo);

    int yPixelsPerCM = screenInfo.ydpcm;

    // A point is 1/72 of an inch.
    // An inch is 2.541 cm.
    // So pixelHeight = (pointSize / 72) (inches) * 2.541 (for cm) * yPixelsPerCM (for pixels)
    // In fact pointSize is 10 * the normal point size so
    // divide by 10.

    int pixelHeight = (int) ( (((float)pointSize) / 720.0) * 2.541 * (float) yPixelsPerCM) ;

    // An alternative: assume that the screen is 72 dpi.
    //int pixelHeight = (int) (((float)pointSize / 720.0) * 72.0) ;
    //int pixelHeight = (int) ((float)pointSize / 10.0) ;

    GR_LOGFONT logFont;
    logFont.lfHeight = pixelHeight;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfWeight = xweight;
    logFont.lfItalic = (style == wxFontStyle::Italic ? 0 : 1) ;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = MWLF_CHARSET_DEFAULT; // TODO: select appropriate one
    logFont.lfOutPrecision = MWLF_TYPE_DEFAULT;
    logFont.lfClipPrecision = 0; // Not used
    logFont.lfRoman = (family == wxFontFamily::Roman ? 1 : 0) ;
    logFont.lfSerif = (family == wxFontFamily::Swiss ? 0 : 1) ;
    logFont.lfSansSerif = !logFont.lfSerif ;
    logFont.lfModern = (family == wxFontFamily::Modern ? 1 : 0) ;
    logFont.lfProportional = (family == wxFontFamily::Teletype ? 0 : 1) ;
    logFont.lfOblique = 0;
    logFont.lfSmallCaps = 0;
    logFont.lfPitch = 0; // 0 = default
    strcpy(logFont.lfFaceName, facename.c_str());

    XFontStruct* fontInfo = (XFontStruct*) malloc(sizeof(XFontStruct));
    fontInfo->fid = GrCreateFont((GR_CHAR*) facename.c_str(), pixelHeight, & logFont);
    GrGetFontInfo(fontInfo->fid, & fontInfo->info);
    return (wxNativeFont) fontInfo;

#else
    wxNativeFontInfo info;
    info.SetFractionalPointSize(pointSize);

    if ( !facename.empty() )
    {
        info.SetFaceName(facename);
        if ( !wxTestFontSpec(info.GetXFontName()) )
        {
            // No such face name, use just the family (we assume this will
            // never fail).
            info.SetFamily(family);
        }
    }
    else
    {
        info.SetFamily(family);
    }

    wxNativeFontInfo infoWithStyle(info);
    infoWithStyle.SetStyle(style);
    if ( wxTestFontSpec(infoWithStyle.GetXFontName()) )
        info = infoWithStyle;

    wxNativeFontInfo infoWithWeight(info);
    infoWithWeight.SetNumericWeight(weight);
    if ( wxTestFontSpec(infoWithWeight.GetXFontName()) )
        info = infoWithWeight;

    // construct the X font spec from our data
    wxString fontSpec;
    fontSpec.Printf("-*-%s-%s-%s-normal-*-*-%s-*-*-*-*-%s-%s",
                    info.GetXFontComponent(wxXLFD_FAMILY),
                    info.GetXFontComponent(wxXLFD_WEIGHT),
                    info.GetXFontComponent(wxXLFD_SLANT),
                    info.GetXFontComponent(wxXLFD_POINTSIZE),
                    xregistry,
                    xencoding);

    if( xFontName )
        *xFontName = fontSpec;

    return wxLoadFont(fontSpec);
#endif
    // wxUSE_NANOX
}

// ----------------------------------------------------------------------------
// wxFontModule
// ----------------------------------------------------------------------------

class wxFontModule : public wxModule
{
public:
    bool OnInit();
    void OnExit();

private:
    wxDECLARE_DYNAMIC_CLASS(wxFontModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFontModule, wxModule);

bool wxFontModule::OnInit()
{
    g_fontHash = new wxHashTable( wxKEY_STRING );

    return true;
}

void wxFontModule::OnExit()
{
    wxDELETE(g_fontHash);
}

#endif // GTK 2.0/1.x
