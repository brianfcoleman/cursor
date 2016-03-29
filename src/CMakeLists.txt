cmake_minimum_required(VERSION 3.3)

project(cursor
    VERSION 0.1.0
    LANGUAGES CXX
)

include(FindGTest)
enable_testing()
find_package(GTest REQUIRED)

set(gap_buffer_headers
    "gap-buffer.hh"
    "range.hh"
)

set(gap_buffer_test_sources
    "test/gap-buffer-test.cc"
)

add_executable(gap_buffer_test
    "${gap_buffer_headers}"
    "${gap_buffer_test_sources}"
)

target_include_directories(gap_buffer_test
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(gap_buffer_test
    PRIVATE
    "${GTEST_BOTH_LIBRARIES}"
)

set_target_properties(gap_buffer_test
    PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_NAME gap-buffer-test
)

