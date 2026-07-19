#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(push_constant) uniform PushConstants {
    mat3 transform;
} pushConstants;

layout(location = 0) out vec4 fragColor;

void main() {
    vec3 position = pushConstants.transform * vec3(inPosition, 1.0);
    gl_Position = vec4(position.xy, 0.0, 1.0);
    fragColor = inColor;
}
