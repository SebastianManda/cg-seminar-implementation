#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
// Normals should be transformed differently than positions:
// https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
layout(location = 2) uniform mat3 normalModelMatrix;

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

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1);
    
    fragPos = (modelMatrix * vec4(position, 1)).xyz;
    fragNormal = normalModelMatrix * normal;
    fragKd = kd;
    fragKs = ks;
    fragShininess = shininess;
    fragRoughness = roughness;
    fragTexCoord = texCoord;
}
