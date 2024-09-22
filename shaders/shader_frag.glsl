#version 450

layout(location = 3) uniform sampler2D texColor;
layout(location = 4) uniform bool hasTexCoords;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragKd;
in vec3 fragKs;
in float fragShininess;
in float fragRoughness;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

vec3 lightPos = vec3(0.0, 2.0, 0.0);

float lambert() {
    return max(dot(fragNormal, normalize(lightPos - fragPos)), 0.0);
}

void main()
{
    const vec3 normal = normalize(fragNormal);

    if (hasTexCoords) {
        fragColor = vec4(texture(texColor, fragTexCoord).rgb, 1);
    } else {
        fragColor = vec4(lambert() * fragKd, 1);
    }
}
