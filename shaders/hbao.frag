#version 330

smooth in vec2 pos;
smooth in vec2 pos_ray;

uniform mat4 projection;

uniform float aspect_ratio;
uniform float tan_half_fov;

uniform vec2 pixel_size;

uniform sampler2D normalDepthTexture;

out vec4 frag_color;

const float PI = 3.14159265359;

const int DIRECTIONS = 6;
const int STEPS = 6;
const float RADIUS = 0.5;
const float T_BIAS = 30.0 * (PI / 180.0);

// TODO Uniform distribution of directions per pixel
// TODO Face normals ddx/ddy. Looks like it will solve the problem of diferent faces having diferent ao values.
// TODO Per Sample Attenuation

// https://thebookofshaders.com/10/
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}

void main (void) {
  float rand = random(pos);

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

  float start = rand * (2 * PI);
  float end = start + (2.0 * PI);
  const float step = (2 * PI) / float(DIRECTIONS);
  for (float d_a = start; d_a < end; d_a += step) {
    vec3 r_view = vec3(cos(d_a), sin(d_a), 0.0) * RADIUS;

    vec4 r_clip = projection * vec4(r_view, 0.0); // Project shpere from veiw to screen.
    vec2 r_screen = r_clip.xy * 0.5 + 0.5; // Skip dividing by w because is 0.

    vec2 r_screen_inc = r_screen / STEPS;

    vec3 q_view = p_view + n_view + r_view;
    vec3 u_view = normalize(q_view - p_view);
    vec3 t_view = normalize((q_view - dot(u_view, n_view) * n_view) - p_view); // Project point q_view to plane (p_view, n_view).
    float t_a = atan(t_view.z, length(t_view.xy)) + T_BIAS;

    float h_a = t_a;
    float r = RADIUS;

    vec2 s_screen = pos;
    for (int i = 0; i < STEPS; ++i) {
      s_screen += r_screen_inc + rand * r_screen_inc;
      s_screen = round(s_screen / pixel_size) * pixel_size + (pixel_size / 2.0); // Snap to pixels centers.

      float s_depth = texture(normalDepthTexture, s_screen.xy).a;

      vec3 s_view;
      s_view.z = -projection[3][2] / (2.0 * s_depth - 1.0 + projection[2][2]);
      s_view.x = (s_screen.x * 2.0 - 1.0) * aspect_ratio * tan_half_fov * -s_view.z;
      s_view.y = (s_screen.y * 2.0 - 1.0) * tan_half_fov * -s_view.z;

      vec3 d_view = s_view - p_view;

      float angle = atan(d_view.z / length(d_view.xy));
      float d_view_len = length(d_view);
      if (angle > h_a && d_view_len <= RADIUS) {
        h_a = angle;
        r = d_view_len;
      }
    }

    float r_norm = r / RADIUS;
    sum += (sin(h_a) - sin(t_a)) * (1.0 - r_norm * r_norm);
  }

  float ao = 1.0 - (sum / float(DIRECTIONS));
  frag_color = vec4(ao, ao, ao, 1.0);
}
