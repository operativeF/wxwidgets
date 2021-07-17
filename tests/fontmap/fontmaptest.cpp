///////////////////////////////////////////////////////////////////////////////
// Name:        tests/fontmap/fontmap.cpp
// Purpose:     wxFontMapper unit test
// Author:      Vadim Zeitlin
// Created:     14.02.04
// Copyright:   (c) 2003 TT-Solutions
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#if wxUSE_FONTMAP

#include "wx/fontmap.h"

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------


TEST_CASE("Names and Descriptions")
{
    static const wxChar *charsets[] =
    {
        // some valid charsets
        wxT("us-ascii"    ),
        wxT("iso8859-1"   ),
        wxT("iso-8859-12" ),
        wxT("koi8-r"      ),
        wxT("utf-7"       ),
        wxT("cp1250"      ),
        wxT("windows-1252"),

        // and now some bogus ones
        wxT(""            ),
        wxT("cp1249"      ),
        wxT("iso--8859-1" ),
        wxT("iso-8859-19" ),
    };

    static const wxChar *names[] =
    {
        // some valid charsets
        wxT("default"     ),
        wxT("iso-8859-1"  ),
        wxT("iso-8859-12" ),
        wxT("koi8-r"      ),
        wxT("utf-7"       ),
        wxT("windows-1250"),
        wxT("windows-1252"),

        // and now some bogus ones
        wxT("default"     ),
        wxT("unknown--1"  ),
        wxT("unknown--1"  ),
        wxT("unknown--1"  ),
    };

    static const wxChar *descriptions[] =
    {
        // some valid charsets
        wxT("Default encoding"                  ),
        wxT("Western European (ISO-8859-1)"     ),
        wxT("Indian (ISO-8859-12)"              ),
        wxT("KOI8-R"                            ),
        wxT("Unicode 7 bit (UTF-7)"             ),
        wxT("Windows Central European (CP 1250)"),
        wxT("Windows Western European (CP 1252)"),

        // and now some bogus ones
        wxT("Default encoding"                  ),
        wxT("Unknown encoding (-1)"             ),
        wxT("Unknown encoding (-1)"             ),
        wxT("Unknown encoding (-1)"             ),
    };

    wxFontMapperBase& fmap = *wxFontMapperBase::Get();
    for ( size_t n = 0; n < WXSIZEOF(charsets); n++ )
    {
        wxFontEncoding enc = fmap.CharsetToEncoding(charsets[n]);
        CHECK_EQ( names[n], fmap.GetEncodingName(enc).Lower() );
        CHECK_EQ( descriptions[n], fmap.GetEncodingDescription(enc) );
    }
}

#endif // wxUSE_FONTMAP
