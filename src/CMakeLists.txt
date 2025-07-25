file(GLOB SOURCES *.cpp)

add_library(sources_lib STATIC ${SOURCES})

target_include_directories(sources_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/include)