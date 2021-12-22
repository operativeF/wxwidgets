///////////////////////////////////////////////////////////////////////////////
// Name:        tests/uris/url.cpp
// Purpose:     wxURL unit test
// Author:      Francesco Montorsi
// Created:     2009-5-31
// Copyright:   (c) 2009 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/url.h"
#include "wx/utils.h"

import WX.Cmn.MemStream;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

TEST_CASE("URL Tests")
{
    // FIXME: Call functions directly
    wxSocketBase::Initialize();

    SUBCASE("GetInputStream")
    {
        if (!IsNetworkAvailable())      // implemented in test.cpp
        {
            wxLogWarning("No network connectivity; skipping the URLTestCase::GetInputStream test unit.");
            return;
        }

        wxURL url("http://www.wxwidgets.org/assets/img/header-logo.png");
        CHECK_EQ(wxURLError::None, url.GetError());

        std::unique_ptr<wxInputStream> in_stream(url.GetInputStream());
        if ( !in_stream && IsAutomaticTest() )
        {
            // Sometimes the connection fails during CI runs, don't consider this
            // as a fatal error because it happens from time to time and there is
            // nothing we can do about it.
            WARN("Connection to www.wxwidgets.org failed, skipping the test.");
            return;
        }

        CHECK(in_stream);
        CHECK(in_stream->IsOk());

        wxMemoryOutputStream ostream;
        CHECK(in_stream->Read(ostream).GetLastError() == wxSTREAM_EOF);

        CHECK_EQ(17334, ostream.GetSize());
    }

    SUBCASE("CopyAndAssignment")
    {
        wxURL url1("http://www.example.org/");
        wxURL url2;
        wxURI *puri = &url2;        // downcast

        { // Copy constructor
            wxURL url3(url1);
            CHECK(url1 == url3);
        }
        { // Constructor for string
            wxURL url3(url1.GetURL());
            CHECK(url1 == url3);
        }
        { // 'Copy' constructor for uri
            wxURL url3(*puri);
            CHECK(url2 == url3);
        }

        // assignment for uri
        *puri = url1;
        CHECK(url1 == url2);

        // assignment to self through base pointer
        *puri = url2;

        // Assignment of string
        url1 = "http://www.example2.org/index.html";
        *puri = wxS("http://www.example2.org/index.html");
        CHECK(url1 == url2);

        // Assignment
        url1 = "";
        url2 = url1;
        CHECK(url1 == url2);

        // assignment to self
        url2 = url2;

        // check for destructor (with base pointer!)
        puri = new wxURL();
        delete puri;
    }

    wxSocketBase::Shutdown();
} // END URL Tests
