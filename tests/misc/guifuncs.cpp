///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/misctests.cpp
// Purpose:     test miscellaneous GUI functions
// Author:      Vadim Zeitlin
// Created:     2008-09-22
// Copyright:   (c) 2008 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "doctest.h"

#include "testprec.h"


#include "wx/defs.h"

#ifndef WX_PRECOMP
    #include "wx/gdicmn.h"
    #include "wx/filefn.h"
#endif // !PCH

#include "wx/app.h"
#include "wx/button.h"
#include "wx/clipbrd.h"
#include "wx/dataobj.h"
#include "wx/panel.h"

#include "asserthelper.h"

TEST_CASE("DisplaySize")
{
    // test that different (almost) overloads return the same results
    int w, h;
    wxDisplaySize(&w, &h);
    wxSize sz = wxGetDisplaySize();

    CHECK_EQ( w, sz.x );
    CHECK_EQ( h, sz.y );

    // test that passing nullptr works as expected, e.g. doesn't crash
    wxDisplaySize(nullptr, nullptr);
    wxDisplaySize(&w, nullptr);
    wxDisplaySize(nullptr, &h);

    CHECK_EQ( w, sz.x );
    CHECK_EQ( h, sz.y );

    // test that display PPI is something reasonable
    sz = wxGetDisplayPPI();
    CHECK( sz.x < 1000 );
    CHECK( sz.y < 1000 );
}

TEST_CASE("URLDataObject")
{
#if wxUSE_DATAOBJ
    // this tests for buffer overflow, see #11102
    const char * const
        url = "http://something.long.to.overwrite.plenty.memory.example.com";
    wxURLDataObject * const dobj = new wxURLDataObject(url);
    CHECK_EQ( url, dobj->GetURL() );

    wxClipboardLocker lockClip;
    CHECK( wxTheClipboard->SetData(dobj) );
    wxTheClipboard->Flush();
#endif // wxUSE_DATAOBJ
}

TEST_CASE("ParseFileDialogFilter")
{
    std::vector<wxString> descs;
    std::vector<wxString> filters;

    CHECK_EQ
    (
        1,
        wxParseCommonDialogsFilter("Image files|*.jpg;*.png", descs, filters)
    );

    CHECK_EQ( "Image files", descs[0] );
    CHECK_EQ( "*.jpg;*.png", filters[0] );

    CHECK_EQ
    (
        2,
        wxParseCommonDialogsFilter
        (
            "All files (*.*)|*.*|Python source (*.py)|*.py",
            descs, filters
        )
    );

    CHECK_EQ( "*.*", filters[0] );
    CHECK_EQ( "*.py", filters[1] );

    // Test some invalid ones too.
    CHECK_THROWS
    (
        wxParseCommonDialogsFilter
        (
            "All files (*.*)|*.*|Python source (*.py)|*.py|",
            descs, filters
        )
    );
}

TEST_CASE("ClientToScreen")
{
    wxWindow* const tlw = wxTheApp->GetTopWindow();
    CHECK( tlw );

    wxPanel* const
        p1 = new wxPanel(tlw, wxID_ANY, wxPoint(0, 0), wxSize(100, 50));
    wxPanel* const
        p2 = new wxPanel(tlw, wxID_ANY, wxPoint(0, 50), wxSize(100, 50));
    wxWindow* const
        b = new wxWindow(p2, wxID_ANY, wxPoint(10, 10), wxSize(30, 10));

    // We need this to realize the windows created above under wxGTK.
    wxYield();

    const wxPoint tlwOrig = tlw->ClientToScreen(wxPoint(0, 0));

    CHECK_EQ
    (
        tlwOrig + wxPoint(0, 50),
        p2->ClientToScreen(wxPoint(0, 0))
    );

    CHECK_EQ
    (
        tlwOrig + wxPoint(10, 60),
        b->ClientToScreen(wxPoint(0, 0))
    );

    p1->Destroy();
    p2->Destroy();
}

namespace
{

// This class is used as a test window here. We can't use a real wxButton
// because we can't create other windows as its children in wxGTK.
class TestButton : public wxWindow
{
public:
    TestButton(wxWindow* parent, const wxString& label, const wxPoint& pos)
        : wxWindow(parent, wxID_ANY, pos, wxSize(100, 50))
    {
        SetLabel(label);
    }
};

// Helper function returning the label of the window at the given point or
// "NONE" if there is no window there.
wxString GetLabelOfWindowAtPoint(wxWindow* parent, int x, int y)
{
    wxWindow* const
        win = wxFindWindowAtPoint(parent->ClientToScreen(wxPoint(x, y)));
    return win ? win->GetLabel() : wxString("NONE");
}

} // anonymous namespace

TEST_CASE("FindWindowAtPoint")
{
    wxWindow* const parent = wxTheApp->GetTopWindow();
    CHECK( parent );

    // Set a label to allow distinguishing it from the other windows in the
    // assertion messages.
    parent->SetLabel("parent");

    // Set the position so it doesn't interfere with other windows.
    parent->SetPosition({400, 400});

    auto btn1 = std::make_unique<TestButton>(parent, "1", wxPoint(10, 10));
    auto btn2 = std::make_unique<TestButton>(parent, "2", wxPoint(10, 90));
    auto btn3 = std::make_unique<TestButton>(btn2.get(), "3", wxPoint(20, 20));

    // We need this to realize the windows created above under wxGTK.
    wxYield();

    CHECK_MESSAGE
    (
        "NONE" == GetLabelOfWindowAtPoint(parent, 900, 900),
        "No window for a point outside of the window"
    );

    CHECK_MESSAGE
    (
        btn1->GetLabel() == GetLabelOfWindowAtPoint(parent, 11, 11),
        "Point over a child control corresponds to it"
    );

    CHECK_MESSAGE
    (
        parent->GetLabel() == GetLabelOfWindowAtPoint(parent, 5, 5),
        "Point outside of any child control returns the TLW itself"
    );

    btn2->Disable();
    CHECK_MESSAGE
    (
        btn2->GetLabel() == GetLabelOfWindowAtPoint(parent, 11, 91),
        "Point over a disabled child control still corresponds to it"
    );

    btn2->Hide();
    CHECK_MESSAGE
    (
        parent->GetLabel() == GetLabelOfWindowAtPoint(parent, 11, 91),
        "Point over a hidden child control doesn't take it into account"
    );

    btn2->Show();
    CHECK_MESSAGE
    (
        btn3->GetLabel() == GetLabelOfWindowAtPoint(parent, 31, 111),
        "Point over child control corresponds to the child"
    );

    btn3->Disable();
    CHECK_MESSAGE
    (
        btn3->GetLabel() == GetLabelOfWindowAtPoint(parent, 31, 111),
        "Point over disabled child controls still corresponds to this child"
    );

    //btn1->Destroy();
    //btn2->Destroy();
    // btn3 was already deleted when its parent was
}
