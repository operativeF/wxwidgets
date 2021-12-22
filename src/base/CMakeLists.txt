#############################################################################
# Name:        build/cmake/lib/base/CMakeLists.txt
# Purpose:     CMake file for base library
# Author:      Tobias Taschner
# Created:     2016-09-20
# Copyright:   (c) 2016 wxWidgets development team
# Licence:     wxWindows licence
#############################################################################

if(wxBUILD_SHARED)
    set(wxBUILD_LIB_TYPE SHARED)
    if(WIN32)
        # Add WIN32 version information
        list(APPEND src_files "${wxSOURCE_DIR}/src/msw/version.rc" "${wxSOURCE_DIR}/include/wx/msw/genrcdefs.h")
    endif()
else()
    set(wxBUILD_LIB_TYPE STATIC)
endif()

if(${name} MATCHES "wx.*")
    string(SUBSTRING ${name} 2 -1 name_short)
else()
    set(name_short ${name})
endif()

set(BASE_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(BASE_CMN_SRC
    ${BASE_SRC_DIR}/common/any.cpp
    ${BASE_SRC_DIR}/common/appbase.cpp
    ${BASE_SRC_DIR}/common/arcall.cpp
    ${BASE_SRC_DIR}/common/arcfind.cpp
    ${BASE_SRC_DIR}/common/archive.cpp
    ${BASE_SRC_DIR}/common/arrstr.cpp
    ${BASE_SRC_DIR}/common/base64.cpp
    ${BASE_SRC_DIR}/common/clntdata.cpp
    ${BASE_SRC_DIR}/common/cmdline.cpp
    ${BASE_SRC_DIR}/common/config.cpp
    ${BASE_SRC_DIR}/common/convauto.cpp
    ${BASE_SRC_DIR}/common/datetime.cpp
    ${BASE_SRC_DIR}/common/datetimefmt.cpp
    ${BASE_SRC_DIR}/common/datstrm.cpp
    ${BASE_SRC_DIR}/common/dircmn.cpp
    ${BASE_SRC_DIR}/common/dynlib.cpp
    ${BASE_SRC_DIR}/common/dynload.cpp
    ${BASE_SRC_DIR}/common/encconv.cpp
    ${BASE_SRC_DIR}/common/evtloopcmn.cpp
    ${BASE_SRC_DIR}/common/ffile.cpp
    ${BASE_SRC_DIR}/common/file.cpp
    ${BASE_SRC_DIR}/common/fileback.cpp
    ${BASE_SRC_DIR}/common/fileconf.cpp
    ${BASE_SRC_DIR}/common/filefn.cpp
    ${BASE_SRC_DIR}/common/filename.cpp
    ${BASE_SRC_DIR}/common/filesys.cpp
    ${BASE_SRC_DIR}/common/filtall.cpp
    ${BASE_SRC_DIR}/common/filtfind.cpp
    ${BASE_SRC_DIR}/common/fmapbase.cpp
    ${BASE_SRC_DIR}/common/fs_arc.cpp
    ${BASE_SRC_DIR}/common/fs_filter.cpp
    ${BASE_SRC_DIR}/common/hash.cpp
    ${BASE_SRC_DIR}/common/hashmap.cpp
    ${BASE_SRC_DIR}/common/init.cpp
    ${BASE_SRC_DIR}/common/intl.cpp
    ${BASE_SRC_DIR}/common/ipcbase.cpp
    ${BASE_SRC_DIR}/common/languageinfo.cpp
    ${BASE_SRC_DIR}/common/list.cpp
    ${BASE_SRC_DIR}/common/log.cpp
    ${BASE_SRC_DIR}/common/longlong.cpp
    ${BASE_SRC_DIR}/common/memory.cpp
    ${BASE_SRC_DIR}/common/mimecmn.cpp
    ${BASE_SRC_DIR}/common/module.cpp
    ${BASE_SRC_DIR}/common/mstream.cpp
    ${BASE_SRC_DIR}/common/numformatter.cpp
    ${BASE_SRC_DIR}/common/object.cpp
    ${BASE_SRC_DIR}/common/platinfo.cpp
    ${BASE_SRC_DIR}/common/powercmn.cpp
    ${BASE_SRC_DIR}/common/process.cpp
    ${BASE_SRC_DIR}/common/regex.cpp
    ${BASE_SRC_DIR}/common/secretstore.cpp
    ${BASE_SRC_DIR}/common/stdpbase.cpp
    ${BASE_SRC_DIR}/common/sstream.cpp
    ${BASE_SRC_DIR}/common/stopwatch.cpp
    ${BASE_SRC_DIR}/common/strconv.cpp
    ${BASE_SRC_DIR}/common/stream.cpp
    ${BASE_SRC_DIR}/common/string.cpp
    ${BASE_SRC_DIR}/common/stringimpl.cpp
    ${BASE_SRC_DIR}/common/stringops.cpp
    ${BASE_SRC_DIR}/common/strvararg.cpp
    ${BASE_SRC_DIR}/common/sysopt.cpp
    ${BASE_SRC_DIR}/common/textbuf.cpp
    ${BASE_SRC_DIR}/common/textfile.cpp
    ${BASE_SRC_DIR}/common/threadinfo.cpp
    ${BASE_SRC_DIR}/common/time.cpp
    ${BASE_SRC_DIR}/common/timercmn.cpp
    ${BASE_SRC_DIR}/common/timerimpl.cpp
    ${BASE_SRC_DIR}/common/tokenzr.cpp
    ${BASE_SRC_DIR}/common/translation.cpp
    ${BASE_SRC_DIR}/common/txtstrm.cpp
    ${BASE_SRC_DIR}/common/unichar.cpp
    ${BASE_SRC_DIR}/common/uri.cpp
    ${BASE_SRC_DIR}/common/ustring.cpp
    ${BASE_SRC_DIR}/common/variant.cpp
    ${BASE_SRC_DIR}/common/wfstream.cpp
    ${BASE_SRC_DIR}/common/wxcrt.cpp
    ${BASE_SRC_DIR}/common/wxprintf.cpp
    ${BASE_SRC_DIR}/common/xlocale.cpp
    ${BASE_SRC_DIR}/common/xti.cpp
    ${BASE_SRC_DIR}/common/xtistrm.cpp
    ${BASE_SRC_DIR}/common/zipstrm.cpp
    ${BASE_SRC_DIR}/common/zstream.cpp
    ${BASE_SRC_DIR}/common/fswatchercmn.cpp
    ${BASE_SRC_DIR}/generic/fswatcherg.cpp
    ${BASE_SRC_DIR}/common/lzmastream.cpp
)

set(BASE_CMN_HDR
)

if(wxUSE_TARSTREAM)
    list(APPEND BASE_CMN_SRC ${BASE_SRC_DIR}/common/tarstrm.cpp)
    list(APPEND BASE_CMN_HDR ${CMAKE_SOURCE_DIR}/include/wx/tarstrm.h)
endif()

set(BASE_CMN_IXX
    ${CMAKE_SOURCE_DIR}/modules/base/bitfield.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/position.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/math.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/algorithm/charutils.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/algorithm/stringutils.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/algorithm/strings/nonmodifying.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/algorithm/strings/modifying.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/algorithm/strings/unsafe.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/algorithm/strings/tokenzr.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/settings.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/versioninfo.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/platinfo.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/textfile.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/txtstrm.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/base64.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/sysopt.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/ffile.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/cmdline.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/textbuf.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/encconv.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/kbdstate.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/mousestate.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/clntdata.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/fs_filter.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/fs_arc.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/stream.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/fileflags.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/convauto.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/fontenc.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/sstream.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/wfstream.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/mstream.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/datstrm.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/zstream.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/zipstrm.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/archive.ixx
)

set(SHARED_BASE_CMN_SRC
    ${BASE_SRC_DIR}/common/event.cpp
    ${BASE_SRC_DIR}/common/fs_mem.cpp
    ${BASE_SRC_DIR}/common/msgout.cpp
    ${BASE_SRC_DIR}/common/utilscmn.cpp
)

set(BASE_WIN32_SRC
    ${BASE_SRC_DIR}/msw/basemsw.cpp
    ${BASE_SRC_DIR}/msw/crashrpt.cpp
    ${BASE_SRC_DIR}/msw/debughlp.cpp
    ${BASE_SRC_DIR}/msw/dde.cpp
    ${BASE_SRC_DIR}/msw/dir.cpp
    ${BASE_SRC_DIR}/msw/dlmsw.cpp
    ${BASE_SRC_DIR}/msw/evtloopconsole.cpp
    ${BASE_SRC_DIR}/msw/mimetype.cpp
    ${BASE_SRC_DIR}/msw/power.cpp
    ${BASE_SRC_DIR}/msw/regconf.cpp # Do not include unless registry is enabled.
    ${BASE_SRC_DIR}/msw/registry.cpp # Do not include unless registry is enabled.
    ${BASE_SRC_DIR}/msw/secretstore.cpp
    ${BASE_SRC_DIR}/msw/snglinst.cpp
    ${BASE_SRC_DIR}/msw/stackwalk.cpp
    ${BASE_SRC_DIR}/msw/stdpaths.cpp
    ${BASE_SRC_DIR}/msw/thread.cpp
    ${BASE_SRC_DIR}/msw/timer.cpp
    ${BASE_SRC_DIR}/msw/utils.cpp
    ${BASE_SRC_DIR}/msw/utilsexc.cpp
    ${BASE_SRC_DIR}/msw/fswatcher.cpp
)

set(SHARED_BASE_WIN32_CMN
    ${BASE_SRC_DIR}/msw/main.cpp
    ${BASE_SRC_DIR}/msw/volume.cpp
)

set(BASE_OSX_SHARED_SRC
    ${BASE_SRC_DIR}/osx/core/mimetype.cpp
    ${BASE_SRC_DIR}/osx/fswatcher_fsevents.cpp
    ${BASE_SRC_DIR}/osx/cocoa/stdpaths.mm
    ${BASE_COREFOUNDATION_SRC}
    ${BASE_UNIX_AND_DARWIN_SRC}
)

set(BASE_AND_GUI_OSX_COCOA_SRC
    ${BASE_SRC_DIR}/osx/cocoa/utils.mm
    ${BASE_SRC_DIR}/osx/cocoa/power.mm
)

set(BASE_UNIX_SRC
    ${BASE_UNIX_AND_DARWIN_NOTWXMAC_SRC}
    ${BASE_SRC_DIR}/unix/fswatcher_inotify.cpp
    ${BASE_SRC_DIR}/unix/secretstore.cpp
    ${BASE_SRC_DIR}/unix/stdpaths.cpp
)

set(GEOMETRY_IXX
    ${CMAKE_SOURCE_DIR}/modules/base/geometry/geometry.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/geometry/affinematrix2d.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/geometry/point.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/geometry/rect.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/geometry/rectaa.ixx
    ${CMAKE_SOURCE_DIR}/modules/base/geometry/size.ixx
)

set(CONFIG_FLAGS_IXX
    ${CMAKE_SOURCE_DIR}/modules/config/flags.ixx
)

set(UNIVERSAL_UTILS_IXX
    ${CMAKE_SOURCE_DIR}/modules/utils/cast.ixx
)

set(WIN32_WRAP_IXX
    ${CMAKE_SOURCE_DIR}/modules/win/windef.ixx
    ${CMAKE_SOURCE_DIR}/modules/win/unique_hnd.ixx
)

set(IMAGE_HANDLERS_IXX
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagani.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagbmp.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagcur.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imaggif.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagiff.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagico.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagjpeg.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagpcx.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagpng.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagpnm.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagtga.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagtiff.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/handlers/imagxpm.ixx

    ${CMAKE_SOURCE_DIR}/modules/image/imagebase.ixx

    ${CMAKE_SOURCE_DIR}/modules/image/image.ixx
)

set(IMAGE_HANDLERS_SRC
    ${CMAKE_SOURCE_DIR}/src/common/imagebase.cpp
)

set(IMAGE_DECODERS_IXX
    ${CMAKE_SOURCE_DIR}/modules/image/decoders/anidecod.ixx
    ${CMAKE_SOURCE_DIR}/modules/image/decoders/gifdecod.ixx
)

set(PRINTER_IXX
    ${CMAKE_SOURCE_DIR}/src/print/print.ixx
    ${CMAKE_SOURCE_DIR}/src/print/printercfg.ixx
)

set(BASE_FILES
    ${BASE_CMN_SRC}
    ${CONFIG_FLAGS_IXX}
    ${BASE_CMN_IXX}
    ${GEOMETRY_IXX}
    ${PRINTER_IXX}
    ${IMAGE_DECODERS_IXX}
)

set(SHARED_BASE_CMN ${SHARED_BASE_CMN_SRC})
if(WIN32)
    list(APPEND BASE_FILES ${BASE_FILES} ${BASE_WIN32_SRC} ${WIN32_WRAP_IXX})
    list(APPEND SHARED_BASE_CMN ${SHARED_BASE_CMN} ${SHARED_BASE_WIN32_CMN})
elseif(APPLE)
    set(BASE_FILES ${BASE_OSX_SHARED})
    if(WXOSX_COCOA)
        set(BASE_FILES ${BASE_AND_GUI_OSX_COCOA})
    endif()
elseif(UNIX)
    set(BASE_FILES ${BASE_UNIX})
endif()

add_library(wxbase
    ${BASE_FILES}
    ${IMAGE_HANDLERS_IXX}
    ${IMAGE_HANDLERS_SRC}
    ${SHARED_BASE_CMN}
    ${UNIVERSAL_UTILS_IXX}
)

target_include_directories(wxbase
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

if(wxUSE_ZLIB)
    target_include_directories(wxbase PRIVATE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(wxbase PRIVATE ${ZLIB_LIBRARIES})
endif()

if(wxUSE_REGEX)
    target_include_directories(wxbase PRIVATE ${REGEX_INCLUDE_DIRS})
    target_link_libraries(wxbase PRIVATE ${REGEX_LIBRARIES})
endif()

if(wxUSE_LIBLZMA)
    target_include_directories(wxbase PRIVATE ${LIBLZMA_INCLUDE_DIRS})
    target_link_libraries(wxbase PRIVATE ${LIBLZMA_LIBRARIES})
endif()

if(UNIX AND wxUSE_SECRETSTORE)
    target_include_directories(wxbase PRIVATE ${LIBSECRET_INCLUDE_DIRS})
    target_link_libraries(wxbase PRIVATE ${LIBSECRET_LIBRARIES})
endif()

if(wxUSE_LIBICONV)
    target_include_directories(wxbase PRIVATE ${ICONV_INCLUDE_DIR})
    target_link_libraries(wxbase PRIVATE ${ICONV_LIBRARIES})
endif()

if(wxUSE_THREADS AND CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(wxbase PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endif()

if(APPLE)
    target_link_libraries(wxbase
        PUBLIC
        "-framework CoreFoundation"
    )
    if(WXOSX_COCOA)
        target_link_libraries(wxbase
            PRIVATE
            "-framework Security"
            PUBLIC
            "-framework Carbon"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework QuartzCore"
        )
    endif()
elseif(UNIX)
    target_link_libraries(wxbase PRIVATE dl)
endif()

target_link_libraries(wxbase
    PRIVATE
        stdlibcore
        Boost::nowide
        boost_tmp
    # Windows libraries
        opengl32
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

target_precompile_headers(wxbase PRIVATE ${CMAKE_SOURCE_DIR}/include/wx/msw/winprecomp.h)
