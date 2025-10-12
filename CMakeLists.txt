cmake_minimum_required(VERSION 3.10)
project(FileMagik C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
find_package(Curses REQUIRED)

# Include directories for GLib
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})


set(SOURCES
    main.c
    src/FileList.c
    src/UI.c
)
add_executable(main ${SOURCES})

# Link libraries
target_link_libraries(main ${GLIB2_LIBRARIES} ${CURSES_LIBRARIES})

# Add compile flags
target_compile_options(main PRIVATE ${GLIB2_CFLAGS_OTHER})
