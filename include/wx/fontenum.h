/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fontenum.h
// Purpose:     wxFontEnumerator class for getting available fonts
// Author:      Julian Smart, Vadim Zeitlin
// Modified by: extended to enumerate more than just font facenames and works
//              not only on Windows now (VZ)
// Created:     04/01/98
// Copyright:   (c) Julian Smart, Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONTENUM_H_
#define _WX_FONTENUM_H_

#if wxUSE_FONTENUM

import WX.Base.FontEnc;

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// wxFontEnumerator enumerates all available fonts on the system or only the
// fonts with given attributes
// ----------------------------------------------------------------------------

class wxFontEnumerator
{
public:
    // virtual dtor for the base class
    virtual ~wxFontEnumerator() = default;

    wxFontEnumerator& operator=(wxFontEnumerator&&) = delete;

    // start enumerating font facenames (either all of them or those which
    // support the given encoding) - will result in OnFacename() being
    // called for each available facename (until they are exhausted or
    // OnFacename returns false)
    virtual bool EnumerateFacenames
                 (
                    wxFontEncoding encoding = wxFONTENCODING_SYSTEM, // all
                    bool fixedWidthOnly = false
                 );

    // enumerate the different encodings either for given font facename or for
    // all facenames - will result in OnFontEncoding() being called for each
    // available (facename, encoding) couple
    virtual bool EnumerateEncodings(const std::string& facename = {});

    // callbacks which are called after one of EnumerateXXX() functions from
    // above is invoked - all of them may return false to stop enumeration or
    // true to continue with it

    // called by EnumerateFacenames
    virtual bool OnFacename([[maybe_unused]] const std::string& facename)
        { return true; }

    // called by EnumerateEncodings
    virtual bool OnFontEncoding([[maybe_unused]] const std::string& facename,
                                [[maybe_unused]] const std::string& encoding)
        { return true; }



    // convenience function that returns array of facenames.
    static std::vector<std::string>
    GetFacenames(wxFontEncoding encoding = wxFONTENCODING_SYSTEM, // all
                 bool fixedWidthOnly = false);

    // convenience function that returns array of all available encodings.
    static std::vector<std::string> GetEncodings(const std::string& facename = {});

    // convenience function that returns true if the given face name exist
    // in the user's system
    static bool IsValidFacename(const std::string &str);

    // Invalidate cache used by some of the methods of this class internally.
    // This should be called if the list of the fonts available on the system
    // changes, for whatever reason.
    static void InvalidateCache();

private:
#ifdef wxHAS_UTF8_FONTS
    // helper for ports that only use UTF-8 encoding natively
    bool EnumerateEncodingsUTF8(const std::string& facename);
#endif
};

#endif // wxUSE_FONTENUM

#endif // _WX_FONTENUM_H_
