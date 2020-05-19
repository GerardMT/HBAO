#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal_matrix;

flat out vec3 normal_view;
smooth out float depth_view;

void main(void) {
  normal_view = normalize(normal_matrix * normal);

  gl_Position = projection * view * model * vec4(vert, 1.0);
  depth_view = (gl_Position.z / gl_Position.w + 1.0) / 2.0;
}
