///////////////////////////////////////////////////////////////////////////////
// Name:        tests/sizers/boxsizer.cpp
// Purpose:     Unit tests for wxBoxSizer
// Author:      Vadim Zeitlin
// Created:     2010-03-06
// Copyright:   (c) 2010 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "wx/listbox.h"

#include "asserthelper.h"

import WX.Core.Sizer;

import WX.Test.Prec;

// ----------------------------------------------------------------------------
// test fixture
// ----------------------------------------------------------------------------

class BoxSizerTestCase
{
public:
    BoxSizerTestCase()
        : m_win(std::make_unique<wxWindow>(wxTheApp->GetTopWindow(), wxID_ANY)),
          m_sizer(new wxBoxSizer(wxHORIZONTAL))
    {
        m_win->SetClientSize(127, 35);
        m_win->SetSizer(m_sizer);
    }

protected:
    const std::unique_ptr<wxWindow> m_win;
    wxSizer* const m_sizer;
};

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::Size1")
{
    const wxSize sizeTotal = m_win->GetClientSize();
    const wxSize sizeChild = sizeTotal / 2;

    wxWindow * const
        child = new wxWindow(m_win.get(), wxID_ANY, wxDefaultPosition, sizeChild);
    m_sizer->Add(child);
    m_win->Layout();
    CHECK(child->GetSize() == sizeChild);

    m_sizer->Clear();
    m_sizer->Add(child, wxSizerFlags(1));
    m_win->Layout();
    CHECK( child->GetSize() == wxSize(sizeTotal.x, sizeChild.y) );

    m_sizer->Clear();
    m_sizer->Add(child, wxSizerFlags(1).Expand());
    m_win->Layout();
    CHECK(child->GetSize() == sizeTotal);

    m_sizer->Clear();
    m_sizer->Add(child, wxSizerFlags());
    m_sizer->SetItemMinSize(child, sizeTotal*2);
    m_win->Layout();
    CHECK(child->GetSize() == sizeTotal);

    m_sizer->Clear();
    m_sizer->Add(child, wxSizerFlags().Expand());
    m_sizer->SetItemMinSize(child, sizeTotal*2);
    m_win->Layout();
    CHECK(child->GetSize() == sizeTotal);
}

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::Size3")
{
    // check that various combinations of minimal sizes and proportions work as
    // expected for different window sizes
    static constexpr struct LayoutTestData
    {
        // proportions of the elements
        int prop[3];

        // minimal sizes of the elements in the sizer direction
        int minsize[3];

        // total size and the expected sizes of the elements
        int x,
            sizes[3];

        // if true, don't try the permutations of our test data
        bool dontPermute;


        // Add the given window to the sizer with the corresponding parameters
        void AddToSizer(wxSizer *sizer, wxWindow *win, int n) const
        {
            sizer->Add(win, wxSizerFlags(prop[n]));
            sizer->SetItemMinSize(win, wxSize(minsize[n], -1));
        }

    } layoutTestData[] =
    {
        // some really simple cases (no need to permute those, they're
        // symmetrical anyhow)
        { { 1, 1, 1, }, {  50,  50,  50, }, 150, {  50,  50,  50, }, true },
        { { 2, 2, 2, }, {  50,  50,  50, }, 600, { 200, 200, 200, }, true },

        // items with different proportions and min sizes when there is enough
        // space to lay them out
        { { 1, 2, 3, }, {   0,   0,   0, }, 600, { 100, 200, 300, } },
        { { 1, 2, 3, }, { 100, 100, 100, }, 600, { 100, 200, 300, } },
        { { 1, 2, 3, }, { 100,  50,  50, }, 600, { 100, 200, 300, } },
        { { 0, 1, 1, }, { 200, 100, 100, }, 600, { 200, 200, 200, } },
        { { 0, 1, 2, }, { 300, 100, 100, }, 600, { 300, 100, 200, } },
        { { 0, 1, 1, }, { 100,  50,  50, }, 300, { 100, 100, 100, } },
        { { 0, 1, 2, }, { 100,  50,  50, }, 400, { 100, 100, 200, } },

        // cases when there is not enough space to lay out the items correctly
        // while still respecting their min sizes
        { { 0, 1, 1, }, { 100, 150,  50, }, 300, { 100, 150,  50, } },
        { { 1, 2, 3, }, { 100, 100, 100, }, 300, { 100, 100, 100, } },
        { { 1, 2, 3, }, { 100,  50,  50, }, 300, { 100,  80, 120, } },
        { { 1, 2, 3, }, { 100,  10,  10, }, 150, { 100,  20,  30, } },

        // cases when there is not enough space even for the min sizes (don't
        // permute in these cases as the layout does depend on the item order
        // because the first ones have priority)
        { { 1, 2, 3, }, { 100,  50,  50, }, 150, { 100,  50,   0, }, true },
        { { 1, 2, 3, }, { 100, 100, 100, }, 200, { 100, 100,   0, }, true },
        { { 1, 2, 3, }, { 100, 100, 100, }, 150, { 100,  50,   0, }, true },
        { { 1, 2, 3, }, { 100, 100, 100, },  50, {  50,   0,   0, }, true },
        { { 1, 2, 3, }, { 100, 100, 100, },   0, {   0,   0,   0, }, true },
    };

    wxWindow *child[3];
    child[0] = new wxWindow(m_win.get(), wxID_ANY);
    child[1] = new wxWindow(m_win.get(), wxID_ANY);
    child[2] = new wxWindow(m_win.get(), wxID_ANY);

    for ( unsigned i = 0; i < WXSIZEOF(layoutTestData); i++ )
    {
        LayoutTestData ltd = layoutTestData[i];

        // the results shouldn't depend on the order of items except in the
        // case when there is not enough space for even the fixed width items
        // (in which case the first ones might get enough of it but not the
        // last ones) so test a couple of permutations of test data unless
        // specifically disabled for this test case
        for ( unsigned p = 0; p < 3; p++)
        {
            switch ( p )
            {
                case 0:
                    // nothing to do, use original data
                    break;

                case 1:
                    // exchange first and last elements
                    std::swap(ltd.prop[0], ltd.prop[2]);
                    std::swap(ltd.minsize[0], ltd.minsize[2]);
                    std::swap(ltd.sizes[0], ltd.sizes[2]);
                    break;

                case 2:
                    // exchange the original third and second elements
                    std::swap(ltd.prop[0], ltd.prop[1]);
                    std::swap(ltd.minsize[0], ltd.minsize[1]);
                    std::swap(ltd.sizes[0], ltd.sizes[1]);
                    break;
            }

            m_sizer->Clear();

            unsigned j;
            for ( j = 0; j < WXSIZEOF(child); j++ )
                ltd.AddToSizer(m_sizer, child[j], j);

            m_win->SetClientSize(ltd.x, -1);
            m_win->Layout();

            for ( j = 0; j < WXSIZEOF(child); j++ )
            {
                CHECK_MESSAGE
                (
                    ltd.sizes[j] == child[j]->GetSize().x,
                    (
                        "test %lu, permutation #%lu: wrong size for child #%d "
                        "for total size %d",
                        static_cast<unsigned long>(i),
                        static_cast<unsigned long>(p),
                        j,
                        ltd.x
                    )
                );
            }

            // don't try other permutations if explicitly disabled
            if ( ltd.dontPermute )
                break;
        }
    }
}

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::CalcMin")
{
    static constexpr unsigned NUM_TEST_ITEM = 3;

    static constexpr struct CalcMinTestData
    {
        // proportions of the elements, if one of them is -1 it means to not
        // use this window at all in this test
        int prop[NUM_TEST_ITEM];

        // minimal sizes of the elements in the sizer direction
        int minsize[NUM_TEST_ITEM];

        // the expected minimal sizer size
        int total;
    } calcMinTestData[] =
    {
        { {  1,  1, -1 }, {  30,  50,   0 },  100 },
        { {  1,  1,  0 }, {  30,  50,  20 },  120 },
        { { 10, 10, -1 }, {  30,  50,   0 },  100 },
        { {  1,  2,  2 }, {  50,  50,  80 },  250 },
        { {  1,  2,  2 }, { 100,  50,  80 },  500 },
    };

    unsigned n;
    wxWindow *child[NUM_TEST_ITEM];
    for ( n = 0; n < NUM_TEST_ITEM; n++ )
        child[n] = new wxWindow(m_win.get(), wxID_ANY);

    for ( unsigned i = 0; i < WXSIZEOF(calcMinTestData); i++ )
    {
        m_sizer->Clear();

        const CalcMinTestData& cmtd = calcMinTestData[i];
        for ( n = 0; n < NUM_TEST_ITEM; n++ )
        {
            if ( cmtd.prop[n] != -1 )
            {
                child[n]->SetInitialSize(wxSize(cmtd.minsize[n], -1));
                m_sizer->Add(child[n], wxSizerFlags(cmtd.prop[n]));
            }
        }

        CHECK_MESSAGE
        (
            cmtd.total == m_sizer->CalcMin().x,
            ("In test #%u", i)
        );
    }
}

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::SetMinSize")
{
    wxWindow* const child = new wxWindow(m_win.get(), wxID_ANY);
    child->SetInitialSize(wxSize(10, -1));
    m_sizer->Add(child);

    // Setting minimal size explicitly must make GetMinSize() return at least
    // this size even if it needs a much smaller one.
    m_sizer->SetMinSize(wxSize{100, 0});
    CHECK(m_sizer->GetMinSize().x == 100);

    m_sizer->Layout();
    CHECK(m_sizer->GetMinSize().x == 100);
}

