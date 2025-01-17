/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontenumcmn.cpp
// Purpose:     wxFontEnumerator class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     7/5/2006
// Copyright:   (c) 1999-2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FONTENUM

#include "wx/module.h"
#include "wx/fontenum.h"

import Utils.Strings;

import <string>;
import <vector>;

namespace
{

// Cached result of GetFacenames().
std::vector<std::string> gs_allFacenames;

// Module used to ensure the cache is cleared on library shutdown and so is not
// reused if it re-initialized again later.
class wxFontEnumCacheCleanupModule : public wxModule
{
public:
    bool OnInit() override { return true; }
    void OnExit() override { gs_allFacenames.clear(); }

private:
    wxDECLARE_DYNAMIC_CLASS(wxFontEnumCacheCleanupModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFontEnumCacheCleanupModule, wxModule);

} // anonymous namespace

// ============================================================================
// implementation
// ============================================================================

// A simple wxFontEnumerator which doesn't perform any filtering and
// just returns all facenames and encodings found in the system
class wxSimpleFontEnumerator : public wxFontEnumerator
{
public:
    // called by EnumerateFacenames
    bool OnFacename(const std::string& facename) override
    {
        m_arrFacenames.push_back(facename);
        return true;
    }

    // called by EnumerateEncodings
    bool OnFontEncoding([[maybe_unused]] const std::string& facename,
                        const std::string& encoding) override
    {
        m_arrEncodings.push_back(encoding);
        return true;
    }

    std::vector<std::string> m_arrFacenames;
    std::vector<std::string> m_arrEncodings;
};


/* static */
std::vector<std::string> wxFontEnumerator::GetFacenames(wxFontEncoding encoding, bool fixedWidthOnly)
{
    wxSimpleFontEnumerator temp;
    temp.EnumerateFacenames(encoding, fixedWidthOnly);
    return temp.m_arrFacenames;
}

/* static */
std::vector<std::string> wxFontEnumerator::GetEncodings(const std::string& facename)
{
    wxSimpleFontEnumerator temp;
    temp.EnumerateEncodings(facename);
    return temp.m_arrEncodings;
}

/* static */
bool wxFontEnumerator::IsValidFacename(const std::string &facename)
{
    // we cache the result of wxFontEnumerator::GetFacenames supposing that
    // the array of face names won't change in the session of this program
    if ( gs_allFacenames.empty() )
        gs_allFacenames = wxFontEnumerator::GetFacenames();

#ifdef __WXMSW__
    // Quoting the MSDN:
    //     "MS Shell Dlg is a mapping mechanism that enables
    //     U.S. English Microsoft Windows NT, and Microsoft Windows 2000 to
    //     support locales that have characters that are not contained in code
    //     page 1252. It is not a font but a face name for a nonexistent font."
    // Thus we need to consider "Ms Shell Dlg" and "Ms Shell Dlg 2" as valid
    // font face names even if they are not enumerated by wxFontEnumerator
    if (wx::utils::IsSameAsNoCase(facename, "Ms Shell Dlg") ||
        wx::utils::IsSameAsNoCase(facename, "Ms Shell Dlg 2"))
        return true;
#endif

    // is given font face name a valid one ?
    return gs_allFacenames.end() != std::ranges::find_if(gs_allFacenames,
        [=](const auto& other)
        {
            return wx::utils::IsSameAsNoCase(facename, other);
        });
}

/* static */
void wxFontEnumerator::InvalidateCache()
{
    gs_allFacenames.clear();
}

#ifdef wxHAS_UTF8_FONTS
bool wxFontEnumerator::EnumerateEncodingsUTF8(const std::string& facename)
{
    // name of UTF-8 encoding: no need to use wxFontMapper for it as it's
    // unlikely to change
    const std::string utf8("UTF-8");

    // all fonts are in UTF-8 only if this code is used
    if ( !facename.empty() )
    {
        OnFontEncoding(facename, utf8);
        return true;
    }

    // so enumerating all facenames supporting this encoding is the same as
    // enumerating all facenames
    const std::vector<std::string> facenames(GetFacenames(wxFONTENCODING_UTF8));
    const size_t count = facenames.size();
    if ( !count )
        return false;

    for ( size_t n = 0; n < count; n++ )
    {
        if ( !OnFontEncoding(facenames[n], utf8) )
            break;
    }

    return true;
}
#endif // wxHAS_UTF8_FONTS

#endif // wxUSE_FONTENUM
