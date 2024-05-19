#version 120

// see the GLSL 1.2 specification:
// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf

void main()
{
  // draw the primitive without transformation
  gl_Position = gl_Vertex; // following code do nothing (input == output)
}
