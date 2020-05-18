#version 330

flat in vec3 normal_view;
smooth in float depth_view;

out vec4 frag_color;

void main (void) {
  frag_color = vec4(normal_view, depth_view);
}
