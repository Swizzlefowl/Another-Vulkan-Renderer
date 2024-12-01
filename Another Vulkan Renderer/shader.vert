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
	mat4x4 mvp;
}ps;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 fragColor;

void main(){
	gl_Position = ps.mvp * vec4(ps.vertices.attribs[gl_VertexIndex].pos, 1.0);
	fragColor = vec3(0.0, 1.0, 0.0);
	uv = ps.vertices.attribs[gl_VertexIndex].texCoord;
}