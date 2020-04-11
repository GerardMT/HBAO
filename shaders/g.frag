#version 330

flat in vec3 normal_world;
smooth in float depth_world;

out vec4 frag_color;

void main (void) {
  frag_color = vec4(normal_world, depth_world);
}
