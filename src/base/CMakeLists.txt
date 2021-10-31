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
set(BASE_INC_DIR ${CMAKE_SOURCE_DIR}/include)

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
    ${BASE_SRC_DIR}/common/stdstream.cpp
    ${BASE_SRC_DIR}/common/stopwatch.cpp
    ${BASE_SRC_DIR}/common/strconv.cpp
    ${BASE_SRC_DIR}/common/stream.cpp
    ${BASE_SRC_DIR}/common/string.cpp
    ${BASE_SRC_DIR}/common/stringimpl.cpp
    ${BASE_SRC_DIR}/common/stringops.cpp
    ${BASE_SRC_DIR}/common/strvararg.cpp
    ${BASE_SRC_DIR}/common/sysopt.cpp
    ${BASE_SRC_DIR}/common/tarstrm.cpp
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
    ${BASE_INC_DIR}/wx/any.h
    ${BASE_INC_DIR}/wx/anystr.h
    ${BASE_INC_DIR}/wx/app.h
    ${BASE_INC_DIR}/wx/apptrait.h
    ${BASE_INC_DIR}/wx/archive.h
    ${BASE_INC_DIR}/wx/arrimpl.cpp
    ${BASE_INC_DIR}/wx/arrstr.h
    ${BASE_INC_DIR}/wx/base64.h
    ${BASE_INC_DIR}/wx/buffer.h
    ${BASE_INC_DIR}/wx/build.h
    ${BASE_INC_DIR}/wx/chartype.h
    ${BASE_INC_DIR}/wx/checkeddelete.h
    ${BASE_INC_DIR}/wx/clntdata.h
    ${BASE_INC_DIR}/wx/cmdargs.h
    ${BASE_INC_DIR}/wx/cmdline.h
    ${BASE_INC_DIR}/wx/compiler.h
    ${BASE_INC_DIR}/wx/confbase.h
    ${BASE_INC_DIR}/wx/config.h
    ${BASE_INC_DIR}/wx/convauto.h
    ${BASE_INC_DIR}/wx/containr.h
    ${BASE_INC_DIR}/wx/cpp.h
    ${BASE_INC_DIR}/wx/crt.h
    ${BASE_INC_DIR}/wx/datetime.h
    ${BASE_INC_DIR}/wx/datstrm.h
    ${BASE_INC_DIR}/wx/dde.h
    ${BASE_INC_DIR}/wx/debug.h
    ${BASE_INC_DIR}/wx/defs.h
    ${BASE_INC_DIR}/wx/dir.h
    ${BASE_INC_DIR}/wx/dlimpexp.h
    ${BASE_INC_DIR}/wx/dlist.h
    ${BASE_INC_DIR}/wx/dynarray.h
    ${BASE_INC_DIR}/wx/dynlib.h
    ${BASE_INC_DIR}/wx/dynload.h
    ${BASE_INC_DIR}/wx/encconv.h
    ${BASE_INC_DIR}/wx/event.h
    ${BASE_INC_DIR}/wx/eventfilter.h
    ${BASE_INC_DIR}/wx/evtloop.h
    ${BASE_INC_DIR}/wx/evtloopsrc.h
    ${BASE_INC_DIR}/wx/except.h
    ${BASE_INC_DIR}/wx/features.h
    ${BASE_INC_DIR}/wx/flags.h
    ${BASE_INC_DIR}/wx/ffile.h
    ${BASE_INC_DIR}/wx/file.h
    ${BASE_INC_DIR}/wx/fileconf.h
    ${BASE_INC_DIR}/wx/filefn.h
    ${BASE_INC_DIR}/wx/filename.h
    ${BASE_INC_DIR}/wx/filesys.h
    ${BASE_INC_DIR}/wx/fontenc.h
    ${BASE_INC_DIR}/wx/fontmap.h
    ${BASE_INC_DIR}/wx/fs_arc.h
    ${BASE_INC_DIR}/wx/fs_filter.h
    ${BASE_INC_DIR}/wx/fs_mem.h
    ${BASE_INC_DIR}/wx/fs_zip.h
    ${BASE_INC_DIR}/wx/hash.h
    ${BASE_INC_DIR}/wx/hashmap.h
    ${BASE_INC_DIR}/wx/hashset.h
    ${BASE_INC_DIR}/wx/iconloc.h
    ${BASE_INC_DIR}/wx/init.h
    ${BASE_INC_DIR}/wx/intl.h
    ${BASE_INC_DIR}/wx/ipc.h
    ${BASE_INC_DIR}/wx/ipcbase.h
    ${BASE_INC_DIR}/wx/kbdstate.h
    ${BASE_INC_DIR}/wx/language.h
    ${BASE_INC_DIR}/wx/link.h
    ${BASE_INC_DIR}/wx/list.h
    ${BASE_INC_DIR}/wx/listimpl.cpp
    ${BASE_INC_DIR}/wx/log.h
    ${BASE_INC_DIR}/wx/longlong.h
    ${BASE_INC_DIR}/wx/math.h
    ${BASE_INC_DIR}/wx/memconf.h
    ${BASE_INC_DIR}/wx/memory.h
    ${BASE_INC_DIR}/wx/memtext.h
    ${BASE_INC_DIR}/wx/mimetype.h
    ${BASE_INC_DIR}/wx/module.h
    ${BASE_INC_DIR}/wx/mousestate.h
    ${BASE_INC_DIR}/wx/msgout.h
    ${BASE_INC_DIR}/wx/msgqueue.h
    ${BASE_INC_DIR}/wx/mstream.h
    ${BASE_INC_DIR}/wx/numformatter.h
    ${BASE_INC_DIR}/wx/object.h
    ${BASE_INC_DIR}/wx/platform.h
    ${BASE_INC_DIR}/wx/platinfo.h
    ${BASE_INC_DIR}/wx/power.h
    ${BASE_INC_DIR}/wx/process.h
    ${BASE_INC_DIR}/wx/ptr_scpd.h
    ${BASE_INC_DIR}/wx/recguard.h
    ${BASE_INC_DIR}/wx/regex.h
    ${BASE_INC_DIR}/wx/rtti.h
    ${BASE_INC_DIR}/wx/scopedarray.h
    ${BASE_INC_DIR}/wx/scopedptr.h
    ${BASE_INC_DIR}/wx/scopeguard.h
    ${BASE_INC_DIR}/wx/secretstore.h
    ${BASE_INC_DIR}/wx/snglinst.h
    ${BASE_INC_DIR}/wx/sstream.h
    ${BASE_INC_DIR}/wx/stackwalk.h
    ${BASE_INC_DIR}/wx/stdpaths.h
    ${BASE_INC_DIR}/wx/stdstream.h
    ${BASE_INC_DIR}/wx/stockitem.h
    ${BASE_INC_DIR}/wx/stopwatch.h
    ${BASE_INC_DIR}/wx/strconv.h
    ${BASE_INC_DIR}/wx/stream.h
    ${BASE_INC_DIR}/wx/string.h
    ${BASE_INC_DIR}/wx/stringimpl.h
    ${BASE_INC_DIR}/wx/stringops.h
    ${BASE_INC_DIR}/wx/strvararg.h
    ${BASE_INC_DIR}/wx/sysopt.h
    ${BASE_INC_DIR}/wx/tarstrm.h
    ${BASE_INC_DIR}/wx/textbuf.h
    ${BASE_INC_DIR}/wx/textfile.h
    ${BASE_INC_DIR}/wx/thread.h
    ${BASE_INC_DIR}/wx/thrimpl.cpp
    ${BASE_INC_DIR}/wx/time.h
    ${BASE_INC_DIR}/wx/timer.h
    ${BASE_INC_DIR}/wx/tls.h
    ${BASE_INC_DIR}/wx/tokenzr.h
    ${BASE_INC_DIR}/wx/tracker.h
    ${BASE_INC_DIR}/wx/translation.h
    ${BASE_INC_DIR}/wx/txtstrm.h
    ${BASE_INC_DIR}/wx/typeinfo.h
    ${BASE_INC_DIR}/wx/types.h
    ${BASE_INC_DIR}/wx/unichar.h
    ${BASE_INC_DIR}/wx/uri.h
    ${BASE_INC_DIR}/wx/ustring.h
    ${BASE_INC_DIR}/wx/utils.h
    ${BASE_INC_DIR}/wx/variant.h
    ${BASE_INC_DIR}/wx/vector.h
    ${BASE_INC_DIR}/wx/version.h
    ${BASE_INC_DIR}/wx/versioninfo.h
    ${BASE_INC_DIR}/wx/volume.h
    ${BASE_INC_DIR}/wx/weakref.h
    ${BASE_INC_DIR}/wx/wfstream.h
    ${BASE_INC_DIR}/wx/wx.h
    ${BASE_INC_DIR}/wx/wxchar.h
    ${BASE_INC_DIR}/wx/wxcrt.h
    ${BASE_INC_DIR}/wx/wxcrtbase.h
    ${BASE_INC_DIR}/wx/wxcrtvararg.h
    ${BASE_INC_DIR}/wx/xlocale.h
    ${BASE_INC_DIR}/wx/xti.h
    ${BASE_INC_DIR}/wx/xti2.h
    ${BASE_INC_DIR}/wx/xtistrm.h
    ${BASE_INC_DIR}/wx/xtictor.h
    ${BASE_INC_DIR}/wx/xtihandler.h
    ${BASE_INC_DIR}/wx/xtiprop.h
    ${BASE_INC_DIR}/wx/xtitypes.h
    ${BASE_INC_DIR}/wx/zipstrm.h
    ${BASE_INC_DIR}/wx/zstream.h
    ${BASE_INC_DIR}/wx/meta/convertible.h
    ${BASE_INC_DIR}/wx/meta/if.h
    ${BASE_INC_DIR}/wx/meta/implicitconversion.h
    ${BASE_INC_DIR}/wx/meta/int2type.h
    ${BASE_INC_DIR}/wx/meta/movable.h
    ${BASE_INC_DIR}/wx/meta/pod.h
    ${BASE_INC_DIR}/wx/meta/removeref.h
    ${BASE_INC_DIR}/wx/fswatcher.h
    ${BASE_INC_DIR}/wx/generic/fswatcher.h
    ${BASE_INC_DIR}/wx/lzmastream.h
)

