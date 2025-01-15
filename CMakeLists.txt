# CMakeLists.txt project file for Raylib projects
# 
cmake_minimum_required(VERSION 3.30)
project(raylib_projects C)

set(CMAKE_C_STANDARD 99)

# Include the command that downloads libraries
include(FetchContent)

# define a function for adding git dependencies
function(include_dependency libName gitURL gitTag)
    # setup the declare
    FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )

    FetchContent_MakeAvailable(${libName})
endfunction()

# add raylib support
set(LIB1 raylib)
find_package(${LIB1} QUIET)
if (NOT ${LIB1}_FOUND)
    message(STATUS "Getting ${LIB1} from Github")
    include_dependency(${LIB1} https://github.com/raysan5/raylib.git 5.5)
else()
    message(STATUS "Using local ${LIB1}")
endif()

add_executable(hello_raylib_with_cmake main.c)

# set the include directory
target_include_directories(hello_raylib_with_cmake PRIVATE ${raylib_INCLUDE_DIRS})

# link all libraries to the project
target_link_libraries(hello_raylib_with_cmake PRIVATE ${LIB1})