#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal_matrix;

flat out vec3 normal_world;
smooth out float depth_world;

void main(void) {
  gl_Position = projection * view * model * vec4(vert, 1.0);

  normal_world = normalize((model * vec4(normal, 1.0f)).xyz);
  depth_world = (gl_Position.z + 1.0f) / 2.0f;
}
