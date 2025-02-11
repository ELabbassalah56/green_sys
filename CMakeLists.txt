cmake_minimum_required(VERSION 3.10) # Updated for better support
project(monitor)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Find NCurses
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

# Find GoogleTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Enable testing
enable_testing()

# Include directories for source files
include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/**/*.cpp") # Fixing recursive file search

# Create the application executable (main project)
add_executable(monitor ${SOURCES})
target_link_libraries(monitor ${CURSES_LIBRARIES})

# Compiler options for the main application
target_compile_options(monitor PRIVATE -Wall -Wextra)

# Add Google Test executable for testing (using the test's main.cpp)
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

if(TEST_SOURCES)
    # Remove src/main.cpp from SOURCES if it exists
    list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

    # Add the test executable, including the test-specific main.cpp
    add_executable(monitor_tests ${TEST_SOURCES} ${SOURCES} test/main.cpp)

    # Link the necessary libraries for the test executable
    target_link_libraries(monitor_tests ${GTEST_LIBRARIES} gtest gtest_main pthread ncurses -lncurses)

    # Register the tests to be run using CTest
    add_test(NAME MonitorTests COMMAND monitor_tests)
endif()