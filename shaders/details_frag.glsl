#version 450

layout(location = 1) uniform sampler2D amplitudeTex;
layout(location = 2) uniform sampler2D noiseTex;

in vec2 texCoords;

layout(location = 0) out vec4 fragColor;

void main() {
    float details = texture2D(amplitudeTex, texCoords).r * texture2D(noiseTex, texCoords).r;
    fragColor = vec4(vec3(details), 1.0);
}