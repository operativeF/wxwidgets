/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/colourcmn.cpp
// Purpose:     wxColourBase implementation
// Author:      Francesco Montorsi
// Modified by:
// Created:     20/4/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#include "wx/colour.h"
#include "wx/log.h"
#include "wx/utils.h"
#include "wx/gdicmn.h"
#include "wx/wxcrtvararg.h"

#if wxUSE_VARIANT
IMPLEMENT_VARIANT_OBJECT_EXPORTED(wxColour,WXDLLEXPORT)
#endif


// ----------------------------------------------------------------------------
// XTI
// ----------------------------------------------------------------------------

#if wxUSE_EXTENDED_RTTI

import <cstring>;

template<> void wxStringReadValue(const wxString &s, wxColour &data )
{
    if ( !data.Set(s) )
    {
        wxLogError(_("String To Colour : Incorrect colour specification : %s"),
                   s.c_str() );
        data = wxNullColour;
    }
}

template<> void wxStringWriteValue(wxString &s, const wxColour &data )
{
    s = data.GetAsString(wxC2S_HTML_SYNTAX);
}

wxTO_STRING_IMP( wxColour )
wxFROM_STRING_IMP( wxColour )

wxIMPLEMENT_DYNAMIC_CLASS_WITH_COPY_AND_STREAMERS_XTI( wxColour, wxObject,  \
                                                      "wx/colour.h",  &wxTO_STRING( wxColour ), &wxFROM_STRING( wxColour ))
//WX_IMPLEMENT_ANY_VALUE_TYPE(wxAnyValueTypeImpl<wxColour>)
wxBEGIN_PROPERTIES_TABLE(wxColour)
wxREADONLY_PROPERTY( Red, unsigned char, Red, wxEMPTY_PARAMETER_VALUE, \
                    0 /*flags*/, "Helpstring"), wxT("group")
wxREADONLY_PROPERTY( Green, unsigned char, Green, wxEMPTY_PARAMETER_VALUE, \
                    0 /*flags*/, "Helpstring"), wxT("group")
wxREADONLY_PROPERTY( Blue, unsigned char, Blue, wxEMPTY_PARAMETER_VALUE, \
                    0 /*flags*/, "Helpstring"), wxT("group")
wxEND_PROPERTIES_TABLE()

wxDIRECT_CONSTRUCTOR_3( wxColour, unsigned char, Red, \
                       unsigned char, Green, unsigned char, Blue )

wxEMPTY_HANDLERS_TABLE(wxColour)
#else

#if wxCOLOUR_IS_GDIOBJECT
wxIMPLEMENT_DYNAMIC_CLASS(wxColour, wxGDIObject);
#else
wxIMPLEMENT_DYNAMIC_CLASS(wxColour, wxObject);
#endif

#endif

// ============================================================================
// wxString <-> wxColour conversions
// ============================================================================

