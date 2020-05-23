#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

smooth out vec3 pos_view;
smooth out float depth_view;

void main(void) {
  gl_Position = view * model * vec4(vert, 1.0);
  pos_view = gl_Position.xyz;
  gl_Position = projection * gl_Position;

  depth_view = (gl_Position.z / gl_Position.w) * 0.5 + 0.5;
}
