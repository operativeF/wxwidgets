
set(HTML_MSW_SRC
    ${CMAKE_SOURCE_DIR}/src/msw/helpbest.cpp
    ${CMAKE_SOURCE_DIR}/src/html/chm.cpp
)

set(HTML_CMN_SRC
    ${CMAKE_SOURCE_DIR}/src/html/helpctrl.cpp
    ${CMAKE_SOURCE_DIR}/src/html/helpdata.cpp
    ${CMAKE_SOURCE_DIR}/src/html/helpdlg.cpp
    ${CMAKE_SOURCE_DIR}/src/html/helpfrm.cpp
    ${CMAKE_SOURCE_DIR}/src/html/helpwnd.cpp
    ${CMAKE_SOURCE_DIR}/src/html/htmlcell.cpp
    ${CMAKE_SOURCE_DIR}/src/html/htmlfilt.cpp
    ${CMAKE_SOURCE_DIR}/src/html/htmlpars.cpp
    ${CMAKE_SOURCE_DIR}/src/html/htmltag.cpp
    ${CMAKE_SOURCE_DIR}/src/html/htmlwin.cpp
    ${CMAKE_SOURCE_DIR}/src/html/htmprint.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_dflist.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_fonts.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_hline.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_image.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_layout.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_links.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_list.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_pre.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_span.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_style.cpp
    ${CMAKE_SOURCE_DIR}/src/html/m_tables.cpp
    ${CMAKE_SOURCE_DIR}/src/html/styleparams.cpp
    ${CMAKE_SOURCE_DIR}/src/html/winpars.cpp
    # wxHTML users:
    ${CMAKE_SOURCE_DIR}/src/generic/htmllbox.cpp
)

if(WIN32 OR wxUSE_LIBMSPACK)
    set(HTML_WIN_LIBMS_FILES ${HTML_MSW_SRC})
endif()

add_library(wxhtml
    ${HTML_CMN_SRC}
    ${HTML_WIN_LIBMS_FILES}
)

if(wxUSE_LIBMSPACK)
    target_include_directories(wxhtml PRIVATE ${MSPACK_INCLUDE_DIRS})
    target_link_libraries(wxhtml PRIVATE ${MSPACK_LIBRARIES})
endif()

target_include_directories(wxhtml
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)