#if wxUSE_LISTBOX
TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::BestSizeRespectsMaxSize")
{
    m_sizer->Clear();

    const int maxWidth = 100;

    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxListBox* listbox = new wxListBox(m_win.get(), wxID_ANY);
    listbox->Append("some very very very very very very very very very very very long string");
    listbox->SetMaxSize(wxSize(maxWidth, -1));
    sizer->Add(listbox);

    m_sizer->Add(sizer);
    m_win->Layout();

    CHECK(listbox->GetSize().GetWidth() == maxWidth);
}

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::RecalcSizesRespectsMaxSize1")
{
    m_sizer->Clear();

    const int maxWidth = 100;

    m_win->SetClientSize(300, 300);

    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    m_sizer->Add(sizer1);

    wxListBox* listbox1 = new wxListBox(m_win.get(), wxID_ANY);
    listbox1->Append("some very very very very very very very very very very very long string");
    sizer1->Add(listbox1);

    wxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
    sizer1->Add(sizer2, wxSizerFlags().Expand());

    wxListBox* listbox2 = new wxListBox(m_win.get(), wxID_ANY);
    listbox2->Append("some string");
    listbox2->SetMaxSize(wxSize(100, -1));
    sizer2->Add(listbox2, wxSizerFlags().Proportion(1));

    m_win->Layout();

    CHECK(listbox2->GetSize().GetWidth() == maxWidth);
}
#endif

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::RecalcSizesRespectsMaxSize2")
{
    m_sizer->Clear();

    m_win->SetClientSize(300, 300);

    wxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
    m_sizer->Add(sizer1, wxSizerFlags().Expand());

    wxWindow* child1 = new wxWindow(m_win.get(), wxID_ANY);
    sizer1->Add(child1, wxSizerFlags().Proportion(1));

    wxWindow* child2 = new wxWindow(m_win.get(), wxID_ANY);
    child2->SetMaxSize(wxSize(-1, 50));
    sizer1->Add(child2, wxSizerFlags().Proportion(1));

    wxWindow* child3 = new wxWindow(m_win.get(), wxID_ANY);
    sizer1->Add(child3, wxSizerFlags().Proportion(1));

    m_win->Layout();

    CHECK(child1->GetSize().GetHeight() == 125);
    CHECK(child2->GetSize().GetHeight() == 50);
    CHECK(child3->GetSize().GetHeight() == 125);
}

TEST_CASE_FIXTURE(BoxSizerTestCase, "BoxSizer::Replace")
{
    m_sizer->AddSpacer(1);
    m_sizer->Replace(0, new wxSizerItem(new wxWindow(m_win.get(), wxID_ANY)));
}
