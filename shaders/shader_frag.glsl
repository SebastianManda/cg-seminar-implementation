#version 450

layout(location = 5) uniform vec3 viewPos;
layout(location = 6) uniform sampler2D tex;
layout(location = 7) uniform vec3 riverOptions;
layout(location = 8) uniform bool useTextureCoords;
layout(location = 9) uniform bool useOrientation;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragKd;
in vec3 fragKs;
in float fragShininess;
in float fragRoughness;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;

#define PI 3.1415926538

vec3 lightPos = vec3(3.0, 3.0, 3.0);
bool useRiver = riverOptions.x == 1.0 ? true : false;
float rivelMultiplier = riverOptions.y;
float riverThreshold = riverOptions.z;

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

    vec3 kd = fragKd;
    if (useRiver) {
        if (texture(tex, fragTexCoord).r > riverThreshold)
            kd = vec3(0, 0, texture(tex, fragTexCoord).r * rivelMultiplier);
    }
    if (useOrientation) kd = vec3(texture(tex, fragTexCoord).r / (2*PI));
    if (useTextureCoords) kd = vec3(texture(tex, fragTexCoord).r);
    if (fragPos.y < 0.0) kd = fragKd;

    fragColor = vec4(kd, 1);
//    fragColor = vec4(lambert() * fragKd, 1);
//    fragColor = vec4(lambert() * fragKd + blinnPhong() * fragKs, 1);
//    fragColor = vec4(vec3(blinnPhong()), 1);
}
