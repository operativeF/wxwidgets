///////////////////////////////////////////////////////////////////////////////
// Name:        tests/lists/lists.cpp
// Purpose:     wxList unit test
// Author:      Vadim Zeitlin, Mattia Barbon
// Created:     2004-12-08
// Copyright:   (c) 2004 Vadim Zeitlin, Mattia Barbon
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/list.h"

// --------------------------------------------------------------------------
// test class
// --------------------------------------------------------------------------


class Baz // Foo is already taken in the hash test
{
public:
    Baz(const wxString& name) : m_name(name) { ms_bars++; }
    Baz(const Baz& bar) : m_name(bar.m_name) { ms_bars++; }
   ~Baz() { ms_bars--; }

   static size_t GetNumber() { return ms_bars; }

   const wxChar *GetName() const { return m_name.c_str(); }

private:
   wxString m_name;

   static size_t ms_bars;
};

size_t Baz::ms_bars = 0;

#include "wx/list.h"

WX_DECLARE_LIST(Baz, wxListBazs);
#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxListBazs)

WX_DECLARE_LIST(int, wxListInt);
WX_DEFINE_LIST(wxListInt)

TEST_CASE("wxListTest")
{
    wxListInt list1;
    int dummy[5];
    size_t i;

    for ( i = 0; i < WXSIZEOF(dummy); ++i )
        list1.Append(dummy + i);

    CHECK_EQ( WXSIZEOF(dummy), list1.GetCount() );
    CHECK_EQ( dummy + 3, list1.Item(3)->GetData() );
    CHECK( list1.Find(dummy + 4) );

    wxListInt::compatibility_iterator node;
    for ( i = 0, node = list1.GetFirst(); node; ++i, node = node->GetNext() )
    {
        CHECK_EQ( dummy + i, node->GetData() );
    }

    CHECK_EQ( i, list1.GetCount() );

    list1.Insert(dummy + 0);
    list1.Insert(1, dummy + 1);
    list1.Insert(list1.GetFirst()->GetNext()->GetNext(), dummy + 2);

    for ( i = 0, node = list1.GetFirst(); i < 3; ++i, node = node->GetNext() )
    {
        int* t = node->GetData();
        CHECK_EQ( dummy + i, t );
    }
}

TEST_CASE("wxStdListTest")
{
    wxListInt list1;
    wxListInt::iterator it, en;
    wxListInt::reverse_iterator rit, ren;
    int i;
    for ( i = 0; i < 5; ++i )
        list1.push_back(i + &i);

    for ( it = list1.begin(), en = list1.end(), i = 0;
          it != en; ++it, ++i )
    {
        CHECK( *it == i + &i );
    }

    for ( rit = list1.rbegin(), ren = list1.rend(), i = 4;
          rit != ren; ++rit, --i )
    {
        CHECK( *rit == i + &i );
    }

    CHECK( *list1.rbegin() == *--list1.end() );
    CHECK( *list1.begin() == *--list1.rend() );
    CHECK( *list1.begin() == *--++list1.begin() );
    CHECK( *list1.rbegin() == *--++list1.rbegin() );

    CHECK( list1.front() == &i );
    CHECK( list1.back() == &i + 4 );

    list1.erase(list1.begin());
    list1.erase(--list1.end());

    for ( it = list1.begin(), en = list1.end(), i = 1;
          it != en; ++it, ++i )
    {
        CHECK( *it == i + &i );
    }

    list1.clear();
    CHECK( list1.empty() );

    it = list1.insert(list1.end(), (int *)1);
    CHECK_EQ( (int *)1, *it );
    CHECK( it == list1.begin() );
    CHECK_EQ( (int *)1, list1.front() );

    it = list1.insert(list1.end(), (int *)2);
    CHECK_EQ( (int *)2, *it );
    CHECK( ++it == list1.end() );
    CHECK_EQ( (int *)2, list1.back() );

    it = list1.begin();
    wxListInt::iterator it2 = list1.insert(++it, (int *)3);
    CHECK_EQ( (int *)3, *it2 );

    it = list1.begin();
    it = list1.erase(++it, list1.end());
    CHECK_EQ( 1, list1.size() );
    CHECK( it == list1.end() );

    wxListInt list2;
    list2.push_back((int *)3);
    list2.push_back((int *)4);
    list1.insert(list1.begin(), list2.begin(), list2.end());
    CHECK_EQ( 3, list1.size() );
    CHECK_EQ( (int *)3, list1.front() );

    list1.insert(list1.end(), list2.begin(), list2.end());
    CHECK_EQ( 5, list1.size() );
    CHECK_EQ( (int *)4, list1.back() );
}

TEST_CASE("wxListCtorTest")
{
    {
        wxListBazs list1;
        list1.Append(new Baz(wxT("first")));
        list1.Append(new Baz(wxT("second")));

        CHECK( list1.GetCount() == 2 );
        CHECK( Baz::GetNumber() == 2 );

        wxListBazs list2;
        list2 = list1;

        CHECK( list1.GetCount() == 2 );
        CHECK( list2.GetCount() == 2 );
        CHECK( Baz::GetNumber() == 2 );

        WX_CLEAR_LIST(wxListBazs, list1);
    }

    CHECK( Baz::GetNumber() == 0 );
}

import <list>;

// Check that we convert wxList to std::list using the latter's ctor taking 2
// iterators: this used to be broken in C++11 because wxList iterators didn't
// fully implement input iterator requirements.
TEST_CASE("wxList::iterator")
{
    Baz baz1("one"),
        baz2("two");

    wxListBazs li;
    li.push_back(&baz1);
    li.push_back(&baz2);

    std::list<Baz*> stdli(li.begin(), li.end());
    CHECK( stdli.size() == 2 );

    const wxListBazs cli;
    CHECK( std::list<Baz*>(cli.begin(), cli.end()).empty() );
}

