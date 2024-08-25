#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : enable

layout(scalar, buffer_reference) readonly buffer VertexBuff{
	vec2 pos[];
};

layout(scalar, push_constant) uniform constants
{
	VertexBuff vertices;
}ps;

layout(location = 0) out vec3 color;
void main(){
	gl_Position = vec4(ps.vertices.pos[gl_VertexIndex], 0.0, 1.0);
	color = vec3(0.0, 1.0, 0.0);
}