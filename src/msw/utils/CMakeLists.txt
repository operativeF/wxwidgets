add_library(mswutils STATIC ${CMAKE_CURRENT_SOURCE_DIR}/wrap/utils.ixx)

target_link_libraries(mswutils
    PRIVATE
        opengl32
)