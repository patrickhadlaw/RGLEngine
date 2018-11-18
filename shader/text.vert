#version 430

in vec3 vertex_position;
in vec4 vertex_color;
in vec2 texture_coords;

uniform mat4 model;

out vec4 color;
out vec2 uv_coords;

void main() {
	color = vertex_color;
	uv_coords = texture_coords;
	gl_Position = model*vec4((vertex_position.x - 1), (vertex_position.y + 1), vertex_position.z, 1.0);
}