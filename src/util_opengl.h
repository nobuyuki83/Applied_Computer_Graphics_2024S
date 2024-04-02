//
// Created by Nobuyuki Umetani on 2024/04/02.
//

#ifndef UTIL_OPENGL_H_
#define UTIL_OPENGL_H_

#include <iostream>
#include <fstream>
#include <vector>

namespace acg {

std::string load_file_as_string(
    const std::string &fname) {
  std::ifstream inputFile1(fname.c_str());
  std::istreambuf_iterator<char> vdataBegin(inputFile1);
  std::istreambuf_iterator<char> vdataEnd;
  return {vdataBegin, vdataEnd};
}

int compile_shader(
    const std::string &str_glsl_vert,
    int shaderType) {
  int id_shader = glCreateShader(shaderType);
  const char *vfile = str_glsl_vert.c_str();
  glShaderSource(id_shader, 1, &vfile, nullptr);
  glCompileShader(id_shader); // compile the code

  { // print if failed
    GLint res;
    glGetShaderiv(id_shader, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE) {
      if (shaderType == GL_VERTEX_SHADER) {
        std::cout << "compile vertex shader failed" << std::endl;
      } else if (shaderType == GL_FRAGMENT_SHADER) {
        std::cout << "compile fragment shader failed" << std::endl;
      }
    }
  }
  { // show log
    const GLsizei maxLength = 1024;
    GLsizei length;
    auto *infoLog = new GLchar[maxLength];
    glGetShaderInfoLog(id_shader, maxLength, &length, infoLog);
    std::cout << "log: " << infoLog << std::endl;
    delete[] infoLog;
  }
  return id_shader;
}

int create_shader_program(
    const std::string &str_glsl_vert,
    const std::string &str_glsl_frag) {
  int vShaderId = compile_shader(str_glsl_vert, GL_VERTEX_SHADER);
  int fShaderId = compile_shader(str_glsl_frag, GL_FRAGMENT_SHADER);

  int id_program = glCreateProgram();
  glAttachShader(id_program, vShaderId);
  glAttachShader(id_program, fShaderId);

  GLint linked;
  glLinkProgram(id_program);
  glGetProgramiv(id_program, GL_LINK_STATUS, &linked);
  if (linked == GL_FALSE) {
    std::cerr << "Link Err.\n";
    GLint maxLength = 0;
    glGetProgramiv(id_program, GL_INFO_LOG_LENGTH, &maxLength);
    // The maxLength includes the NULL character
    std::vector<GLchar> infoLog(maxLength);
    glGetProgramInfoLog(id_program, maxLength, &maxLength, &infoLog[0]);
    for (char i: infoLog) {
      std::cout << i;
    }
    std::cout << std::endl;
    glDeleteProgram(id_program); // The program is useless now. So delete it.
    return 0;
  }
  return id_program;
}


} // acg

#endif //UTIL_OPENGL_H_
