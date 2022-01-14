///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/misctests.cpp
// Purpose:     test miscellaneous GUI functions
// Author:      Vadim Zeitlin
// Created:     2008-09-22
// Copyright:   (c) 2008 Vadim Zeitlin
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "wx/gdicmn.h"
#include "wx/filefn.h"
#include "wx/app.h"
#include "wx/clipbrd.h"
#include "wx/dataobj.h"
#include "wx/panel.h"
#include "wx/utils.h"

export module WX.Test.GUIFuncs;

import WX.Test.Prec;
import WX.MetaTest;

import Utils.Geometry;

namespace ut = boost::ut;

ut::suite DisplaySizeTest = []
{
    using namespace ut;

    // test that display PPI is something reasonable
    auto sz = wxGetDisplayPPI();
    expect( sz.x < 1000 );
    expect( sz.y < 1000 );
};

// NOTE  / FIXME: Internal compiler error when attempting to convert to
// ut::suite test. 
// C:\dev\wxWidgets\tests\misc\guifuncs.ixx(181, 1) : fatal error C1001 : Internal compiler error.
// d:\a01\_work\5\s\src\vctools\Compiler\CxxFE\sl\p1\c\module\writer.cpp:6090 : sorry : not yet implemented
// (compiler file 'd:\a01\_work\5\s\src\vctools\Compiler\CxxFE\sl\p1\c\module\utilities.h', line 34)
// To work around this problem, try simplifying or changing the program near the locations listed above.
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

ut::suite ParseFileDialogFilterTest = []
{
    using namespace ut;

    std::vector<std::string> descs;
    std::vector<std::string> filters;

    expect( 1 == wxParseCommonDialogsFilter("Image files|*.jpg;*.png", descs, filters));

    expect( "Image files" == descs[0] );
    expect( "*.jpg;*.png" == filters[0] );

    expect(2 == wxParseCommonDialogsFilter("All files (*.*)|*.*|Python source (*.py)|*.py", descs, filters));

    expect( "*.*" == filters[0] );
    expect( "*.py" == filters[1] );

    // Test some invalid ones too.
    expect(throws([&]{ wxParseCommonDialogsFilter(
            "All files (*.*)|*.*|Python source (*.py)|*.py|",
            descs, filters); } ));
};

ut::suite ClientToScreenTest = []
{
    using namespace ut;

    wxWindow* const tlw = wxTheApp->GetTopWindow();
    expect( tlw );

    wxPanel* const
        p1 = new wxPanel(tlw, wxID_ANY, wxPoint(0, 0), wxSize(100, 50));
    wxPanel* const
        p2 = new wxPanel(tlw, wxID_ANY, wxPoint(0, 50), wxSize(100, 50));
    wxWindow* const
        b = new wxWindow(p2, wxID_ANY, wxPoint(10, 10), wxSize(30, 10));

    // We need this to realize the windows created above under wxGTK.
    wxYield();

    const wxPoint tlwOrig = tlw->ClientToScreen(wxPoint(0, 0));

    expect(tlwOrig + wxPoint(0, 50) == p2->ClientToScreen(wxPoint(0, 0)));

    expect(tlwOrig + wxPoint(10, 60) == b->ClientToScreen(wxPoint(0, 0)));

    p1->Destroy();
    p2->Destroy();
};

// This class is used as a test window here. We can't use a real wxButton
// because we can't create other windows as its children in wxGTK.
class TestButton : public wxWindow
{
public:
    TestButton(wxWindow* parent, const std::string& label, const wxPoint& pos)
        : wxWindow(parent, wxID_ANY, pos, wxSize(100, 50))
    {
        SetLabel(label);
    }
};

// Helper function returning the label of the window at the given point or
// "NONE" if there is no window there.
std::string GetLabelOfWindowAtPoint(wxWindow* parent, int x, int y)
{
    wxWindow* const
        win = wxFindWindowAtPoint(parent->ClientToScreen(wxPoint(x, y)));
    return win ? win->GetLabel() : "NONE";
}

ut::suite FindWindowAtPointTest = []
{
    using namespace ut;

    wxWindow* const parent = wxTheApp->GetTopWindow();
    expect( parent );

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

    expect("NONE" == GetLabelOfWindowAtPoint(parent, 900, 900)) <<
        "No window for a point outside of the window";

    expect(btn1->GetLabel() == GetLabelOfWindowAtPoint(parent, 11, 11)) <<
        "Point over a child control corresponds to it";

    expect(parent->GetLabel() == GetLabelOfWindowAtPoint(parent, 5, 5)) <<
        "Point outside of any child control returns the TLW itself";

    btn2->Disable();
    
    expect(btn2->GetLabel() == GetLabelOfWindowAtPoint(parent, 11, 91)) <<
        "Point over a disabled child control still corresponds to it";

    btn2->Hide();

    expect(parent->GetLabel() == GetLabelOfWindowAtPoint(parent, 11, 91)) <<
        "Point over a hidden child control doesn't take it into account";

    btn2->Show();

    expect(btn3->GetLabel() == GetLabelOfWindowAtPoint(parent, 31, 111)) <<
        "Point over child control corresponds to the child";

    btn3->Disable();

    expect(btn3->GetLabel() == GetLabelOfWindowAtPoint(parent, 31, 111)) <<
        "Point over disabled child controls still corresponds to this child";

    //btn1->Destroy();
    //btn2->Destroy();
    // NOTE / FIXME:
    // btn3 was already deleted when its parent was
};
