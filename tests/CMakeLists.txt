# add_executable(nongui_test 
#     test_main.cpp
#     any/anytest.cpp

#     testprec.h
#     testableframe.h
#     testdate.h
#     testfile.h
#     archive/archivetest.h
#     streams/bstream.h
# )

# target_compile_features(nongui_test PRIVATE cxx_std_23)

# target_include_directories(nongui_test 
#     PUBLIC
#         ${DOCTEST_INCLUDE_DIR}
#         ${CMAKE_SOURCE_DIR}/include/msvc
#         ${CMAKE_SOURCE_DIR}/include
# )

# target_link_libraries(nongui_test
#     PRIVATE
#         doctest::doctest
# )

set(TEST_GUI_SRC
    asserthelper.cpp
    test.cpp
    testableframe.cpp
    geometry/rect.cpp
    geometry/size.ixx
    geometry/point.ixx
    geometry/region.cpp
    graphics/bitmap.ixx
    #graphics/colour.cpp # Doesn't work yet.
    graphics/ellipsization.cpp
    graphics/measuring.ixx
    #graphics/affinematrix.cpp # Doesn't work yet. CPPUnit still.
    graphics/boundingbox.ixx
    #graphics/clippingbox.cpp # Doesn't work yet. CPPUnit still.
    graphics/coords.ixx
    #graphics/graphbitmap.cpp # FIXME: Works but doesn't test with matchers yet.
    #graphics/graphmatrix.cpp # Doesn't work yet.
    graphics/graphpath.cpp
    graphics/imagelist.cpp
    #config/config.cpp # FIXME: Use a more robust system for writing to file. Currently very fragile.
    controls/auitest.ixx
    controls/bitmapcomboboxtest.ixx
    controls/bitmaptogglebuttontest.ixx
    controls/buttontest.ixx
    controls/checkboxtest.ixx
    controls/checklistboxtest.ixx
    controls/choicebooktest.ixx
    controls/choicetest.ixx
    controls/comboboxtest.ixx
    controls/dataviewctrltest.cpp
    controls/datepickerctrltest.cpp
    controls/frametest.ixx
    controls/gaugetest.ixx
    controls/gridtest.ixx
    controls/headerctrltest.cpp
    #controls/htmllboxtest.cpp # FIXME: Need to get rid of wxArrayString
    controls/hyperlinkctrltest.cpp
    controls/label.cpp
    controls/listbooktest.cpp
    controls/listboxtest.cpp
    controls/listctrltest.cpp
    controls/listviewtest.cpp
    controls/markuptest.ixx
    controls/notebooktest.ixx
    controls/ownerdrawncomboboxtest.ixx
    controls/pickertest.cpp
    controls/radioboxtest.ixx
    controls/radiobuttontest.cpp
    controls/rearrangelisttest.cpp
    #controls/richtextctrltest.cpp # FIXME: Need to get rid of wxArrayString
    controls/searchctrltest.cpp
    controls/simplebooktest.cpp
    controls/slidertest.ixx
    controls/spinctrldbltest.cpp
    controls/spinctrltest.cpp
    controls/styledtextctrltest.cpp
    controls/textctrltest.ixx
    controls/togglebuttontest.cpp
    controls/toolbooktest.ixx
    controls/treebooktest.cpp
    controls/treectrltest.cpp
    controls/treelistctrltest.cpp
    controls/virtlistctrltest.ixx
    #controls/webtest.cpp # FIXME
    controls/windowtest.ixx
    #controls/dialogtest.cpp
    events/clone.ixx
    # Duplicate this file here to test GUI event loops too.
    events/evtlooptest.cpp
    #events/propagation.cpp # Needs some fixes.
    events/keyboard.cpp
    # And duplicate this one too as wxExecute behaves differently in
    # console and GUI applications.
    exec/exec.cpp
    font/fonttest.ixx
    image/image.ixx
    image/rawbmp.cpp
    #html/htmlparser.cpp # FIXME: Need to get rid of wxArrayString.
    #html/htmlwindow.cpp
    #html/htmprint.cpp
    menu/accelentry.ixx
    menu/menu.ixx
    misc/guifuncs.cpp
    misc/selstoretest.cpp
    #misc/garbage.cpp
    misc/safearrayconverttest.cpp
    misc/settings.cpp
    # This one is intentionally duplicated here (it is also part of
    # non-GUI test) as sockets behave differently in console and GUI
    # applications.
    #net/socket.cpp
    #persistence/tlw.cpp # FIXME: Relies on config to work correctly.
    persistence/dataview.cpp
    rowheightcache/rowheightcachetest.cpp
    sizers/boxsizer.ixx
    sizers/gridsizer.ixx
    sizers/wrapsizer.cpp
    toplevel/toplevel.cpp
    validators/valnum.cpp
    validators/valtext.ixx
    window/clientsize.ixx
    window/setsize.cpp
    xml/xrctest.cpp
)

add_executable(test_gui
    ${TEST_GUI_SRC}
)

target_compile_features(test_gui PRIVATE cxx_std_23)

target_include_directories(test_gui 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(test_gui
    PRIVATE
        wxcore
        wxbase
        wxconfigflags
        stdlibcore
        wxtype
        doctest::doctest
        wxprint
        ${JPEG_LIBRARIES}
    # Windows libraries
        wxwinwrap

        kernel32
        user32
        comdlg32
        winspool
        shell32
        comctl32
        ole32
        oleaut32
        uuid
        rpcrt4
        advapi32
        wininet
)

add_test(test_gui 
    ${TEST_GUI_SRC}
)

if(wxUSE_AUI)
    target_link_libraries(test_gui PRIVATE wxaui)
endif()
# FIXME: Need to get rid of wxArrayString
# if(wxUSE_RICHTEXT)
#     target_link_libraries(test_gui PRIVATE wxrichtext)
# endif()
# if(wxUSE_MEDIACTRL)
#     target_link_libraries(test_gui PRIVATE wxmedia)
# endif()
# if(wxUSE_XRC)
#     target_link_libraries(test_gui PRIVATE wxxrc)
# endif()
if(wxUSE_XML)
    target_link_libraries(test_gui PRIVATE wxxml)
endif()
# if(wxUSE_HTML)
#     target_link_libraries(test_gui PRIVATE wxhtml)
# endif()
# if(wxUSE_SOCKETS)
#     target_link_libraries(test_gui PRIVATE wxnet)
# endif()
# if(wxUSE_WEBVIEW)
#     target_link_libraries(test_gui PRIVATE wxwebview)
# endif()

target_precompile_headers(test_gui REUSE_FROM wxsharedcmn)