bool wxColourBase::FromString(const wxString& str)
{
    if ( str.empty() )
        return false;       // invalid or empty string

    if ( wxStrnicmp(str, "RGB", 3) == 0 )
    {
        // CSS-like RGB specification
        // according to http://www.w3.org/TR/css3-color/#colorunits
        // values outside 0-255 range are allowed but should be clipped
        // TODO: Do these values actually change meaningfully?
        int red;
        int green;
        int blue;
        int alpha = wxALPHA_OPAQUE;

        if ( str.length() > 3 && (str[3] == wxT('a') || str[3] == wxT('A')) )
        {
            // We can't use sscanf() for the alpha value as sscanf() uses the
            // current locale while the floating point numbers in CSS always
            // use point as decimal separator, regardless of locale. So parse
            // the tail of the string manually by putting it in a buffer and
            // using wxString::ToCDouble() below. Notice that we can't use "%s"
            // for this as it stops at white space, so we use "[^)] )" to take
            // everything until the closing bracket.

            const unsigned len = str.length(); // always big enough
            wxCharBuffer alphaBuf(len);
            char * const alphaPtr = alphaBuf.data();

            for ( unsigned n = 0; n < len; n++ )
                alphaPtr[n] = '\0';

            // Construct the format string which ensures that the last argument
            // receives all the rest of the string.
            wxString formatStr;
            formatStr << "( %d , %d , %d , %" << len << "[^)] )";

            // Notice that we use sscanf() here because if the string is not
            // ASCII it can't represent a valid RGB colour specification anyhow
            // and like this we can be sure that %c corresponds to "char *"
            // while with wxSscanf() it depends on the type of the string
            // passed as first argument: if it is a wide string, then %c
            // expects "wchar_t *" matching parameter under MSW for example.
            if ( sscanf(str.c_str() + 4,
                        formatStr.mb_str(),
                        &red, &green, &blue, alphaPtr) != 4 )
                return false;

            // Notice that we must explicitly specify the length to get rid of
            // trailing NULs.
            wxString alphaStr(alphaPtr, wxStrlen(alphaPtr));
            if ( alphaStr.empty() )
                return false;

            alphaStr.Trim();

            double a{0.0};
            if ( !alphaStr.ToCDouble(&a) )
                return false;

            alpha = std::lround(a * 255);
        }
        else // no 'a' following "rgb"
        {
            if ( wxSscanf(str.wx_str() + 3, "( %d , %d , %d )",
                                                &red, &green, &blue) != 3 )
                return false;
        }

        Set((unsigned char)std::clamp(red, 0, 255),
            (unsigned char)std::clamp(green, 0, 255),
            (unsigned char)std::clamp(blue, 0, 255),
            (unsigned char)std::clamp(alpha, 0, 255));
    }
    else if ( str[0] == wxT('#') )
    {
        // hexadecimal prefixed with # ("HTML syntax")
        // see https://drafts.csswg.org/css-color/#hex-notation
        unsigned long tmp{0};
        if (wxSscanf(str.wx_str() + 1, "%lx", &tmp) != 1)
            return false;

        switch (wxStrlen(str) - 1)
        {
            case 6: // #rrggbb
                tmp = (tmp << 8) + wxALPHA_OPAQUE;
                [[fallthrough]];

            case 8: // #rrggbbaa
                Set((unsigned char)((tmp >> 24) & 0xFFU),
                    (unsigned char)((tmp >> 16) & 0xFFU),
                    (unsigned char)((tmp >> 8)  & 0xFFU),
                    (unsigned char)( tmp        & 0xFFU));
                break;

            case 3: // #rgb
                tmp = (tmp << 4) + 0xF;
                [[fallthrough]];

            case 4: // #rgba
                Set((unsigned char)(((tmp >> 12) & 0xFU) * 0x11U),
                    (unsigned char)(((tmp >> 8)  & 0xFU) * 0x11U),
                    (unsigned char)(((tmp >> 4)  & 0xFU) * 0x11U),
                    (unsigned char)(( tmp        & 0xFU) * 0x11U));
                break;

            default:
                return false; // unrecognized
        }
    }
    else if (wxTheColourDatabase) // a colour name ?
    {
        // we can't do
        // *this = wxTheColourDatabase->Find(str)
        // because this place can be called from constructor
        // and 'this' could not be available yet
        wxColour clr = wxTheColourDatabase->Find(str);
        if (!clr.IsOk())
            return false;

        Set((unsigned char)clr.Red(),
            (unsigned char)clr.Green(),
            (unsigned char)clr.Blue());
    }
    else // unrecognized
    {
        return false;
    }

    return true;
}

wxString wxColourBase::GetAsString(unsigned int flags) const
{
    if ( !IsOk() )
        return {};

    wxString colName;

    if ( IsSolid() )
    {
        const int alpha = Alpha();
        const bool isOpaque = alpha == wxALPHA_OPAQUE;

        // we can't use the name format if the colour is not opaque as the alpha
        // information would be lost
        if ( (flags & wxC2S_NAME) && isOpaque )
        {
            colName = wxTheColourDatabase->FindName(
                        static_cast<const wxColour &>(*this)).MakeLower();
        }

        if ( colName.empty() )
        {
            const int red = Red(),
                      green = Green(),
                      blue = Blue();

            if ( flags & wxC2S_CSS_SYNTAX )
            {
                // no name for this colour; return it in CSS syntax
                if ( isOpaque )
                {
                    colName.Printf("rgb(%d, %d, %d)", red, green, blue);
                }
                else // use rgba() form
                {
                    colName.Printf("rgba(%d, %d, %d, %s)",
                                   red, green, blue,
                                   wxString::FromCDouble(alpha / 255., 3));
                }
            }
            else if ( flags & wxC2S_HTML_SYNTAX )
            {
                // no name for this colour; return it in HTML syntax
                if ( isOpaque )
                    colName.Printf("#%02X%02X%02X", red, green, blue);
                else
                    colName.Printf("#%02X%02X%02X%02X", red, green, blue, alpha);
            }
        }
    }
    else
    {
        if ( flags & wxC2S_CSS_SYNTAX )
        {
            colName = "rgb(??, ??, \?\?)"; // \? form to avoid ??) trigraph
        }
        else if ( flags & wxC2S_HTML_SYNTAX )
        {
            colName = "#??????";
        }
    }

    // this function should alway returns a non-empty string
    wxASSERT_MSG(!colName.empty(),
                 "Invalid wxColour -> wxString conversion flags");

    return colName;
}

