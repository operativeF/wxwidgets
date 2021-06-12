include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG 4d8716f1efc1d14aa736ef52ee727bd4204f4c40
)

FetchContent_GetProperties(doctest)
if(NOT doctest_POPULATED)
  FetchContent_Populate(doctest)
  add_subdirectory(${doctest_SOURCE_DIR} ${doctest_BINARY_DIR})
endif()

add_executable(nongui_test 
    test_main.cpp
    any/anytest.cpp

    testprec.h
    testableframe.h
    testdate.h
    testfile.h
    archive/archivetest.h
    streams/bstream.h
)

target_compile_features(nongui_test PRIVATE cxx_std_17)

target_include_directories(nongui_test 
    PUBLIC
        ${DOCTEST_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/include/msvc
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(nongui_test
    PRIVATE
        doctest::doctest
)