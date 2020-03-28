#version 440

in vec3 vertex_position;
in vec2 texture_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 uv_coords;

void main() {
	uv_coords = texture_coords;
	gl_Position = projection*view*model*vec4(vertex_position, 1.0);
}
