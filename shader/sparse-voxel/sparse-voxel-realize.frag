#version 450

struct OctreeNode {
	vec4 color;
	// NOTE: only position.xyz are used, use a vec4 here to avoid alignment issues
	vec4 position;
	uint depth;
	int next;
};

layout(std430, binding=1) readonly buffer octree_buffer {
	OctreeNode nodes[];
} OctreeBuffer;

in vec2 uv_coords;

layout(r32i) uniform isampler2D texture_0;

out vec4 frag_color;

void main() {
	int offset = texture(texture_0, uv_coords).x;
	frag_color = offset >= 0 ? OctreeBuffer.nodes[offset].color : vec4(0.0f);
}