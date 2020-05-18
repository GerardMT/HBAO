#version 330

smooth in vec2 pos;

out vec4 frag_color;

uniform sampler2D normalDepthTexture;

void main (void) {
  float d = texture(normalDepthTexture, pos).a + 1.0;
  frag_color = vec4(d, d, d, 1.0);
}
