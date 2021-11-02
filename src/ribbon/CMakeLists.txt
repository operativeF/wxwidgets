set(RIBBON_SRC
    ${CMAKE_SOURCE_DIR}/src/ribbon/art_internal.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/art_msw.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/art_aui.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/bar.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/buttonbar.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/control.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/gallery.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/page.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/panel.cpp
    ${CMAKE_SOURCE_DIR}/src/ribbon/toolbar.cpp
)

if(wxUSE_XRC)
    set(RIBBON_XRC_SRC ${CMAKE_SOURCE_DIR}/src/xrc/xh_ribbon.cpp)
endif()

add_library(wxribbon
    ${RIBBON_SRC}
    ${RIBBON_XRC_SRC}
)

target_include_directories(wxribbon
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxribbon
    PRIVATE
        Microsoft.GSL::GSL
)