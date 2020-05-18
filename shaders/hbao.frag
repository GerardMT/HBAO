#version 330

smooth in vec2 pos;

out vec4 frag_color;

uniform sampler2D normalDepthTexture;

void main (void) {
  frag_color = vec4(texture(normalDepthTexture, pos).rgb, 1.0);
}
