#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : enable

struct Attrib{
	vec3 pos;
    vec3 color;
    vec2 texCoord;
};

layout(scalar, buffer_reference, buffer_reference_align = 8) readonly buffer VertexBuff{
	    Attrib attribs[];
};

layout(scalar, push_constant) uniform constants
{
	VertexBuff vertices;
}ps;

layout(location = 0) out vec3 color;
void main(){
	gl_Position = vec4(ps.vertices.attribs[gl_VertexIndex].pos, 1.0);
	color = vec3(0.0, 1.0, 0.0);
}