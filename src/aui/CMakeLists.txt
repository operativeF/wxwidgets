set(AUI_CMN_SRC
    ${CMAKE_SOURCE_DIR}/src/aui/framemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/dockart.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/floatpane.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/auibook.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/auibar.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/tabmdi.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/tabart.cpp
)

if(wxUSE_XRC)
    set(AUI_XRC_CMN ${CMAKE_SOURCE_DIR}/src/xrc/xh_aui.cpp
                    ${CMAKE_SOURCE_DIR}/src/xrc/xh_auitoolb.cpp)
endif()

set(AUI_MSW_SRC
    ${CMAKE_SOURCE_DIR}/src/aui/tabartmsw.cpp
    ${CMAKE_SOURCE_DIR}/src/aui/barartmsw.cpp
)

set(AUI_GTK_SRC
    ${CMAKE_SOURCE_DIR}/src/aui/tabartgtk.cpp
)

set(AUI_FILES ${AUI_CMN_SRC})

if(WIN32)
    list(APPEND AUI_FILES ${AUI_MSW_SRC})
elseif(WXGTK2)
    list(APPEND AUI_FILES ${AUI_GTK_SRC})
endif()

add_library(wxaui
    ${AUI_FILES}
    ${AUI_XRC_CMN}
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
        stdlibcore
        wxtype
        Microsoft.GSL::GSL
)
