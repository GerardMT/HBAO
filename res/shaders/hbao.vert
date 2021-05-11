#version 330

layout (location = 0) in vec3 vert;

uniform float aspect_ratio;
uniform float tan_half_fov;

smooth out vec2 pos;
smooth out vec2 screen_ray;

void main(void) {
  screen_ray.x = vert.x * tan_half_fov * aspect_ratio; // vert goes from -1 to 1
  screen_ray.y = vert.y * tan_half_fov;

  pos = (vert.xy) * 0.5 + 0.5;
  gl_Position = vec4(vert, 1.0);
}
