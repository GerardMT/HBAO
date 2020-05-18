#version 330

smooth in vec2 pos;

out vec4 frag_color;

uniform bool h;
uniform sampler2D normalDepthTexture;

uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main (void) {
  vec2 offset = 1.0 / textureSize(normalDepthTexture, 0);

  vec3 res = texture(normalDepthTexture, pos).rgb * weight[0];

  for(int i = 1; i < 5; ++i) {
    if (h) {
      res += texture(normalDepthTexture, pos + vec2(offset.x * i, 0.0)).rgb * weight[i];
      res += texture(normalDepthTexture, pos - vec2(offset.x * i, 0.0)).rgb * weight[i];
    } else {
      res += texture(normalDepthTexture, pos + vec2(0.0, offset.y * i)).rgb * weight[i];
      res += texture(normalDepthTexture, pos - vec2(0.0, offset.y * i)).rgb * weight[i];
    }
  }

  frag_color = vec4(res, 1.0);
}
