#version 450

layout(location = 1) uniform sampler2D tex;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(texture2D(tex, texCoords).xyz, 1);
}