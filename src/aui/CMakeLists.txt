set(AUI_CMN_SRC
    ${CMAKE_SOURCE_DIR}/src/aui/framemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/dockart.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/floatpane.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/book.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/auibar.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/tabmdi.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/tabart.cpp
)

set(AUI_CMN_INF
    ${CMAKE_SOURCE_DIR}/modules/aui/book.ixx
    ${CMAKE_SOURCE_DIR}/modules/aui/dockart.ixx
    ${CMAKE_SOURCE_DIR}/modules/aui/floatpane.ixx
    ${CMAKE_SOURCE_DIR}/modules/aui/tabmdi.ixx
)

if(wxUSE_MDI)
    set(AUI_MDI_SRC ${CMAKE_SOURCE_DIR}/src/aui/tabmdi.cpp)
endif()

if(wxUSE_UXTHEME AND WIN32)
    set(AUI_BAR_ART_SRC ${CMAKE_SOURCE_DIR}/src/aui/msw/barart.cpp)
endif()

if(wxUSE_XRC)
    set(AUI_XRC_CMN ${CMAKE_SOURCE_DIR}/src/xrc/xh_aui.cpp
                    ${CMAKE_SOURCE_DIR}/src/xrc/xh_auitoolb.cpp)
endif()

set(AUI_MSW_SRC
    ${CMAKE_SOURCE_DIR}/src/aui/msw/tabart.cpp
)

set(AUI_GTK_SRC
    ${CMAKE_SOURCE_DIR}/src/aui/gtk/tabart.cpp
)

set(AUI_FILES ${AUI_CMN_SRC})

if(WIN32)
    list(APPEND AUI_FILES ${AUI_MSW_SRC} ${AUI_BAR_ART_SRC} ${AUI_CMN_INF})
elseif(WXGTK2)
    list(APPEND AUI_FILES ${AUI_GTK_SRC})
endif()

add_library(wxaui
    ${AUI_FILES}
    ${AUI_XRC_CMN}
    ${AUI_MDI_SRC}
)

target_include_directories(wxaui
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxaui
    PRIVATE
        wxalgorithm
        mswutils
        stdlibcore
        wxtype
        Microsoft.GSL::GSL
)

target_precompile_headers(wxaui
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/wx/msw/winprecomp.h
)
