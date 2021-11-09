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

set(BASE_FILES ${BASE_CMN_SRC})
set(SHARED_BASE_CMN ${SHARED_BASE_CMN_SRC})
if(WIN32)
    list(APPEND BASE_FILES ${BASE_FILES} ${BASE_WIN32_SRC})
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
)

target_include_directories(wxbase
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_compile_options(wxbase PRIVATE -translateInclude)

add_library(wxsharedcmn
    OBJECT
        ${SHARED_BASE_CMN}
)

# target_compile_options(wxsharedcmn
#     PRIVATE
#         -translateInclude
# )

target_include_directories(wxsharedcmn
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxsharedcmn
    PRIVATE
        Microsoft.GSL::GSL
        #stdlibcore
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
        wxsharedcmn
        #stdlibcore
        Boost::nowide
        Microsoft.GSL::GSL
    
    # Windows libraries
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
