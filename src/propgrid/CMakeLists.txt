set(PROPGRID_SRC
    ${CMAKE_SOURCE_DIR}/src/propgrid/advprops.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/editors.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/manager.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/property.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/propgrid.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/propgridiface.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/propgridpagestate.cpp
    ${CMAKE_SOURCE_DIR}/src/propgrid/props.cpp
)

add_library(propgrid
    ${PROPGRID_SRC}
)

target_include_directories(propgrid
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)
