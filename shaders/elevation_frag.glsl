#version 450

layout(location = 1) uniform sampler2D terrainTex;
layout(location = 2) uniform sampler2D detailsTex;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    float color = texture2D(terrainTex, texCoords).r + texture2D(detailsTex, texCoords).r * 0.05;
    fragColor = vec4(vec3(color), 1.0);
}