set(BASE_AND_GUI_CMN_SRC
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
    ${BASE_SRC_DIR}/msw/regconf.cpp
    ${BASE_SRC_DIR}/msw/registry.cpp
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

set(BASE_WIN32_HDR
    ${BASE_INC_DIR}/wx/msw/apptrait.h
    ${BASE_INC_DIR}/wx/msw/apptbase.h
    ${BASE_INC_DIR}/wx/msw/crashrpt.h
    ${BASE_INC_DIR}/wx/msw/dde.h
    ${BASE_INC_DIR}/wx/msw/debughlp.h
    ${BASE_INC_DIR}/wx/msw/evtloopconsole.h
    ${BASE_INC_DIR}/wx/msw/gccpriv.h
    ${BASE_INC_DIR}/wx/msw/libraries.h
    ${BASE_INC_DIR}/wx/msw/mimetype.h
    ${BASE_INC_DIR}/wx/msw/private.h
    ${BASE_INC_DIR}/wx/msw/regconf.h
    ${BASE_INC_DIR}/wx/msw/registry.h
    ${BASE_INC_DIR}/wx/msw/seh.h
    ${BASE_INC_DIR}/wx/msw/stackwalk.h
    ${BASE_INC_DIR}/wx/msw/stdpaths.h
    ${BASE_INC_DIR}/wx/msw/winver.h
    ${BASE_INC_DIR}/wx/msw/wrapcctl.h
    ${BASE_INC_DIR}/wx/msw/wrapcdlg.h
    ${BASE_INC_DIR}/wx/msw/wrapwin.h
    ${BASE_INC_DIR}/wx/msw/fswatcher.h
)

set(BASE_AND_GUI_WIN32_SRC
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

set(BASE_FILES ${BASE_CMN_SRC})

list(APPEND BASE_FILES ${BASE_AND_GUI_CMN_SRC})

set(BASE_HDR ${BASE_CMN_HDR})

if(WIN32)
    list(APPEND BASE_FILES ${BASE_WIN32_SRC} ${BASE_AND_GUI_WIN32_SRC})
    list(APPEND BASE_HDR ${BASE_WIN32_HDR})
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
)

target_include_directories(wxbase
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
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
        Boost::nowide
        fmt::fmt
        Microsoft.GSL::GSL
)
