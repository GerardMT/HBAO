#version 330

smooth in vec2 pos;
smooth in vec2 screen_ray;

uniform mat4 projection;

uniform float aspect_ratio;
uniform float tan_half_fov;

uniform vec2 pixel_size;

uniform sampler2D normalDepthTexture;

uniform sampler2D noise_texture;

out vec4 frag_color;

const float PI = 3.14159265359;

const mat2 UNIFORM_DIRECTIONS[4] = mat2[](
  mat2(cos(0),              sin(0),              -sin(0),              cos(0)),
  mat2(cos(PI * 0.5),       sin(PI * 0.5),       -sin(PI * 0.5),       cos(PI * 0.5)),
  mat2(cos(PI),             sin(PI),             -sin(PI),             cos(PI)),
  mat2(cos(PI * 3.0 / 2.0), sin(PI * 3.0 / 2.0), -sin(PI * 3.0 / 2.0), cos(PI * 3.0 / 2.0))
);

const int DIRECTIONS = 4;
const int STEPS = 6;
const float RADIUS = 0.2;
const float T_BIAS = 30.0 * (PI / 180.0);
const float STRENGTH = 1.0;

vec3 unproject (vec2 screen_ray, float p_depth) {
  vec3 p_view;
  p_view.z = -projection[3][2] / (2.0 * p_depth - 1.0 + projection[2][2]);
  p_view.x = screen_ray.x * -p_view.z;
  p_view.y = screen_ray.y * -p_view.z;

  return p_view;
}

float random(vec2 st) {
  return texture(noise_texture, st / (pixel_size * textureSize(noise_texture, 0))).r;
}

void main (void) {
  vec4 p_g_buffer = texture(normalDepthTexture, pos);

  vec3 n_view = p_g_buffer.rgb;
  float p_depth = p_g_buffer.a;

  if (p_depth == 0.0) {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }

  vec3 p_view = unproject(screen_ray, p_depth);

  float sum = 0.0;

  float start = random(pos) * (PI * 0.5); // Random starting angle.
  float end = start + (PI * 0.5);
  const float step = (PI * 0.5) / float(DIRECTIONS);
  for (float d_a = start; d_a < end; d_a += step) { // Iterate over a single quadrant.
    for (int i = 0; i < 4; ++i) { // Uniform distribution of directions (4 quadrants).
      vec3 q_view = p_view + vec3(UNIFORM_DIRECTIONS[i] * vec2(cos(d_a) * RADIUS, sin(d_a) * RADIUS), 0.0); // Sphere end point.

      vec4 q_clip = projection * vec4(q_view, 1.0); // Project shpere end point from veiw to texture sapce.
      vec2 q_texture = (q_clip.xy / q_clip.w) * 0.5 + 0.5;

      vec2 r_texture_inc = (q_texture- pos) / STEPS;

      vec3 t_view = normalize((q_view - dot(q_view - p_view, n_view) * n_view) - p_view); // Project point q_view to plane (p_view, n_view).

      float t_a = atan(t_view.z, length(t_view.xy)) + T_BIAS;

      float h_a_pre = t_a;
      float ao_pre = 0.0;
      float wao = 0.0;

      vec2 s_texture = pos;
      for (int j = 0; j < STEPS; ++j) { // Marching on the heighfield.
        s_texture += r_texture_inc * random(pos + j);
        vec2 s_texture_snap = (round(s_texture / pixel_size) + 0.5) * pixel_size; // Snap to pixels centers.

        float s_depth = texture(normalDepthTexture, s_texture_snap.xy).a;
        if (s_depth == 0.0) {
          continue;
        }

        vec2 s_screen_ray = (s_texture_snap * 2.0 - 1.0) * tan_half_fov;
        s_screen_ray.x *= aspect_ratio;
        vec3 s_view = unproject(s_screen_ray, s_depth);

        vec3 d_view = s_view - p_view;// BUG: Normalized does not work.

        float h_a = atan(d_view.z / length(d_view.xy));

        float d_view_len = length(d_view);
        if (h_a > h_a_pre && d_view_len <= RADIUS) {
          float r_norm = d_view_len / RADIUS;

          float ao = sin(h_a) - sin(t_a); // Per sample attenuation.
          wao += (ao - ao_pre) * (1.0 - r_norm * r_norm);

          h_a_pre = h_a;
          ao_pre = ao;
        }
      }

      sum += wao;
    }
  }

  float ao = 1.0 - (sum * STRENGTH / float(4 * DIRECTIONS));
  frag_color = vec4(ao, ao, ao, 1.0);
}
