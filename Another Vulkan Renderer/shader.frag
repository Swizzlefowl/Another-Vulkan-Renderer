#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 fragColor;
layout(location = 0) out vec4 color;
layout(binding = 1) uniform texture2D albedo[];
layout(binding = 0) uniform sampler samp;

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
	uint index;
}ps;

void main(){
	color = texture(sampler2D(albedo[ps.index], samp), uv);
}