double wxColourBase::GetLuminance() const
{
    return (0.299*Red() + 0.587*Green() + 0.114*Blue()) / 255.0;
}

// static
void wxColourBase::MakeMono(unsigned char* r, unsigned char* g, unsigned char* b,
                            bool on)
{
    *r = *g = *b = on ? 255 : 0;
}

// static
void wxColourBase::MakeGrey(unsigned char* r, unsigned char* g, unsigned char* b
                            /*, unsigned char brightness */
                           )
{
    *r = *g = *b = (wxByte)(((*b)*117UL + (*g)*601UL + (*r)*306UL) >> 10);
}

// static
void wxColourBase::MakeGrey(unsigned char* r, unsigned char* g, unsigned char* b,
                            double weight_r, double weight_g, double weight_b)
{
    double luma = (*r) * weight_r + (*g) * weight_g + (*b) * weight_b;
    *r = *g = *b = (wxByte)std::lround(luma);
}

// static
void wxColourBase::MakeDisabled(unsigned char* r, unsigned char* g, unsigned char* b,
                                unsigned char brightness)
{
    //MakeGrey(r, g, b, brightness); // grey no-blend version
    *r = AlphaBlend(*r, brightness, 0.4);
    *g = AlphaBlend(*g, brightness, 0.4);
    *b = AlphaBlend(*b, brightness, 0.4);
}

wxColour& wxColourBase::MakeDisabled(unsigned char brightness)
{
    unsigned char r = Red(),
                  g = Green(),
                  b = Blue();
    MakeDisabled(&r, &g, &b, brightness);
    Set(r, g, b, Alpha());
    return static_cast<wxColour&>(*this);
}

// AlphaBlend is used by ChangeLightness and MakeDisabled

// static
unsigned char wxColourBase::AlphaBlend(unsigned char fg, unsigned char bg,
                                       double alpha)
{
    double result = bg + (alpha * (fg - bg));

    return (unsigned char)std::clamp(result, 0.0, 255.0);
}

// ChangeLightness() is a utility function that simply darkens
// or lightens a color, based on the specified percentage
// ialpha of 0 would be completely black, 100 completely white
// an ialpha of 100 returns the same colour

// static
void wxColourBase::ChangeLightness(unsigned char* r, unsigned char* g, unsigned char* b,
                                   int ialpha)
{
    if (ialpha == 100) return;

    // ialpha is 0..200 where 0 is completely black
    // and 200 is completely white and 100 is the same
    // convert that to normal alpha 0.0 - 1.0
    ialpha = std::clamp(ialpha, 0, 200);

    double alpha = ((double)(ialpha - 100.0))/100.0;

    unsigned char bg;
    if (ialpha > 100)
    {
        // blend with white
        bg = 255;
        alpha = 1.0 - alpha;  // 0 = transparent fg; 1 = opaque fg
    }
    else
    {
        // blend with black
        bg = 0;
        alpha = 1.0 + alpha;  // 0 = transparent fg; 1 = opaque fg
    }

    *r = AlphaBlend(*r, bg, alpha);
    *g = AlphaBlend(*g, bg, alpha);
    *b = AlphaBlend(*b, bg, alpha);
}

wxColour wxColourBase::ChangeLightness(int ialpha) const
{
    wxByte r = Red();
    wxByte g = Green();
    wxByte b = Blue();
    ChangeLightness(&r, &g, &b, ialpha);
    return {r, g, b};
}

// wxColour <-> wxString utilities, used by wxConfig
wxString wxToString(const wxColourBase& col)
{
    return col.IsOk() ? col.GetAsString(wxC2S_CSS_SYNTAX)
                      : wxString();
}

bool wxFromString(const wxString& str, wxColourBase *col)
{
    wxCHECK_MSG( col, false, "NULL output parameter" );

    if ( str.empty() )
    {
        *col = wxNullColour;
        return true;
    }

    return col->Set(str);
}


