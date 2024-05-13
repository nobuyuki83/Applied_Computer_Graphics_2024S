cmake_minimum_required(VERSION 3.10)

#############################
# set C++ detail
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#############################

# set project name
project(task04)

# define a macro to get absolute path in C++
add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

#############################
# specifying libraries to use

# use opengl
find_package(OpenGL REQUIRED)

# use glfw
set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/../external/glfwlib)
find_package(glfw3 REQUIRED)

########################
# include, build, & link

include_directories(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/../external
  ${PROJECT_SOURCE_DIR}/../external/glad3/include
  ${PROJECT_SOURCE_DIR}/../external/eigen
)

add_executable(${PROJECT_NAME}
  main.cpp
  ${PROJECT_SOURCE_DIR}/../external/glad3/src/glad.c
)

# https://stackoverflow.com/questions/33678965/need-to-link-cmake-project-to-dl-library
target_link_libraries(${PROJECT_NAME}
  OpenGL::GL
  glfw
  ${CMAKE_DL_LIBS}
)

#############################
# showing status of libraries

GET_TARGET_PROPERTY(GLFW_INCLUDE_DIR
  glfw INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "task04 glfw include directory: ${GLFW_INCLUDE_DIR}")