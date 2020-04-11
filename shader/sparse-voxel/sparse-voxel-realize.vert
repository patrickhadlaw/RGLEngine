#version 450

in vec3 vertex_position;
in vec2 texture_coords;

uniform mat4 model;

out vec2 uv_coords;

void main() {
	uv_coords = texture_coords;
	gl_Position = model*vec4(vertex_position, 1.0);
}
