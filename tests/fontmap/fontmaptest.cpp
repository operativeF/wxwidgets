///////////////////////////////////////////////////////////////////////////////
// Name:        tests/fontmap/fontmap.cpp
// Purpose:     wxFontMapper unit test
// Author:      Vadim Zeitlin
// Created:     14.02.04
// Copyright:   (c) 2003 TT-Solutions
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_FONTMAP

#include "wx/fontmap.h"

import Utils.Strings;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------


TEST_CASE("Names and Descriptions")
{
    static const char* charsets[] =
    {
        // some valid charsets
        "us-ascii"    ,
        "iso8859-1"   ,
        "iso-8859-12" ,
        "koi8-r"      ,
        "utf-7"       ,
        "cp1250"      ,
        "windows-1252",

        // and now some bogus ones
        ""            ,
        "cp1249"      ,
        "iso--8859-1" ,
        "iso-8859-19" ,
    };

    static const char* names[] =
    {
        // some valid charsets
        "default"     ,
        "iso-8859-1"  ,
        "iso-8859-12" ,
        "koi8-r"      ,
        "utf-7"       ,
        "windows-1250",
        "windows-1252",

        // and now some bogus ones
        "default"     ,
        "unknown--1"  ,
        "unknown--1"  ,
        "unknown--1"  ,
    };

    static const char* descriptions[] =
    {
        // some valid charsets
        "Default encoding"                  ,
        "Western European (ISO-8859-1)"     ,
        "Indian (ISO-8859-12)"              ,
        "KOI8-R"                            ,
        "Unicode 7 bit (UTF-7)"             ,
        "Windows Central European (CP 1250)",
        "Windows Western European (CP 1252)",

        // and now some bogus ones
        "Default encoding"                  ,
        "Unknown encoding (-1)"             ,
        "Unknown encoding (-1)"             ,
        "Unknown encoding (-1)"             ,
    };

    wxFontMapperBase& fmap = *wxFontMapperBase::Get();
    for ( size_t n = 0; n < WXSIZEOF(charsets); n++ )
    {
        wxFontEncoding enc = fmap.CharsetToEncoding(charsets[n]);
        CHECK_EQ( names[n], wx::utils::ToLowerCopy(fmap.GetEncodingName(enc)) );
        CHECK_EQ( descriptions[n], fmap.GetEncodingDescription(enc) );
    }
}

#endif // wxUSE_FONTMAP
