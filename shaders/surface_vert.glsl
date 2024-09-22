#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
layout(location = 2) uniform mat3 normalModelMatrix;
layout(location = 3) uniform sampler2D heights;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 kd;
layout(location = 3) in vec3 ks;
layout(location = 4) in float shininess;
layout(location = 5) in float roughness;
layout(location = 6) in vec2 texCoord;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragKd;
out vec3 fragKs;
out float fragShininess;
out float fragRoughness;
out vec2 fragTexCoord;


void main() {
    vec2 texPos = vec2((position.x + 1.0) / 2, (position.z + 1.0) / 2);
    vec3 newPos = vec3(position.x, texture(heights, texPos).r * 0.2, position.z);

    gl_Position = mvpMatrix * vec4(newPos, 1);

    fragPos = (modelMatrix * vec4(newPos, 1)).xyz;
    fragNormal = vec3(0, 1, 0);
    fragKd = vec3(0, 0.5, 0) * texture(heights, texPos).r + vec3(0, 0, 0.5) * (1 - texture(heights, texPos).r);
    fragKs = vec3(0.3, 0.3, 0.3);
    fragShininess = shininess;
    fragRoughness = roughness;
    fragTexCoord = texCoord;
}