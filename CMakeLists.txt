cmake_minimum_required(VERSION 3.22)
project(raylib_projects C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

# Define function to include dependencies via FetchContent
function(include_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${libName})
endfunction()

# Add Raylib dependency
set(LIB1 raylib)
find_package(${LIB1} QUIET)
if (NOT ${LIB1}_FOUND)
    message(STATUS "Getting ${LIB1} from Github")
    include_dependency(${LIB1} https://github.com/raysan5/raylib.git 4.5.0)
else()
    message(STATUS "Using local ${LIB1}")
endif()

# Add executable
add_executable(hello_raylib_with_cmake main.c)

# Include Raylib headers
target_include_directories(hello_raylib_with_cmake PRIVATE ${raylib_SOURCE_DIR}/src)

# Link Raylib library
target_link_libraries(hello_raylib_with_cmake PRIVATE raylib)
