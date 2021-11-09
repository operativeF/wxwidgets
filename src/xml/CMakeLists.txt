find_package(expat CONFIG REQUIRED)

set(XML_SRC
    ${CMAKE_SOURCE_DIR}/src/xml/xml.cpp
    ${CMAKE_SOURCE_DIR}/src/common/xtixml.cpp # FIXME - temporary solution
)

add_library(wxxml
    ${XML_SRC}
)

target_link_libraries(wxxml
    PRIVATE
        stdlibcore
        Microsoft.GSL::GSL
        expat::expat
)

target_include_directories(wxxml
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)
