

set(NET_UNIX_SRC
    ${CMAKE_SOURCE_DIR}/src/common/socketiohandler.cpp
    ${CMAKE_SOURCE_DIR}/src/unix/sockunix.cpp
)

set(NET_OSX_SRC
    ${CMAKE_SOURCE_DIR}src/osx/core/sockosx.cpp
    ${CMAKE_SOURCE_DIR}src/osx/webrequest_urlsession.mm
)

set(NET_WIN32_SRC
    ${CMAKE_SOURCE_DIR}/src/msw/sockmsw.cpp
    ${CMAKE_SOURCE_DIR}/src/msw/urlmsw.cpp
    ${CMAKE_SOURCE_DIR}/src/msw/webrequest_winhttp.cpp
)

set(NET_CMN_SRC
    ${CMAKE_SOURCE_DIR}/src/common/fs_inet.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ftp.cpp
    ${CMAKE_SOURCE_DIR}/src/common/http.cpp
    ${CMAKE_SOURCE_DIR}/src/common/protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/common/sckaddr.cpp
    ${CMAKE_SOURCE_DIR}/src/common/sckfile.cpp
    ${CMAKE_SOURCE_DIR}/src/common/sckipc.cpp
    ${CMAKE_SOURCE_DIR}/src/common/sckstrm.cpp
    ${CMAKE_SOURCE_DIR}/src/common/socket.cpp
    ${CMAKE_SOURCE_DIR}/src/common/url.cpp
    ${CMAKE_SOURCE_DIR}/src/common/webrequest.cpp
    ${CMAKE_SOURCE_DIR}/src/common/webrequest_curl.cpp
)

set(NET_CMN_HDR
    wx/fs_inet.h
    wx/protocol/file.h
    wx/protocol/ftp.h
    wx/protocol/http.h
    wx/protocol/log.h
    wx/protocol/protocol.h
    wx/sckaddr.h
    wx/sckipc.h
    wx/sckstrm.h
    wx/socket.h
    wx/url.h
    wx/webrequest.h
)

set(NET_FILES ${NET_CMN_SRC})

if(WIN32)
    list(APPEND NET_FILES ${NEW_WIN32_SRC})
elseif(APPLE)
    list(APPEND NET_FILES ${NET_OSX_SRC})
endif()

if(UNIX AND NOT WIN32)
    list(APPEND NET_FILES ${NET_UNIX_SRC})
endif()

add_library(wxnet
    ${NET_FILES}
)

target_include_directories(wxnet
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

if(WIN32)
    target_link_libraries(wxnet PRIVATE ws2_32)

    if(wxUSE_WEBREQUEST_WINHTTP)
        target_link_libraries(wxnet PRIVATE Winhttp)
    endif()
endif()

if (wxUSE_WEBREQUEST_CURL)
    target_include_directories(wxnet PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(wxnet PRIVATE ${CURL_LIBRARIES})
endif()
