#version 450

layout(set = 0, binding = 0) uniform sampler2D uAtlas;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    float alpha = texture(uAtlas, fragTexCoord).r;
    outColor = vec4(fragColor.rgb * alpha, alpha);
}
