#version 450

layout(location = 0) in vec2 inPosition;

layout(std430, push_constant) uniform PushConstants {
    mat4 transform;
    vec4 color;
} pushConstants;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = pushConstants.transform * vec4(inPosition, 0.0, 1.0);
    fragColor = pushConstants.color;
}
