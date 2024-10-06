#version 450

layout(location = 5) uniform vec3 viewPos;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragKd;
in vec3 fragKs;
in float fragShininess;
in float fragRoughness;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

vec3 lightPos = vec3(3.0, 3.0, 3.0);

float lambert() {
    return max(dot(fragNormal, normalize(lightPos - fragPos)), 0.0);
}

float blinnPhong() {
    vec3 H = normalize(viewPos - fragPos + lightPos - fragPos);
    vec3 N = normalize(fragNormal);
    float d = dot(H, N);
    if (dot(lightPos - fragPos, fragNormal) <= 0.0) d = 0.0;
    return pow(d, fragShininess);
}

void main()
{
    const vec3 normal = normalize(fragNormal);
    fragColor = vec4(fragKd, 1);
//    fragColor = vec4(lambert() * fragKd, 1);
//    fragColor = vec4(lambert() * fragKd + blinnPhong() * fragKs, 1);
//    fragColor = vec4(vec3(blinnPhong()), 1);
}
