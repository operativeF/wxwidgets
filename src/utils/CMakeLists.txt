set(GeometrySrc
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/geometry.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/point.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/rect.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/size.ixx
)

add_library(wxutils
    STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/bitfield.ixx
        ${GeometrySrc}
)

target_include_directories(wxutils
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wxutils
    PRIVATE
        Microsoft.GSL::GSL
)
