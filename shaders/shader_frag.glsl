#version 450

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragKd;
in vec3 fragKs;
in float fragShininess;
in float fragRoughness;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

vec3 lightPos = vec3(0.0, 5.0, 0.0);

float lambert() {
    return max(dot(fragNormal, normalize(lightPos - fragPos)), 0.0);
}

void main()
{
    const vec3 normal = normalize(fragNormal);
    fragColor = vec4(lambert() * fragKd, 1);
}
