///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/fontenum.cpp
// Purpose:     wxFontEnumerator class for Windows
// Author:      Julian Smart
// Modified by: Vadim Zeitlin to add support for font encodings
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FONTENUM

#include "wx/msw/private.h"

#include "wx/fontenum.h"

#include "wx/gdicmn.h"
#include "wx/font.h"

#include "wx/encinfo.h"
#include "wx/fontutil.h"
#include "wx/fontmap.h"

import WX.WinDef;
import WX.Win.UniqueHnd;

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// the helper class which calls ::EnumFontFamilies() and whose OnFont() is
// called from the callback passed to this function and, in its turn, calls the
// appropariate wxFontEnumerator method
class wxFontEnumeratorHelper
{
public:
    explicit wxFontEnumeratorHelper(wxFontEnumerator *fontEnum);

    wxFontEnumeratorHelper(const wxFontEnumeratorHelper&) = delete;
	wxFontEnumeratorHelper& operator=(const wxFontEnumeratorHelper&) = delete;

    // control what exactly are we enumerating
        // we enumerate fonts with the given encoding
    bool SetEncoding(wxFontEncoding encoding);
        // we enumerate fixed-width fonts
    void SetFixedOnly(bool fixedOnly) { m_fixedOnly = fixedOnly; }
        // we enumerate the encodings this font face is available in
    void SetFaceName(const std::string& facename);

    // call to start enumeration
    void DoEnumerate();

    // called by our font enumeration proc
    bool OnFont(const LPLOGFONT lf, const LPTEXTMETRIC tm) const;

private:
    // if not empty, enum only the fonts with this facename
    std::string m_facename;

    // the list of charsets we already found while enumerating charsets
    std::vector<int> m_charsets;

    // the list of facenames we already found while enumerating facenames
    std::vector<std::string> m_facenames;

    // the object we forward calls to OnFont() to
    wxFontEnumerator *m_fontEnum;

    // if != -1, enum only fonts which have this encoding
    int m_charset{DEFAULT_CHARSET};

    // if true, enum only fixed fonts
    bool m_fixedOnly{false};

    // if true, we enumerate the encodings, not fonts
    bool m_enumEncodings{false};
};

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

int CALLBACK wxFontEnumeratorProc(LPLOGFONT lplf, LPTEXTMETRIC lptm,
                                  WXDWORD dwStyle, WXLPARAM lParam);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxFontEnumeratorHelper
// ----------------------------------------------------------------------------

wxFontEnumeratorHelper::wxFontEnumeratorHelper(wxFontEnumerator *fontEnum)
    : m_fontEnum(fontEnum)
{
}

void wxFontEnumeratorHelper::SetFaceName(const std::string& facename)
{
    m_enumEncodings = true;
    m_facename = facename;
}

bool wxFontEnumeratorHelper::SetEncoding(wxFontEncoding encoding)
{
    if ( encoding != wxFONTENCODING_SYSTEM )
    {
        wxNativeEncodingInfo info;
        if ( !wxGetNativeFontEncoding(encoding, &info) )
        {
#if wxUSE_FONTMAP
            if ( !wxFontMapper::Get()->GetAltForEncoding(encoding, &info) )
#endif // wxUSE_FONTMAP
            {
                // no such encodings at all
                return false;
            }
        }

        m_charset = info.charset;
        m_facename = info.facename;
    }

    return true;
}

void wxFontEnumeratorHelper::DoEnumerate()
{
    msw::utils::unique_dcwnd hDC{::GetDC(nullptr)};

    LOGFONTW lf
    {
        .lfCharSet = static_cast<BYTE>(m_charset),
        .lfPitchAndFamily = 0
    };

    auto wideFaceName = boost::nowide::widen(m_facename);
    wxStrlcpy(lf.lfFaceName, wideFaceName.c_str(), WXSIZEOF(lf.lfFaceName));
    ::EnumFontFamiliesExW(hDC.get(), &lf, (FONTENUMPROCW)wxFontEnumeratorProc,
                         (WXLPARAM)this, wxRESERVED_PARAM) ;
}

