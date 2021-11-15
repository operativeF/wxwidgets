set(CORE_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(GUI_CMN_SRC
    ${CORE_SRC_DIR}/common/accelcmn.cpp
    ${CORE_SRC_DIR}/common/accesscmn.cpp
    ${CORE_SRC_DIR}/common/anidecod.cpp
    ${CORE_SRC_DIR}/common/appcmn.cpp
    ${CORE_SRC_DIR}/common/artprov.cpp
    ${CORE_SRC_DIR}/common/artstd.cpp
    ${CORE_SRC_DIR}/common/arttango.cpp
    ${CORE_SRC_DIR}/common/bmpbase.cpp
    ${CORE_SRC_DIR}/common/bmpbtncmn.cpp
    ${CORE_SRC_DIR}/common/bookctrl.cpp
    ${CORE_SRC_DIR}/common/btncmn.cpp
    ${CORE_SRC_DIR}/common/cairo.cpp
    ${CORE_SRC_DIR}/common/checkboxcmn.cpp
    ${CORE_SRC_DIR}/common/checklstcmn.cpp
    ${CORE_SRC_DIR}/common/choiccmn.cpp
    ${CORE_SRC_DIR}/common/clipcmn.cpp
    ${CORE_SRC_DIR}/common/clrpickercmn.cpp
    ${CORE_SRC_DIR}/common/colourcmn.cpp
    ${CORE_SRC_DIR}/common/colourdata.cpp
    ${CORE_SRC_DIR}/common/combocmn.cpp
    ${CORE_SRC_DIR}/common/cmdproc.cpp
    ${CORE_SRC_DIR}/common/cmndata.cpp
    ${CORE_SRC_DIR}/common/containr.cpp
    ${CORE_SRC_DIR}/common/cshelp.cpp
    ${CORE_SRC_DIR}/common/ctrlcmn.cpp
    ${CORE_SRC_DIR}/common/ctrlsub.cpp
    ${CORE_SRC_DIR}/common/dcbase.cpp
    ${CORE_SRC_DIR}/common/dcbufcmn.cpp
    ${CORE_SRC_DIR}/common/dcgraph.cpp
    ${CORE_SRC_DIR}/common/dcsvg.cpp
    ${CORE_SRC_DIR}/common/dirctrlcmn.cpp
    ${CORE_SRC_DIR}/common/dlgcmn.cpp
    ${CORE_SRC_DIR}/common/dobjcmn.cpp
    ${CORE_SRC_DIR}/common/docmdi.cpp
    ${CORE_SRC_DIR}/common/docview.cpp
    ${CORE_SRC_DIR}/common/dpycmn.cpp
    ${CORE_SRC_DIR}/common/dseldlg.cpp
    ${CORE_SRC_DIR}/common/fddlgcmn.cpp
    ${CORE_SRC_DIR}/common/filectrlcmn.cpp
    ${CORE_SRC_DIR}/common/filehistorycmn.cpp
    ${CORE_SRC_DIR}/common/filepickercmn.cpp
    ${CORE_SRC_DIR}/common/fontpickercmn.cpp
    ${CORE_SRC_DIR}/common/fldlgcmn.cpp
    ${CORE_SRC_DIR}/common/fontcmn.cpp
    ${CORE_SRC_DIR}/common/fontdata.cpp
    ${CORE_SRC_DIR}/generic/graphicc.cpp
    ${CORE_SRC_DIR}/common/fontenumcmn.cpp
    ${CORE_SRC_DIR}/common/fontmap.cpp
    ${CORE_SRC_DIR}/common/fontutilcmn.cpp
    ${CORE_SRC_DIR}/common/framecmn.cpp
    ${CORE_SRC_DIR}/common/gaugecmn.cpp
    ${CORE_SRC_DIR}/common/gbsizer.cpp
    ${CORE_SRC_DIR}/common/gdicmn.cpp
    ${CORE_SRC_DIR}/common/geometry.cpp
    ${CORE_SRC_DIR}/common/gifdecod.cpp
    ${CORE_SRC_DIR}/common/graphcmn.cpp
    ${CORE_SRC_DIR}/common/headercolcmn.cpp
    ${CORE_SRC_DIR}/common/headerctrlcmn.cpp
    ${CORE_SRC_DIR}/common/helpbase.cpp
    ${CORE_SRC_DIR}/common/iconbndl.cpp
    ${CORE_SRC_DIR}/common/imagall.cpp
    ${CORE_SRC_DIR}/common/imagbmp.cpp
    ${CORE_SRC_DIR}/common/image.cpp
    ${CORE_SRC_DIR}/common/imagfill.cpp
    ${CORE_SRC_DIR}/common/imaggif.cpp
    ${CORE_SRC_DIR}/common/imagiff.cpp
    ${CORE_SRC_DIR}/common/imagjpeg.cpp
    ${CORE_SRC_DIR}/common/imagpcx.cpp
    ${CORE_SRC_DIR}/common/imagpng.cpp
    ${CORE_SRC_DIR}/common/imagpnm.cpp
    ${CORE_SRC_DIR}/common/imagtga.cpp
    ${CORE_SRC_DIR}/common/imagtiff.cpp
    ${CORE_SRC_DIR}/common/imagxpm.cpp
    ${CORE_SRC_DIR}/common/layout.cpp
    ${CORE_SRC_DIR}/common/lboxcmn.cpp
    ${CORE_SRC_DIR}/common/listctrlcmn.cpp
    ${CORE_SRC_DIR}/common/markupparser.cpp
    ${CORE_SRC_DIR}/common/menucmn.cpp
    ${CORE_SRC_DIR}/common/modalhook.cpp
    ${CORE_SRC_DIR}/common/mousemanager.cpp
    ${CORE_SRC_DIR}/common/nbkbase.cpp
    ${CORE_SRC_DIR}/common/overlaycmn.cpp
    ${CORE_SRC_DIR}/common/ownerdrwcmn.cpp
    ${CORE_SRC_DIR}/common/paper.cpp
    ${CORE_SRC_DIR}/common/panelcmn.cpp
    ${CORE_SRC_DIR}/common/persist.cpp
    ${CORE_SRC_DIR}/common/pickerbase.cpp
    ${CORE_SRC_DIR}/common/popupcmn.cpp
    ${CORE_SRC_DIR}/common/preferencescmn.cpp
    ${CORE_SRC_DIR}/common/prntbase.cpp
    ${CORE_SRC_DIR}/common/quantize.cpp
    ${CORE_SRC_DIR}/common/radiobtncmn.cpp
    ${CORE_SRC_DIR}/common/radiocmn.cpp
    ${CORE_SRC_DIR}/common/rearrangectrl.cpp
    ${CORE_SRC_DIR}/common/rendcmn.cpp
    ${CORE_SRC_DIR}/common/rgncmn.cpp
    ${CORE_SRC_DIR}/common/scrolbarcmn.cpp
    ${CORE_SRC_DIR}/common/settcmn.cpp
    ${CORE_SRC_DIR}/common/sizer.cpp
    ${CORE_SRC_DIR}/common/slidercmn.cpp
    ${CORE_SRC_DIR}/common/spinbtncmn.cpp
    ${CORE_SRC_DIR}/common/spinctrlcmn.cpp
    ${CORE_SRC_DIR}/common/srchcmn.cpp
    ${CORE_SRC_DIR}/common/statbar.cpp
    ${CORE_SRC_DIR}/common/statbmpcmn.cpp
    ${CORE_SRC_DIR}/common/statboxcmn.cpp
    ${CORE_SRC_DIR}/common/statlinecmn.cpp
    ${CORE_SRC_DIR}/common/stattextcmn.cpp
    ${CORE_SRC_DIR}/common/stockitem.cpp
    ${CORE_SRC_DIR}/common/tbarbase.cpp
    ${CORE_SRC_DIR}/common/textcmn.cpp
    ${CORE_SRC_DIR}/common/textentrycmn.cpp
    ${CORE_SRC_DIR}/common/textmeasurecmn.cpp
    ${CORE_SRC_DIR}/common/toplvcmn.cpp
    ${CORE_SRC_DIR}/common/treebase.cpp
    ${CORE_SRC_DIR}/common/uiactioncmn.cpp
    ${CORE_SRC_DIR}/common/valgen.cpp
    ${CORE_SRC_DIR}/common/validate.cpp
    ${CORE_SRC_DIR}/common/valtext.cpp
    ${CORE_SRC_DIR}/common/valnum.cpp
    ${CORE_SRC_DIR}/common/wincmn.cpp
    ${CORE_SRC_DIR}/common/windowid.cpp
    ${CORE_SRC_DIR}/common/wrapsizer.cpp
    ${CORE_SRC_DIR}/common/xpmdecod.cpp
    ${CORE_SRC_DIR}/generic/busyinfo.cpp
    ${CORE_SRC_DIR}/generic/buttonbar.cpp
    ${CORE_SRC_DIR}/generic/choicdgg.cpp
    ${CORE_SRC_DIR}/generic/choicbkg.cpp
    ${CORE_SRC_DIR}/generic/collheaderctrlg.cpp
    ${CORE_SRC_DIR}/generic/combog.cpp
    ${CORE_SRC_DIR}/generic/dcpsg.cpp
    ${CORE_SRC_DIR}/generic/dirctrlg.cpp
    ${CORE_SRC_DIR}/generic/dragimgg.cpp
    ${CORE_SRC_DIR}/generic/filectrlg.cpp
    ${CORE_SRC_DIR}/generic/headerctrlg.cpp
    ${CORE_SRC_DIR}/generic/infobar.cpp
    ${CORE_SRC_DIR}/generic/listbkg.cpp
    ${CORE_SRC_DIR}/generic/logg.cpp
    ${CORE_SRC_DIR}/generic/markuptext.cpp
    ${CORE_SRC_DIR}/generic/msgdlgg.cpp
    ${CORE_SRC_DIR}/generic/numdlgg.cpp
    ${CORE_SRC_DIR}/generic/progdlgg.cpp
    ${CORE_SRC_DIR}/generic/preferencesg.cpp
    ${CORE_SRC_DIR}/generic/printps.cpp
    ${CORE_SRC_DIR}/generic/renderg.cpp
    ${CORE_SRC_DIR}/generic/richmsgdlgg.cpp
    ${CORE_SRC_DIR}/generic/scrlwing.cpp
    ${CORE_SRC_DIR}/generic/selstore.cpp
    ${CORE_SRC_DIR}/generic/spinctlg.cpp
    ${CORE_SRC_DIR}/generic/splitter.cpp
    ${CORE_SRC_DIR}/generic/srchctlg.cpp
    ${CORE_SRC_DIR}/generic/statbmpg.cpp
    ${CORE_SRC_DIR}/generic/stattextg.cpp
    ${CORE_SRC_DIR}/generic/textdlgg.cpp
    ${CORE_SRC_DIR}/generic/tipwin.cpp
    ${CORE_SRC_DIR}/generic/toolbkg.cpp
    ${CORE_SRC_DIR}/generic/treectlg.cpp
    ${CORE_SRC_DIR}/generic/treebkg.cpp
    ${CORE_SRC_DIR}/generic/vlbox.cpp
    ${CORE_SRC_DIR}/generic/vscroll.cpp
    ${CORE_SRC_DIR}/xrc/xmlreshandler.cpp
    ${CORE_SRC_DIR}/common/bmpcboxcmn.cpp
    ${CORE_SRC_DIR}/generic/grideditors.cpp
    ${CORE_SRC_DIR}/generic/gridctrl.cpp
    ${CORE_SRC_DIR}/generic/grid.cpp
    ${CORE_SRC_DIR}/generic/hyperlinkg.cpp
    ${CORE_SRC_DIR}/common/calctrlcmn.cpp
    ${CORE_SRC_DIR}/generic/notifmsgg.cpp
    ${CORE_SRC_DIR}/generic/odcombo.cpp
    ${CORE_SRC_DIR}/generic/splash.cpp
    ${CORE_SRC_DIR}/common/gridcmn.cpp
    ${CORE_SRC_DIR}/common/addremovectrl.cpp
    ${CORE_SRC_DIR}/generic/tipdlg.cpp
    ${CORE_SRC_DIR}/generic/aboutdlgg.cpp
    ${CORE_SRC_DIR}/generic/gridsel.cpp
    ${CORE_SRC_DIR}/generic/sashwin.cpp
    ${CORE_SRC_DIR}/generic/helpext.cpp
    ${CORE_SRC_DIR}/generic/richtooltipg.cpp
    ${CORE_SRC_DIR}/generic/bmpcboxg.cpp
    ${CORE_SRC_DIR}/generic/timectrlg.cpp
    ${CORE_SRC_DIR}/common/notifmsgcmn.cpp
    ${CORE_SRC_DIR}/generic/commandlinkbuttong.cpp
    ${CORE_SRC_DIR}/generic/propdlg.cpp
    ${CORE_SRC_DIR}/generic/treelist.cpp
    ${CORE_SRC_DIR}/common/datavcmn.cpp
    ${CORE_SRC_DIR}/common/animatecmn.cpp
    ${CORE_SRC_DIR}/common/odcombocmn.cpp
    ${CORE_SRC_DIR}/common/hyperlnkcmn.cpp
    ${CORE_SRC_DIR}/common/richtooltipcmn.cpp
    ${CORE_SRC_DIR}/generic/datectlg.cpp
    ${CORE_SRC_DIR}/generic/bannerwindow.cpp
    ${CORE_SRC_DIR}/generic/laywin.cpp
    ${CORE_SRC_DIR}/generic/calctrlg.cpp
    ${CORE_SRC_DIR}/generic/wizard.cpp
    ${CORE_SRC_DIR}/generic/editlbox.cpp
    ${CORE_SRC_DIR}/generic/datavgen.cpp
    ${CORE_SRC_DIR}/generic/creddlgg.cpp
    ${CORE_SRC_DIR}/generic/rowheightcache.cpp
    ${CORE_SRC_DIR}/generic/animateg.cpp
)

set(MSW_DESKTOP_LOWLEVEL_SRC
    ${CORE_SRC_DIR}/msw/helpchm.cpp
    ${CORE_SRC_DIR}/msw/helpwin.cpp
    ${CORE_SRC_DIR}/msw/ole/automtn.cpp
    ${CORE_SRC_DIR}/msw/ole/uuid.cpp
)

set(MSW_LOWLEVEL_SRC
    ${CORE_SRC_DIR}/msw/ole/activex.cpp
    ${CORE_SRC_DIR}/msw/app.cpp
    ${CORE_SRC_DIR}/msw/bitmap.cpp
    ${CORE_SRC_DIR}/msw/brush.cpp
    ${CORE_SRC_DIR}/msw/caret.cpp
    ${CORE_SRC_DIR}/msw/clipbrd.cpp
    ${CORE_SRC_DIR}/msw/colour.cpp
    ${CORE_SRC_DIR}/msw/cursor.cpp
    ${CORE_SRC_DIR}/msw/data.cpp
    ${CORE_SRC_DIR}/msw/dc.cpp
    ${CORE_SRC_DIR}/msw/dcclient.cpp
    ${CORE_SRC_DIR}/msw/dcmemory.cpp
    ${CORE_SRC_DIR}/msw/dcprint.cpp
    ${CORE_SRC_DIR}/msw/dcscreen.cpp
    ${CORE_SRC_DIR}/msw/dib.cpp
    ${CORE_SRC_DIR}/msw/display.cpp
    ${CORE_SRC_DIR}/msw/enhmeta.cpp
    ${CORE_SRC_DIR}/msw/font.cpp
    ${CORE_SRC_DIR}/msw/fontenum.cpp
    ${CORE_SRC_DIR}/msw/fontutil.cpp
    ${CORE_SRC_DIR}/msw/gdiimage.cpp
    ${CORE_SRC_DIR}/msw/gdiobj.cpp
    ${CORE_SRC_DIR}/msw/gdiplus.cpp
    ${CORE_SRC_DIR}/msw/graphics.cpp
    ${CORE_SRC_DIR}/msw/graphicsd2d.cpp
    ${CORE_SRC_DIR}/msw/icon.cpp
    ${CORE_SRC_DIR}/msw/imaglist.cpp
    ${CORE_SRC_DIR}/msw/minifram.cpp
    ${CORE_SRC_DIR}/msw/nonownedwnd.cpp
    ${CORE_SRC_DIR}/msw/ole/comimpl.cpp
    ${CORE_SRC_DIR}/msw/ole/dataobj.cpp
    ${CORE_SRC_DIR}/msw/ole/dropsrc.cpp
    ${CORE_SRC_DIR}/msw/ole/droptgt.cpp
    ${CORE_SRC_DIR}/msw/ole/oleutils.cpp
    ${CORE_SRC_DIR}/msw/ole/safearray.cpp
    ${CORE_SRC_DIR}/msw/palette.cpp
    ${CORE_SRC_DIR}/msw/pen.cpp
    ${CORE_SRC_DIR}/msw/popupwin.cpp
    ${CORE_SRC_DIR}/msw/printdlg.cpp
    ${CORE_SRC_DIR}/msw/printwin.cpp
    ${CORE_SRC_DIR}/msw/region.cpp
    ${CORE_SRC_DIR}/msw/renderer.cpp
    ${CORE_SRC_DIR}/msw/rt/utilsrt.cpp
    ${CORE_SRC_DIR}/msw/settings.cpp
    ${CORE_SRC_DIR}/msw/textmeasure.cpp
    ${CORE_SRC_DIR}/msw/tooltip.cpp
    ${CORE_SRC_DIR}/msw/toplevel.cpp
    ${CORE_SRC_DIR}/msw/uiaction.cpp
    ${CORE_SRC_DIR}/msw/utilsgui.cpp
    ${CORE_SRC_DIR}/msw/utilswin.cpp
    ${CORE_SRC_DIR}/msw/uxtheme.cpp
    ${CORE_SRC_DIR}/msw/window.cpp
    ${CORE_SRC_DIR}/msw/joystick.cpp
    ${CORE_SRC_DIR}/msw/rt/notifmsgrt.cpp
    ${CORE_SRC_DIR}/msw/aboutdlg.cpp
    ${CORE_SRC_DIR}/msw/sound.cpp
    ${CORE_SRC_DIR}/msw/ole/uuid.cpp
    ${CORE_SRC_DIR}/msw/helpchm.cpp
    ${CORE_SRC_DIR}/common/taskbarcmn.cpp
    ${CORE_SRC_DIR}/msw/helpwin.cpp
    ${CORE_SRC_DIR}/msw/notifmsg.cpp
    ${CORE_SRC_DIR}/msw/ole/automtn.cpp
    ${CORE_SRC_DIR}/msw/taskbar.cpp
    ${CORE_SRC_DIR}/msw/richtooltip.cpp
    ${CORE_SRC_DIR}/msw/evtloop.cpp
    ${CORE_SRC_DIR}/msw/ole/access.cpp
)

set(MSW_SRC
    ${CORE_SRC_DIR}/generic/clrpickerg.cpp
    ${CORE_SRC_DIR}/generic/collpaneg.cpp
    ${CORE_SRC_DIR}/generic/filepickerg.cpp
    ${CORE_SRC_DIR}/generic/fontpickerg.cpp
    ${CORE_SRC_DIR}/generic/statusbr.cpp
    ${CORE_SRC_DIR}/generic/prntdlgg.cpp
    ${CORE_SRC_DIR}/msw/accel.cpp
    ${CORE_SRC_DIR}/msw/anybutton.cpp
    ${CORE_SRC_DIR}/msw/appprogress.cpp
    ${CORE_SRC_DIR}/msw/artmsw.cpp
    ${CORE_SRC_DIR}/msw/bmpbuttn.cpp
    ${CORE_SRC_DIR}/msw/button.cpp
    ${CORE_SRC_DIR}/msw/checkbox.cpp
    ${CORE_SRC_DIR}/msw/choice.cpp
    ${CORE_SRC_DIR}/msw/colordlg.cpp
    ${CORE_SRC_DIR}/msw/combo.cpp
    ${CORE_SRC_DIR}/msw/combobox.cpp
    ${CORE_SRC_DIR}/msw/control.cpp
    ${CORE_SRC_DIR}/msw/customdraw.cpp
    ${CORE_SRC_DIR}/msw/dialog.cpp
    ${CORE_SRC_DIR}/msw/dirdlg.cpp
    ${CORE_SRC_DIR}/msw/dragimag.cpp
    ${CORE_SRC_DIR}/msw/filedlg.cpp
    ${CORE_SRC_DIR}/msw/frame.cpp
    ${CORE_SRC_DIR}/msw/gauge.cpp
    ${CORE_SRC_DIR}/msw/headerctrl.cpp
    ${CORE_SRC_DIR}/msw/iniconf.cpp
    ${CORE_SRC_DIR}/msw/listbox.cpp
    ${CORE_SRC_DIR}/msw/listctrl.cpp
    ${CORE_SRC_DIR}/msw/mdi.cpp
    ${CORE_SRC_DIR}/msw/menu.cpp
    ${CORE_SRC_DIR}/msw/menuitem.cpp
    ${CORE_SRC_DIR}/msw/metafile.cpp
    ${CORE_SRC_DIR}/msw/msgdlg.cpp
    ${CORE_SRC_DIR}/msw/nativdlg.cpp
    ${CORE_SRC_DIR}/msw/nativewin.cpp
    ${CORE_SRC_DIR}/msw/notebook.cpp
    ${CORE_SRC_DIR}/msw/ownerdrw.cpp
    ${CORE_SRC_DIR}/msw/progdlg.cpp
    ${CORE_SRC_DIR}/msw/radiobox.cpp
    ${CORE_SRC_DIR}/msw/radiobut.cpp
    ${CORE_SRC_DIR}/msw/richmsgdlg.cpp
    ${CORE_SRC_DIR}/msw/scrolbar.cpp
    ${CORE_SRC_DIR}/msw/slider.cpp
    ${CORE_SRC_DIR}/msw/spinbutt.cpp
    ${CORE_SRC_DIR}/msw/spinctrl.cpp
    ${CORE_SRC_DIR}/msw/statbmp.cpp
    ${CORE_SRC_DIR}/msw/statbox.cpp
    ${CORE_SRC_DIR}/msw/statusbar.cpp
    ${CORE_SRC_DIR}/msw/statline.cpp
    ${CORE_SRC_DIR}/msw/stattext.cpp
    ${CORE_SRC_DIR}/msw/systhemectrl.cpp
    ${CORE_SRC_DIR}/msw/taskbarbutton.cpp
    ${CORE_SRC_DIR}/msw/toolbar.cpp
    ${CORE_SRC_DIR}/msw/textctrl.cpp
    ${CORE_SRC_DIR}/msw/textentry.cpp
    ${CORE_SRC_DIR}/msw/tglbtn.cpp
    ${CORE_SRC_DIR}/msw/treectrl.cpp
    ${CORE_SRC_DIR}/msw/bmpcbox.cpp
    ${CORE_SRC_DIR}/msw/datectrl.cpp
    ${CORE_SRC_DIR}/msw/calctrl.cpp
    ${CORE_SRC_DIR}/msw/datecontrols.cpp
    ${CORE_SRC_DIR}/msw/timectrl.cpp
    ${CORE_SRC_DIR}/msw/commandlinkbutton.cpp
    ${CORE_SRC_DIR}/msw/datetimectrl.cpp
    ${CORE_SRC_DIR}/msw/hyperlink.cpp
    ${CORE_SRC_DIR}/generic/activityindicator.cpp
)

set(MSW_DESKTOP_SRC
    ${CORE_SRC_DIR}/msw/checklst.cpp
    ${CORE_SRC_DIR}/msw/fdrepdlg.cpp
    ${CORE_SRC_DIR}/msw/fontdlg.cpp
)

if(UNIX)
    list(APPEND CORE_SRC ${UNIX_SRC})
    if(wxUSE_LIBSDL)
        list(APPEND CORE_SRC ${UNIX_SOUND_SDL_SRC})
    endif()
endif()

if(WX_WINDOWS)
    if(NOT wxUSE_OLE)
        wx_list_add_prefix(CORE_SRC "${wxSOURCE_DIR}/" "src/generic/dirdlgg.cpp")
    endif()
elseif(WXGTK)
    if(WXGTK2)
        wx_append_sources(CORE_SRC GTK2_LOWLEVEL)
        wx_append_sources(CORE_SRC GTK2)
    else()
        wx_append_sources(CORE_SRC GTK_LOWLEVEL)
        wx_append_sources(CORE_SRC GTK)
    endif()

    if(UNIX)
        wx_append_sources(CORE_SRC XWIN_LOWLEVEL)
    elseif(WIN32)
        list(APPEND CORE_SRC ${GTK_WIN32})
    endif()
elseif(WXOSX_COCOA)
    wx_append_sources(CORE_SRC BASE_AND_GUI_OSX_COCOA)
    wx_append_sources(CORE_SRC OSX_LOWLEVEL)
    wx_append_sources(CORE_SRC OSX_SHARED)
    wx_append_sources(CORE_SRC OSX_COCOA)
elseif(WXOSX_IPHONE)
    wx_append_sources(CORE_SRC OSX_LOWLEVEL)
    wx_append_sources(CORE_SRC OSX_IPHONE)
elseif(WXQT)
    wx_append_sources(CORE_SRC QT)
    if(WIN32)
        wx_append_sources(CORE_SRC QT_WIN32)
    endif()
endif()

add_library(wxcore 
    ${GUI_CMN_SRC}
    ${MSW_LOWLEVEL_SRC}
    ${MSW_DESKTOP_SRC}
    ${MSW_DESKTOP_LOWLEVEL_SRC}
    ${MSW_SRC}
)

target_include_directories(wxcore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxcore
    PRIVATE
        wxsharedcmn
        wxbase
        wxtype
        wxalgorithm
        stdlibcore
        Boost::nowide
        fmt::fmt
        Microsoft.GSL::GSL
        png
        ${JPEG_LIBRARIES}
        TIFF::TIFF

    # Microsoft Libraries
        mswutils

        kernel32
        user32
        comdlg32
        winspool
        shell32
        comctl32
        ole32
        oleacc
        uxtheme
        oleaut32
        uuid
        rpcrt4
        advapi32
        wininet
)

if(WIN32)
    target_link_libraries(wxcore
        PRIVATE
            winmm
            mswutils
    )
endif()

if(WXOSX_COCOA)
    wx_lib_link_libraries(wxcore PUBLIC "-framework AudioToolbox")
    if(wxUSE_WEBVIEW)
        wx_lib_link_libraries(wxcore PUBLIC "-framework WebKit")
    endif()
endif()

if(WXOSX_IPHONE)
    wx_lib_link_libraries(wxcore
        PUBLIC
        "-framework AudioToolbox"
        "-framework CoreGraphics"
        "-framework CoreText"
        "-framework UIKit"
        )
endif()

if(WXGTK AND wxUSE_PRIVATE_FONTS)
    wx_lib_include_directories(wxcore PUBLIC ${FONTCONFIG_INCLUDE_DIRS} ${PANGOFT2_INCLUDE_DIRS})
    wx_lib_link_libraries(wxcore PUBLIC ${FONTCONFIG_LIBRARIES} ${PANGOFT2_LIBRARIES})
endif()

if(wxUSE_LIBSDL)
    if(SDL2_FOUND)
        wx_lib_include_directories(wxcore PUBLIC ${SDL2_INCLUDE_DIR})
        wx_lib_link_libraries(wxcore PUBLIC ${SDL2_LIBRARY})
    elseif(SDL_FOUND)
        wx_lib_include_directories(wxcore PUBLIC ${SDL_INCLUDE_DIR})
        wx_lib_link_libraries(wxcore PUBLIC ${SDL_LIBRARY})
    endif()
endif()

if(wxUSE_LIBNOTIFY)
    wx_lib_include_directories(wxcore PUBLIC ${LIBNOTIFY_INCLUDE_DIRS})
    wx_lib_link_libraries(wxcore PUBLIC ${LIBNOTIFY_LIBRARIES})
endif()
