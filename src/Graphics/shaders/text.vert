#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(std430, push_constant) uniform PushConstants {
    vec2 viewportSize;
    vec4 color;
} pushConstants;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;

void main() {
    float x = (inPosition.x / pushConstants.viewportSize.x) * 2.0f - 1.0f;
    float y = (inPosition.y / pushConstants.viewportSize.y) * 2.0f - 1.0f;
    gl_Position = vec4(x, y, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    fragColor = pushConstants.color;
}
