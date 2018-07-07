#version 430

in vec3 vertex_position;
in vec4 vertex_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;

void main(){
	color = vertex_color;
	gl_Position = projection*view*model*vec4(vertex_position, 1.0);
}
