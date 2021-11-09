
add_library(stdlibcore
    ${CMAKE_CURRENT_SOURCE_DIR}/stdlibcore.ixx
)

target_compile_options(stdlibcore
    PUBLIC
        /wd4005
        /wd5106
)
