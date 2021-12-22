///////////////////////////////////////////////////////////////////////////////
// Name:        tests/misc/garbage.cpp
// Purpose:     test if loading garbage fails
// Author:      Francesco Montorsi
// Created:     2009-01-10
// Copyright:   (c) 2009 Francesco Montorsi
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/filename.h"
#include "wx/icon.h"
#include "wx/animate.h"
#include "wx/dynlib.h"
#include "wx/mediactrl.h"
#include "wx/html/htmlwin.h"
#include "wx/xrc/xmlres.h"

import WX.Image;
import WX.Cmn.MemStream;

constexpr int GARBAGE_DATA_SIZE   = 1000000; // in bytes; ~ 1MB

// Execute the given macro with the given first and second parameters and
// bitmap type as its third parameter for all bitmap types.
#define wxFOR_ALL_VALID_BITMAP_TYPES(m, p1, p2) \
    for ( wxBitmapType type = wxBitmapType(static_cast<int>(wxBitmapType::Invalid) + 1); \
          static_cast<int>(type) < static_cast<int>(wxBitmapType::Max); \
          type = static_cast<wxBitmapType>(static_cast<int>(type) + 1) ) \
        m(p1, p2, type)

// Similar to above but for animation types.
#define wxFOR_ALL_VALID_ANIMATION_TYPES(m, p1, p2) \
    for ( wxAnimationType type = wxAnimationType(wxANIMATION_TYPE_INVALID + 1); \
          type < wxANIMATION_TYPE_ANY; \
          type = (wxAnimationType)(type + 1) ) \
        m(p1, p2, type)

// This macro is used as an argument to wxFOR_ALL_VALID_BITMAP_TYPES() to
// include the information about the type for which the test failed.
#define ASSERT_FUNC_FAILS_FOR_TYPE(func, arg, type) \
    CHECK_MESSAGE \
    ( \
      !func(arg), \
      (#func "() unexpectedly succeeded for type %d", type) \
    )

// And this one exists mostly just for consistency with the one above.
#define ASSERT_FUNC_FAILS(func, arg) \
    CHECK_MESSAGE \
    ( \
      !func(arg), \
      (#func "() unexpectedly succeeded for default type") \
    )


static void DoLoadFile(const wxString& fullname)
{
    // test wxImage
    wxImage img;
    ASSERT_FUNC_FAILS(img.LoadFile, fullname);
    wxFOR_ALL_VALID_BITMAP_TYPES(ASSERT_FUNC_FAILS_FOR_TYPE, img.LoadFile, fullname);

    // test wxBitmap
    wxBitmap bmp;
    ASSERT_FUNC_FAILS(bmp.LoadFile, fullname);
    wxFOR_ALL_VALID_BITMAP_TYPES(ASSERT_FUNC_FAILS_FOR_TYPE, bmp.LoadFile, fullname);

    // test wxIcon
    wxIcon icon;
    ASSERT_FUNC_FAILS(icon.LoadFile, fullname);
    wxFOR_ALL_VALID_BITMAP_TYPES(ASSERT_FUNC_FAILS_FOR_TYPE, icon.LoadFile, fullname);

#if wxUSE_ANIMATIONCTRL
    // test wxAnimation
    wxAnimation anim;
    ASSERT_FUNC_FAILS(anim.LoadFile, fullname);
    wxFOR_ALL_VALID_ANIMATION_TYPES(ASSERT_FUNC_FAILS_FOR_TYPE, anim.LoadFile, fullname);
#endif

    // test wxDynamicLibrary
    wxDynamicLibrary lib;
    CHECK(lib.Load(fullname) == false);

    /*
    #if wxUSE_MEDIACTRL
        // test wxMediaCtrl
        wxMediaCtrl *media = new wxMediaCtrl(wxTheApp->GetTopWindow());
        CHECK( media->Load(fullname) == false );
    #endif

        // test wxHtmlWindow
        wxHtmlWindow *htmlwin = new wxHtmlWindow(wxTheApp->GetTopWindow());
        CHECK( htmlwin->LoadFile(fullname) == false );
        delete htmlwin;
    */
    // test wxXmlResource
#if wxUSE_XRC
    CHECK(wxXmlResource::Get()->Load(fullname) == false);
#endif
}

static void DoLoadStream(wxInputStream& stream)
{
    // NOTE: not all classes tested by DoLoadFile() supports loading
    //       from an input stream!

    // test wxImage
    wxImage img;
    ASSERT_FUNC_FAILS(img.LoadFile, stream);
    wxFOR_ALL_VALID_BITMAP_TYPES(ASSERT_FUNC_FAILS_FOR_TYPE, img.LoadFile, stream);

#if wxUSE_ANIMATIONCTRL
    // test wxAnimation
    wxAnimation anim;
    ASSERT_FUNC_FAILS(anim.Load, stream);
    wxFOR_ALL_VALID_BITMAP_TYPES(ASSERT_FUNC_FAILS_FOR_TYPE, anim.Load, stream);
#endif
    /*
        // test wxHtmlWindow
        wxHtmlWindow *htmlwin = new wxHtmlWindow(wxTheApp->GetTopWindow());
        CHECK( htmlwin->LoadFile(fullname) == false );
        delete htmlwin;
    */
}


TEST_CASE("LoadGarbage")
{
    srand(1234);

    wxInitAllImageHandlers();

    for (size_t size = 1; size < GARBAGE_DATA_SIZE; size *= size+1)
    {
        // first, generate some garbage data
        unsigned char *data = new unsigned char[size];
        for (size_t i = 0; i < size; i++)
            data[i] = rand();

        // write it to a file
        wxString garbagename = wxFileName::CreateTempFileName("garbage");
        CHECK( !garbagename.empty() );

        wxFile garbage(garbagename, wxFile::write);
        CHECK( garbage.IsOpened() );

        CHECK( garbage.Write(data, size) == size );
        garbage.Close();

        // try to load it by name
        DoLoadFile(garbagename);

        // try to load it from a wxInputStream
        wxMemoryInputStream stream(data, size);
        DoLoadStream(stream);

        wxDELETEA(data);
    }
}


#undef ASSERT_FUNC_FAILS
#undef ASSERT_FUNC_FAILS_FOR_TYPE
#undef wxFOR_ALL_VALID_ANIMATION_TYPES
#undef wxFOR_ALL_VALID_BITMAP_TYPES
