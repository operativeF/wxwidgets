
set(CORE_GRID_SRC
    ${CMAKE_SOURCE_DIR}/src/generic/grideditors.cpp
    ${CMAKE_SOURCE_DIR}/src/generic/gridctrl.cpp
    ${CMAKE_SOURCE_DIR}/src/generic/grid.cpp
    ${CMAKE_SOURCE_DIR}/src/generic/gridsel.cpp
)

set(CORE_GRID_IXX
    ${CMAKE_SOURCE_DIR}/modules/grid/grid.ixx
    ${CMAKE_SOURCE_DIR}/modules/grid/gridcoord.ixx
    ${CMAKE_SOURCE_DIR}/modules/grid/gridctrl.ixx
    ${CMAKE_SOURCE_DIR}/modules/grid/grideditors.ixx
    ${CMAKE_SOURCE_DIR}/modules/grid/gridevt.ixx
    ${CMAKE_SOURCE_DIR}/modules/grid/gridinf.ixx
    ${CMAKE_SOURCE_DIR}/modules/grid/gridsel.ixx
)

add_library(wxgrid
    ${CORE_GRID_SRC}
    ${CORE_GRID_IXX}
)

target_include_directories(wxgrid
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(wxgrid
    PUBLIC
        wxbase

    PRIVATE
        stdlibcore
)
