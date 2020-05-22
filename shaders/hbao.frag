#version 330

smooth in vec2 pos;
smooth in vec2 pos_ray;

uniform mat4 projection;

uniform float aspect_ratio;
uniform float tan_half_fov;

uniform vec2 pixel_size; // TODO Send uniform

uniform sampler2D normalDepthTexture;

out vec4 frag_color;

const float PI = 3.14159265359;

const int DIRECTIONS = 6;
const int STEPS = 6;
const float RADIUS = 2.0;

void main (void) {
  vec4 p_g_buffer = texture(normalDepthTexture, pos);

  vec3 n_view = p_g_buffer.rgb;
  float p_depth = p_g_buffer.a;

  if (p_depth == 0.0) {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }

  vec3 p_view;
  p_view.z = -projection[3][2] / (2.0 * p_depth - 1.0 + projection[2][2]);
  p_view.x = pos_ray.x * -p_view.z;
  p_view.y = pos_ray.y * -p_view.z;

  float sum = 0.0;
  const float end = 2.0 * PI;
  const float step = end / float(DIRECTIONS);
  for (float d_a = 0.0; d_a < end; d_a += step) {
    vec2 d_screen_inc = vec2(cos(d_a), sin(d_a)) * RADIUS; // TODO Snap

    float h_a = 0.0;
    float r = RADIUS;

    vec3 d_view; // Used outside the loop for computing the tangent vector
    vec3 s_view; // Also used outside

    vec2 s_screen = pos;
    for (int i = 0; i < STEPS; ++i) {
      s_screen += d_screen_inc;

      float s_depth = texture(normalDepthTexture, s_screen.xy).a;

      s_view.z = -projection[3][2] / (2.0 * s_depth - 1.0 + projection[2][2]);
      s_view.x = (s_screen.x * 2.0 - 1.0) * aspect_ratio * tan_half_fov -s_view.z;
      s_view.y = (s_screen.y * 2.0 - 1.0) * tan_half_fov * -s_view.z;

      vec3 d_view = s_view - p_view; // Not normalized

      float angle = atan(d_view.z / length(d_view.xy));
      float d_view_len = length(d_view);
      if (angle > h_a && d_view_len <= RADIUS) {
        h_a = angle;
        r = d_view_len;
      }
    }

    d_view = normalize(d_view);

    vec3 t_view = normalize((s_view - dot(d_view, n_view) * n_view) - p_view);

    float t_a = atan(t_view.z, length(t_view.xy));

    if (t_a > h_a) {
      h_a = t_a;
      r = RADIUS;
    }

    sum += (sin(h_a) - sin(t_a)) * max(0.0, 1.0 - r / RADIUS);
  }

  float ao = 1.0 - (sum / float(DIRECTIONS));
  frag_color = vec4(ao, ao, ao, 1.0);
}
