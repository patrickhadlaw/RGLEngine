#version 430

in vec4 color;
in vec3 normal;
in vec2 uv_coords;

uniform sampler2D texture_0;
uniform bool enable_texture;

out vec4 frag_color;

void main(){
	frag_color = texture(texture_0, uv_coords)*float(enable_texture) + color*float(!enable_texture);
}