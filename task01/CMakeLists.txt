cmake_minimum_required(VERSION 3.10)  # specify the version of cmake

#############################
# set C++ detail
enable_language(CXX)  # we are using C++
set(CMAKE_CXX_STANDARD 17)  # we are using C++ 17
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)  # we are using STL library

#############################
# set project name

project(task01)

#############################
# define macro
add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

#############################
# specifying libraries to use

########################
# include, build, and link

include_directories(
    ${PROJECT_SOURCE_DIR}/../external
)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
)