#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 fragColor;
layout(location = 0) out vec4 color;
layout(binding = 0) uniform texture2D albedo;
layout(binding = 1) uniform sampler samp;

void main(){
	color = texture(sampler2D(albedo, samp), uv);
}
