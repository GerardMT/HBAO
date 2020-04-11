#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal_matrix;

smooth out vec3 eye_normal;

void main(void)  {
  vec4 world_vertex = model * vec4(vert, 1.0);
  vec4 view_vertex = view * world_vertex;

  eye_normal = normalize(normal_matrix * normal);

  gl_Position = projection * view_vertex;
}
