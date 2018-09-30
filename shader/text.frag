#version 430

in vec4 color;
in vec2 uv_coords;

uniform sampler2D texture_0;
uniform bool enable_texture;

out vec4 frag_color;

void main() {
	frag_color = vec4(color.rgb, texture(texture_0, uv_coords).r)*float(enable_texture) + vec4(color.rgb, 1.0)*float(!enable_texture);
}