#version 440

struct OctreeNode {
	vec4 color;
	vec3 position;
	uint depth;
	int next;
};

layout(std430, binding=1) readonly buffer octree_buffer {
	OctreeNode nodes[];
};

in vec3 normal;
in vec2 uv_coords;

layout(r32i) uniform isampler2D texture_0;

out vec4 frag_color;

void main() {
	int offset = texture(texture_0, uv_coords).x;
	frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f) + (offset >= 0 ? nodes[offset].color : vec4(1.0f, 0.0f, 0.0f, 1.0f));
}