///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/htmllboxtest.cpp
// Purpose:     wxSimpleHtmlListBoxNameStr unit test
// Author:      Vadim Zeitlin
// Created:     2010-11-27
// Copyright:   (c) 2010 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_HTML


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/htmllbox.h"
#include "itemcontainertest.h"

using HTMLBoxTest = ItemContainerTest<wxSimpleHtmlListBox>;

TEST_CASE_FIXTURE(HTMLBoxTest, "HTML List Box test")
{
    m_container = std::make_unique<wxSimpleHtmlListBox>(
        wxTheApp->GetTopWindow(), wxID_ANY);

    //wxITEM_CONTAINER_TESTS();
}

#endif //wxUSE_HTML
