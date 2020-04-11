#version 330

layout (location = 0) in vec3 vert;

void main(void)  {
  gl_Position = vec4(vert, 0.0f);
}
