#version 330

smooth in vec3 pos_view;
smooth in float depth_view;

out vec4 frag_color;

void main (void) {
  vec3 normal_view = normalize(cross(dFdx(pos_view), dFdy(pos_view)));
  frag_color = vec4(normal_view, depth_view);
}
