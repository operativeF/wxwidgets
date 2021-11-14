add_library(wxalgorithm
    STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/charutils.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/stringutils.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/strings/nonmodifying.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/strings/modifying.ixx
        ${CMAKE_CURRENT_SOURCE_DIR}/strings/unsafe.ixx
)

target_link_libraries(wxalgorithm
    PRIVATE
        stdlibcore
        fmt::fmt
)
