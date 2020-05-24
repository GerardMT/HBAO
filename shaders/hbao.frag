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
  mat2(cos(0),        sin(0),        -sin(0),         cos(0)),
  mat2(cos(PI * 0.5), sin(PI * 0.5), -sin(PI * 0.5),  cos(PI * 0.5)),
  mat2(cos(PI),       sin(PI),       -sin(PI),        cos(PI)),
  mat2(cos(PI * 2.0), sin(PI * 2.0), -sin(PI * 2.0),  cos(PI * 2.0))
);

const int DIRECTIONS = 2;
const int STEPS = 6;
const float RADIUS = 0.3;
const float T_BIAS = 45.0 * (PI / 180.0);
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

  float start = random(pos) * (2.0 * PI); // Random starting angle. // BUG: Should be 0.5 * PI
  float end = start + (PI * 0.5);
  const float step = (PI * 0.5) / float(DIRECTIONS);
  for (float d_a = start; d_a < end; d_a += step) { // Iterate over a single quadrant.
    vec3 r_view_quad = p_view + vec3(cos(d_a), sin(d_a), 0.0) * RADIUS;

    vec4 r_clip_quad = projection * vec4(r_view_quad, 1.0); // Project shpere from veiw to texture sapce.
    vec2 r_texture_quad = (r_clip_quad.xy / r_clip_quad.w) * 0.5 + 0.5; // Skip dividing by w because is 0.

    vec2 r_texture_inc_quad = (r_texture_quad - pos) / STEPS;

    vec3 u_view_quad = normalize(r_view_quad - p_view);
    vec3 t_view_quad = normalize((r_view_quad - dot(u_view_quad, n_view) * n_view) - p_view); // Project point q_view to plane (p_view, n_view).

    float t_a = atan(t_view_quad.z, length(t_view_quad.xy)) + T_BIAS;

    for (int i = 0; i < 4; ++i) { // Uniform distribution of directions (4 quadrants).
      vec2 r_texture_inc = UNIFORM_DIRECTIONS[i] * r_texture_inc_quad;

      float h_a_pre = t_a;
      float ao_pre = 0.0;
      float wao = 0.0;

      vec2 s_texture = pos;
      for (int j = 0; j < STEPS; ++j) { // Marching on the heighfield.
        s_texture += r_texture_inc * random(pos + j);
        s_texture = round(s_texture / pixel_size) * pixel_size + (pixel_size / 2.0); // Snap to pixels centers.

        float s_depth = texture(normalDepthTexture, s_texture.xy).a;
        if (s_depth == 0.0) {
          continue;
        }

        vec2 s_screen_ray = (s_texture * 2.0 - 1.0) * tan_half_fov;
        s_screen_ray.x *= aspect_ratio;
        vec3 s_view = unproject(s_screen_ray, s_depth);

        vec3 d_view = s_view - p_view;

        float h_a = atan(d_view.z / length(d_view.xy));
        float d_view_len = length(d_view);
        if (h_a > h_a_pre  && d_view_len <= RADIUS) {
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
