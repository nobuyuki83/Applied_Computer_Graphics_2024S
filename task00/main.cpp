#include <cstdio>
#include <cstdlib>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

// print out error
static void error_callback(
  [[maybe_unused]] int error, 
  const char* description)
{
  fputs(description, stderr);
}

int main()
{
  ::glfwSetErrorCallback(error_callback);
  if ( !glfwInit() ) { exit(EXIT_FAILURE); }
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  // make window
  GLFWwindow* window = ::glfwCreateWindow(
      640, 480, "task0: Build C++ Program with CMake",
      nullptr, nullptr);
  if (!window) { // exit if failed to create window
    ::glfwTerminate();
    exit(EXIT_FAILURE);
  }
  ::glfwMakeContextCurrent(window); // working on this window
  while ( !::glfwWindowShouldClose(window) )
  {
    // get windowsize
    int width, height;
    ::glfwGetFramebufferSize(window, &width, &height);
    // get aspect ratio of the window
    const float ratio = (float)width / (float) height;
    ::glViewport(0, 0, width, height);
    ::glClear(GL_COLOR_BUFFER_BIT);
    // matrix mode: projection
    ::glMatrixMode(GL_PROJECTION);
    ::glLoadIdentity();
    ::glOrtho(-ratio, ratio, -1.f, 1.f, -1.f, +1.f);
    // matrix mode : model view
    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadIdentity();

    // draw a triangle here
    ::glBegin(GL_TRIANGLES);
    ::glColor3f(1.f, 0.f, 0.f); // color of the following vertex
    ::glVertex3f(-0.6f, -0.4f, 0.f);
    ::glColor3f(0.f, 1.f, 0.f); // color of the following vertex
    ::glVertex3f(0.6f, -0.4f, 0.f);
    ::glColor3f(0.f, 0.f, 1.f); // color of the following vertex
    ::glVertex3f(0.f, 0.6f, 0.f);
    ::glEnd();

    // finalize drawing by swapping buffer
    ::glfwSwapBuffers(window);
    ::glfwPollEvents();
  }
  ::glfwDestroyWindow(window);
  ::glfwTerminate();
  exit(EXIT_SUCCESS);
}
