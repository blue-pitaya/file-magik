cmake_minimum_required(VERSION 3.10)
project(FileMagik LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(FileMagik
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(FileMagik PRIVATE src)
target_link_libraries(FileMagik ${CURSES_LIBRARIES})
