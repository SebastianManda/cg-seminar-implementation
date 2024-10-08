#version 450

layout(location = 1) uniform sampler2D tex;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(vec3(texture2D(tex, texCoords).x < 0.2 ? 0 : texture2D(tex, texCoords).x), 1);
}