bool wxFontEnumeratorHelper::OnFont(const LPLOGFONT lf,
                                    const LPTEXTMETRIC tm) const
{
    if ( m_enumEncodings )
    {
        // is this a new charset?
        int cs = lf->lfCharSet;
        if (std::ranges::find(m_charsets, cs) == m_charsets.end())
        {
            const_cast<wxFontEnumeratorHelper *>(this)->m_charsets.push_back(cs);

#if wxUSE_FONTMAP
            wxFontEncoding enc = wxGetFontEncFromCharSet(cs);
            return m_fontEnum->OnFontEncoding(boost::nowide::narrow(lf->lfFaceName),
                                              wxFontMapper::GetEncodingName(enc));
#else // !wxUSE_FONTMAP
            // Just use some unique and, hopefully, understandable, name.
            return m_fontEnum->OnFontEncoding
                               (
                                lf->lfFaceName,
                                wxString::Format("Code page %d", cs)
                               );
#endif // wxUSE_FONTMAP/!wxUSE_FONTMAP
        }
        else
        {
            // continue enumeration
            return true;
        }
    }

    if ( m_fixedOnly )
    {
        // check that it's a fixed pitch font (there is *no* error here, the
        // flag name is misleading!)
        if ( tm->tmPitchAndFamily & TMPF_FIXED_PITCH )
        {
            // not a fixed pitch font
            return true;
        }
    }

    if ( m_charset != DEFAULT_CHARSET )
    {
        // check that we have the right encoding
        if ( lf->lfCharSet != m_charset )
        {
            return true;
        }
    }
    else // enumerating fonts in all charsets
    {
        // we can get the same facename twice or more in this case because it
        // may exist in several charsets but we only want to return one copy of
        // it (note that this can't happen for m_charset != DEFAULT_CHARSET)
        if(std::ranges::find(m_facenames, boost::nowide::narrow(lf->lfFaceName)) != m_facenames.cend())
        {
            // continue enumeration
            return true;
        }

        const_cast<wxFontEnumeratorHelper *>(this)->
            m_facenames.push_back(boost::nowide::narrow(lf->lfFaceName));
    }

    return m_fontEnum->OnFacename(boost::nowide::narrow(lf->lfFaceName));
}

// ----------------------------------------------------------------------------
// wxFontEnumerator
// ----------------------------------------------------------------------------

bool wxFontEnumerator::EnumerateFacenames(wxFontEncoding encoding,
                                          bool fixedWidthOnly)
{
    wxFontEnumeratorHelper fe(this);
    if ( fe.SetEncoding(encoding) )
    {
        fe.SetFixedOnly(fixedWidthOnly);

        fe.DoEnumerate();
    }
    // else: no such fonts, unknown encoding

    return true;
}

bool wxFontEnumerator::EnumerateEncodings(const std::string& facename)
{
    wxFontEnumeratorHelper fe(this);
    fe.SetFaceName(facename);
    fe.DoEnumerate();

    return true;
}

// ----------------------------------------------------------------------------
// Windows callbacks
// ----------------------------------------------------------------------------

int CALLBACK wxFontEnumeratorProc(LPLOGFONT lplf, LPTEXTMETRIC lptm,
                                  [[maybe_unused]] WXDWORD dwStyle, WXLPARAM lParam)
{

    // we used to process TrueType fonts only, but there doesn't seem to be any
    // reasons to restrict ourselves to them here
#if 0
    // Get rid of any fonts that we don't want...
    if ( dwStyle != TRUETYPE_FONTTYPE )
    {
        // continue enumeration
        return TRUE;
    }
#endif // 0

    wxFontEnumeratorHelper *fontEnum = (wxFontEnumeratorHelper *)lParam;

    return fontEnum->OnFont(lplf, lptm);
}

#endif // wxUSE_FONTENUM
