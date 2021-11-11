# Basic types that are universally used in all libraries.

set(GeometrySrc
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/geometry.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/affinematrix2d.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/point.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/rect.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/rectaa.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/size.ixx
)

add_library(wxtype
    STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/bitfield.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/position.ixx
        ${GeometrySrc}
)

target_include_directories(wxtype
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxtype
    PRIVATE
        Microsoft.GSL::GSL
)
