add_library(wxalgorithm
    STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/charutils.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/stringutils.ixx
)

target_link_libraries(wxalgorithm
    PRIVATE
        stdlibcore
        fmt::fmt
)
