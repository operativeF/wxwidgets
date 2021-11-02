set(RICHTEXT_SRC
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextbuffer.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextctrl.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextformatdlg.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtexthtml.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextimagedlg.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextprint.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextstyledlg.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextstyles.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextsymboldlg.cpp
    ${CMAKE_SOURCE_DIR}/src/richtext/richtextxml.cpp
)

if(wxUSE_XRC)
    set(RICHTEXT_XRC_SRC ${CMAKE_SOURCE_DIR}/src/xrc/xh_richtext.cpp)
endif()

add_library(wxrichtext
    ${RICHTEXT_SRC}
    ${RICHTEXT_XRC_SRC}
)

add_dependencies(wxrichtext
    wxhtml
    wxxml
)

target_include_directories(wxrichtext
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxrichtext
    PRIVATE
        wxhtml
        wxxml
        Microsoft.GSL::GSL
)
