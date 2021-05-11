#version 330

layout (location = 0) in vec3 vert;

smooth out vec2 pos;

void main(void) {
  pos = (vert.xy) * 0.5 + 0.5;
  gl_Position = vec4(vert, 1.0);
}