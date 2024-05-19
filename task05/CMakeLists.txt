cmake_minimum_required(VERSION 3.12)

#############################
# set C++ detail
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#############################

# set project name
project(task05)

# define a macro to get absolute path in C++
add_definitions(-DSOURCE_DIR="${PROJECT_SOURCE_DIR}")

#############################
# specifying libraries to use

# use opengl
find_package(OpenGL REQUIRED)

# use glfw
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../external/glfwlib)
find_package(glfw3 REQUIRED)

########################
# include, build, & link

include_directories(${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/../external/delfem2/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad3/include
)

add_executable(${PROJECT_NAME}
#  main_ans.cpp
  main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad3/include/glad/glad.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad3/src/glad.c
)

# https://stackoverflow.com/questions/33678965/need-to-link-cmake-project-to-dl-library
target_link_libraries(${PROJECT_NAME}
    # stdc++fs # uncomment here if <filesystem> cannot be included
  OpenGL::GL
  glfw
  ${CMAKE_DL_LIBS})

#############################
# showing status of libraries

GET_TARGET_PROPERTY(GLFW_INCLUDE_DIR
  glfw INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "task02 glfw include directory: ${GLFW_INCLUDE_DIR}")