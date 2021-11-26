set(IMAGE_HANDLERS_IXX
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagbmp.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imaggif.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagiff.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagjpeg.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagpcx.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagpng.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagpnm.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagtga.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagtiff.ixx
    ${CMAKE_CURRENT_SOURCE_DIR}/handlers/imagxpm.ixx

    ${CMAKE_CURRENT_SOURCE_DIR}/imagebase.ixx

    ${CMAKE_CURRENT_SOURCE_DIR}/image.ixx
)

set(IMAGE_HANDLERS_SRC
    ${CMAKE_SOURCE_DIR}/src/common/imagbmp.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imaggif.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagiff.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagjpeg.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagpcx.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagpng.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagpnm.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagtga.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagtiff.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagxpm.cpp
    ${CMAKE_SOURCE_DIR}/src/common/imagebase.cpp
)

add_library(wximage
    STATIC
        ${IMAGE_HANDLERS_IXX}
        ${IMAGE_HANDLERS_SRC}
)

target_include_directories(wximage
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/tmp/include
)

target_link_libraries(wximage
    PRIVATE
        wxconfigflags
        wxalgorithm
        wxwinwrap
        wxtype
        stdlibcore
        Microsoft.GSL::GSL
)

target_precompile_headers(wximage
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/wx/msw/winprecomp.h
)
