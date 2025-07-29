file(GLOB TEST_SOURCES test_*.cpp)

add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(unit_tests PRIVATE sources_lib)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(unit_tests)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Compiling in Debug mode")
    target_compile_definitions(unit_tests PRIVATE DEBUG)
    target_compile_options(unit_tests PRIVATE -g -O0)
endif()