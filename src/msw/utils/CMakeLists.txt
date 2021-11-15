add_library(mswutils
    STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/wrap/utils.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/wrapwin.ixx
)

target_include_directories(mswutils
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(mswutils
    PRIVATE
        Onecore
        opengl32
)