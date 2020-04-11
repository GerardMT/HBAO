#version 330

smooth in vec3 eye_normal;

out vec4 frag_color;

void main (void) {
  frag_color = vec4(vec3(eye_normal.z), 1.0);
}
