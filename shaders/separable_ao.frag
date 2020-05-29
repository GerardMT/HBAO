#version 330

smooth in vec2 pos;

out vec4 frag_color;

uniform sampler2D normalDepthTexture;

void main (void) {
  if (texture(normalDepthTexture, pos).b < 0) {
     frag_color = vec4(1.0, 1.0, 1.0, 1.0);
  } else {
    frag_color = vec4(0.5 + texture(normalDepthTexture, pos).rgb, 1.0);
  }
}
