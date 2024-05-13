#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
//
#include "../src/util_opengl.h"
#include "../src/util_triangle_mesh.h"

/***
 * draw triangle mesh
 * @param tri2vtx triangle index
 * @param vtx2xyz vertex coordinates
 * @param vtx2normal vertex normals
 */
void draw(
    const Eigen::Matrix<unsigned int, 3, Eigen::Dynamic> &tri2vtx,
    const Eigen::Matrix3Xf &vtx2xyz,
    const Eigen::Matrix3Xf &vtx2normal) {
  ::glBegin(GL_TRIANGLES);
  for (auto i_tri = 0; i_tri < tri2vtx.cols(); ++i_tri) {
    const auto i0 = tri2vtx(0, i_tri);
    const auto i1 = tri2vtx(1, i_tri);
    const auto i2 = tri2vtx(2, i_tri);
    ::glNormal3fv(vtx2normal.data() + i0 * 3);
    ::glVertex3fv(vtx2xyz.data() + i0 * 3);
    ::glNormal3fv(vtx2normal.data() + i1 * 3);
    ::glVertex3fv(vtx2xyz.data() + i1 * 3);
    ::glNormal3fv(vtx2normal.data() + i2 * 3);
    ::glVertex3fv(vtx2xyz.data() + i2 * 3);
  }
  ::glEnd();
}

int main() {
  const auto file_path = std::filesystem::path(PROJECT_SOURCE_DIR) / ".." / "asset" / "armadillo.obj";
  auto[tri2vtx, vtx2xyz] = acg::read_wavefrontobj_as_3d_triangle_mesh(file_path);
  // bounding box
  auto aabb_max = vtx2xyz.rowwise().maxCoeff();
  auto aabb_min = vtx2xyz.rowwise().minCoeff();
  auto aabb_center = (aabb_min + aabb_max) * 0.5f; // center of the bounding box
  auto aabb_size = (aabb_max - aabb_min).maxCoeff(); // size of the bounding box
  // normalize coordinate
  vtx2xyz = (vtx2xyz.colwise() - aabb_center) / aabb_size * 1.3;
  vtx2xyz = Eigen::AngleAxisf(M_PI, Eigen::Vector3f::UnitY()).matrix() * vtx2xyz;
  vtx2xyz = vtx2xyz.colwise() + Eigen::Vector3f(0.2, 0.0, 0.0);
  // compute normals at vertices
  const auto vtx2normal = acg::vertex_normals_of_triangle_mesh(tri2vtx, vtx2xyz);

  if (!glfwInit()) { exit(EXIT_FAILURE); }
  // set OpenGL's version (note: ver. 2.1 is very old, but I chose because it's simple)
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  // open window
  GLFWwindow *window = ::glfwCreateWindow(500, 500, "task04", nullptr, nullptr);
  if (!window) { // exit if failed to create window
    ::glfwTerminate();
    exit(EXIT_FAILURE);
  }
  ::glfwMakeContextCurrent(window); // working on this window
  if (!gladLoadGL()) {     // glad: load all OpenGL function pointers
    std::cout << "Something went wrong in loading OpenGL functions!\n" << std::endl;
    exit(-1);
  }

  int shaderProgram;
  { // compile shader program
    std::string vrt_path = std::filesystem::path(PROJECT_SOURCE_DIR) / "shader.vert";
    std::string frg_path = std::filesystem::path(PROJECT_SOURCE_DIR) / "shader.frag";
    std::string vrt = acg::load_file_as_string(vrt_path); // read source code of vertex shader program
    std::string frg = acg::load_file_as_string(frg_path); // read source code of fragment shader program
    shaderProgram = acg::create_shader_program(vrt, frg); // compile the shader on GPU
  }
  ::glUseProgram(shaderProgram); // use the shader program
  GLint iloc = glGetUniformLocation(shaderProgram, "is_reflection"); // shader program variable

  ::glClearColor(1, 1, 1, 1);  // set the color to fill the frame buffer when glClear is called.
  ::glEnable(GL_DEPTH_TEST);
  while (!::glfwWindowShouldClose(window)) {
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ::glUniform1i(iloc, true); // set value to the shader program (draw reflection of the triangle mesh)
    draw(tri2vtx, vtx2xyz, vtx2normal);
    ::glUniform1i(iloc, false); // set value to the shader program (draw triangle mesh)
    draw(tri2vtx, vtx2xyz, vtx2normal);

    ::glfwSwapBuffers(window);
    ::glfwPollEvents();
  }
  ::glfwDestroyWindow(window);
  ::glfwTerminate();
  exit(EXIT_SUCCESS);
}
