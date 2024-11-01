#version 450

layout(location = 0) uniform mat4 mvpMatrix;
layout(location = 1) uniform mat4 modelMatrix;
layout(location = 2) uniform mat3 normalModelMatrix;
layout(location = 3) uniform sampler2D heights;
layout(location = 4) uniform vec4 meshSettings;

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
out vec2 fragTexCoord;

const vec3 c0 = vec3(38/255.0, 40/255.0, 0/255.0); // 0.0
const vec3 c1 = vec3(101/255.0, 90/255.0, 31/255.0); // 0.25
const vec3 c2 = vec3(113/255.0, 94/255.0, 52/255.0); // 0.5
const vec3 c3 = vec3(107/255.0, 75/255.0, 32/255.0); // 0.75
const vec3 c4 = vec3(42/148.0, 29/255.0, 8/255.0); // 1.0

const float heightScale = meshSettings.x;
const float meshScale = meshSettings.y;
const float meshRes = meshSettings.z;
const bool useColor = meshSettings.w == 1.0 ? true : false;

vec3 computeStepKd(float h) {
    if (h < 0.25) return c0 + (c1 - c0) * h * 4.0;
    else if (h < 0.5) return c1 + (c2 - c1) * (h - 0.25) * 4.0;
    else if (h < 0.75) return c2 + (c3 - c2) * (h - 0.5) * 4.0;
    else return c3 + (c4 - c3) * (h - 0.75) * 4.0;
}

vec3 computeNewPos(vec3 pos) {
    vec2 texPos = vec2((pos.x + 1) / 2, (pos.z + 1) / 2);
    return vec3(pos.x * meshScale, texture(heights, texPos).x * heightScale, pos.z * meshScale);
}

vec3 computeNormal() {
    float offset = 1.990 / (meshRes - 1.0);

    vec3 pos = computeNewPos(vec3(position.x, 0, position.z));
    vec3 posL = computeNewPos(vec3(position.x - offset, 0, position.z));
    vec3 posR = computeNewPos(vec3(position.x + offset, 0, position.z));
    vec3 posD = computeNewPos(vec3(position.x, 0, position.z - offset));
    vec3 posU = computeNewPos(vec3(position.x, 0, position.z + offset));

    vec3 n1 = normalize(cross(normalize(posU - pos), normalize(posR - pos)));
    vec3 n2 = normalize(cross(normalize(posR - pos), normalize(posD - pos)));
    vec3 n3 = normalize(cross(normalize(posD - pos), normalize(posL - pos)));
    vec3 n4 = normalize(cross(normalize(posL - pos), normalize(posU - pos)));

    return normalize(n1 + n2 + n3 + n4);
}

void main() {
    vec2 texPos = vec2((position.x + 1) / 2, (position.z + 1) / 2);
    vec3 newPos = vec3(position.x * meshScale, texture(heights, texPos).x * heightScale, position.z * meshScale);
    newPos = position.y < 0 ? vec3(position.x * meshScale, position.y, position.z * meshScale) : newPos;
    vec3 kd = position.y < 0 ? (useColor ? c0 : vec3(0)) : useColor ? computeStepKd(newPos.y / heightScale) : vec3(texture(heights, texPos).x);

    gl_Position = mvpMatrix * vec4(newPos, 1);

    fragPos = (modelMatrix * vec4(newPos, 1)).xyz;
    fragNormal = computeNormal();
    fragKd = kd;
    fragKs = vec3(0.2);
    fragShininess = 0.1;
    fragTexCoord = texPos;
}