cmake_minimum_required(VERSION 3.14)

project(yaschi
        VERSION 1.0
        DESCRIPTION "Yet another Scheme interpreter"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
add_subdirectory(src)

add_executable(scheme-interpreter main.cpp)

target_link_libraries(scheme-interpreter PRIVATE sources_lib)
target_compile_options(scheme-interpreter PRIVATE -Wall -Wextra -Wpedantic)

# Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Compiling in Debug mode")
    target_compile_definitions(MyTarget PRIVATE DEBUG)
    target_compile_options(MyTarget PRIVATE -g -O0)
endif()

option(ENABLE_TESTING "Enable tests" ON)
if(ENABLE_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_subdirectory(tests)
endif()