#include <filesystem>
// #include <experimental/filesystem> // uncomment here if the <filesystem> cannot be included above
#include <cstdlib>
#include <glad/glad.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
//
#include "../src/util_opengl.h"

int main() {
  if (!glfwInit()) { exit(EXIT_FAILURE); }
  // set OpenGL's version (note: ver. 2.1 is very old, but I chose because it's simple)
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  GLFWwindow *window = ::glfwCreateWindow(500, 500, "task05", nullptr, nullptr);
  if (!window) { // exit if failed to create window
    ::glfwTerminate();
    exit(EXIT_FAILURE);
  }
  ::glfwMakeContextCurrent(window); // working on this window below
  //
  if (!gladLoadGL()) {     // glad: load all OpenGL function pointers
    printf("Something went wrong in loading OpenGL functions!\n");
    exit(-1);
  }

  int shaderProgram;
  {
    const auto vrt_path = std::filesystem::path(SOURCE_DIR) / "shader.vert";
    const auto frg_path = std::filesystem::path(SOURCE_DIR) / "shader.frag";
    std::string vrt = acg::load_file_as_string(vrt_path.string().c_str()); // read source code of vertex shader program
    std::string frg = acg::load_file_as_string(frg_path.string().c_str()); // read source code of fragment shader program
    shaderProgram = acg::create_shader_program(vrt, frg); // compile the shader on GPU
  }

  glDisable(GL_MULTISAMPLE);
  const GLint iloc = glGetUniformLocation(shaderProgram, "time");  // location of variable in the shader program

  ::glClearColor(1, 1, 1, 1);  // set the color to fill the frame buffer when glClear is called.
  ::glEnable(GL_DEPTH_TEST);
  while (!::glfwWindowShouldClose(window)) {
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const auto time = static_cast<float>(glfwGetTime());
    glUniform1f(iloc,time);
    ::glMatrixMode(GL_PROJECTION);
    ::glLoadIdentity(); // identity transformation
    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadIdentity(); // identity transformation
    ::glUseProgram(shaderProgram);  // use the shader program from here
    ::glBegin(GL_QUADS); // draw a rectangle that cover the entire screen
    ::glVertex2d(-1,-1);
    ::glVertex2d(+1,-1);
    ::glVertex2d(+1,+1);
    ::glVertex2d(-1,+1);
    ::glEnd();
    ::glfwSwapBuffers(window);
    ::glfwPollEvents();
  }
  ::glfwDestroyWindow(window);
  ::glfwTerminate();
  exit(EXIT_SUCCESS);
}
