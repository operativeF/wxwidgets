///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/bitmapcomboboxtest.cpp
// Purpose:     wxBitmapComboBox unit test
// Author:      Steven Lamerton
// Created:     2010-07-15
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_BITMAPCOMBOBOX

#include "wx/app.h"

#include "wx/bmpcbox.h"
#include "wx/artprov.h"

#include "textentrytest.h"
#include "itemcontainertest.h"
#include "asserthelper.h"

import WX.Test.Prec;

using BitmapComboBoxTest = ItemContainerTest<wxBitmapComboBox>;

TEST_CASE_FIXTURE(BitmapComboBoxTest, "Bitmap combobox test")
{
    m_container = std::make_unique<wxBitmapComboBox>(wxTheApp->GetTopWindow(),
                                                     wxID_ANY);

    SUBCASE("Bitmap")
    {
        const std::vector<wxString> items = { "item 0", "item 1" };

        for( const auto& item : items )
            m_container->Append(item);

        CHECK(!m_container->GetItemBitmap(0).IsOk());

        wxBitmap bitmap = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER,
                                                 wxSize(16, 16));

        m_container->Append("item with bitmap", bitmap);

        CHECK(m_container->GetItemBitmap(2).IsOk());

        m_container->Insert("item with bitmap", bitmap, 1);

        CHECK(m_container->GetItemBitmap(1).IsOk());

        m_container->SetItemBitmap(0, bitmap);

        CHECK(m_container->GetItemBitmap(0).IsOk());

        CHECK_EQ(wxSize(16, 16), m_container->GetBitmapSize());

        m_container->SetSelection( 1 );

        CHECK_EQ( m_container->GetStringSelection(), "item with bitmap" );
    }

    //wxITEM_CONTAINER_TESTS();
}

using BitmapComboBoxTextTest = TextEntryTest<wxBitmapComboBox>;

TEST_CASE_FIXTURE(BitmapComboBoxTextTest, "Bitmap combobox test")
{
    m_entry = std::make_unique<wxBitmapComboBox>(wxTheApp->GetTopWindow(),
                                                 wxID_ANY);

    wxTEXT_ENTRY_TESTS();
}

#endif //wxUSE_BITMAPCOMBOBOX
