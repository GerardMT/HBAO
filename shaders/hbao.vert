#version 330

layout (location = 0) in vec3 vert;

uniform float aspect_ratio;
uniform float tan_half_fov;

smooth out vec2 pos;
smooth out vec2 pos_ray;

void main(void) {
  pos_ray.x = vert.x * aspect_ratio * tan_half_fov; // vert goes from -1 to 1
  pos_ray.y = vert.y * tan_half_fov;

  pos = (vert.xy) * 0.5 + 0.5;
  gl_Position = vec4(vert, 1.0);
}
