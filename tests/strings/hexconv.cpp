///////////////////////////////////////////////////////////////////////////////
// Name:        tests/strings/hexconv.cpp
// Purpose:     wxDecToHex, wxHexToDec unit test
// Author:      Artur Wieczorek
// Created:     2017-02-23
// Copyright:   (c) 2017 wxWidgets development team
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#include "wx/utils.h"

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

TEST_CASE("Hex conversion")
{

    SUBCASE("DecToHex1")
    {
        // Conversion to wxString
        for ( int i = 0; i < 256; i++ )
        {
            char szHexStrRef[16];
            sprintf(szHexStrRef, "%02X", i);
            wxString hexStrRef = wxString(szHexStrRef);

            wxString hexStr = wxDecToHex(i);

            CHECK_EQ( hexStr, hexStrRef );
        }
    }

    SUBCASE("DecToHex2")
    {
        // Conversion to wxChar string
        for ( int i = 0; i < 256; i++ )
        {
            char szHexStrRef[16];
            sprintf(szHexStrRef, "%02X", i);

            wxChar hexStr[4];
            memset(hexStr, 0xFF, sizeof(hexStr));
            wxChar c3 = hexStr[3]; // This character should remain untouched
            wxDecToHex(i, hexStr);

            CHECK_EQ( hexStr[0], (wxChar)szHexStrRef[0] );
            CHECK_EQ( hexStr[1], (wxChar)szHexStrRef[1] );
            CHECK_EQ( hexStr[2], (wxChar)'\0' );
            CHECK_EQ( hexStr[3], c3 );
        }
    }

    SUBCASE("DecToHex3")
    {
        // Conversion to 2 characters
        for ( int i = 0; i < 256; i++ )
        {
            char szHexStrRef[16];
            sprintf(szHexStrRef, "%02X", i);

            char c1 = '\xFF';
            char c2 = '\xFF';
            wxDecToHex(i, &c1, &c2);

            CHECK_EQ( c1, szHexStrRef[0] );
            CHECK_EQ( c2, szHexStrRef[1] );
        }
    }

    SUBCASE("HexToDec1")
    {
        // Conversion from char string
        for ( int i = 0; i < 256; i++ )
        {
            char szHexStr[16];
            sprintf(szHexStr, "%02X", i);

            int n = wxHexToDec(szHexStr);
            CHECK_EQ( n, i );
        }
    }

    SUBCASE("HexToDec2")
    {
        // Conversion from wxString
        for ( int i = 0; i < 256; i++ )
        {
            char szHexStr[16];
            sprintf(szHexStr, "%02X", i);
            wxString hexStr = wxString(szHexStr);

            int n = wxHexToDec(hexStr);
            CHECK_EQ( n, i );
        }
    }

}
