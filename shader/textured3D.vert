#version 430

in vec3 vertex_position;
in vec3 vertex_normal;
in vec4 vertex_color;
in vec2 texture_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec4 color;
out vec2 uv_coords;

void main(){
	normal = vertex_normal;
	color = vertex_color;
	uv_coords = texture_coords;
	gl_Position = projection*view*model*vec4(vertex_position, 1.0);
}