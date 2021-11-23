add_library(wxwinwrap
    STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/windef.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/unique_hnd.ixx
)

target_link_libraries(wxwinwrap
    PRIVATE
        opengl32
)
