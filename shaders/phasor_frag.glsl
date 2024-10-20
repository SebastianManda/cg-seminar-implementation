#version 450 core

layout(location = 0) out vec4 fragColor;

// Phasor noise
// https://hal.science/hal-02118508/file/ProceduralPhasorNoise.pdf


#define PI 3.1415926538
#define UINT_MAX 4294967295u
#define EPS 1e-6f

layout(location = 1) uniform sampler2D orientationTex;
layout(location = 2) uniform float Kernel_Harmonic_Frequency_Magnitude;

layout(location = 11) uniform int toggle_ang_profile;
layout(location = 12) uniform float epsilon;
layout(location = 13) uniform int toggle_rad_attenuation;
layout(location = 14) uniform float C;
layout(location = 15) uniform int toggle_transform;
layout(location = 16) uniform float p_complement;

in vec2 texCoords;
// CONFIGURATION

bool ang_profile = toggle_ang_profile == 1 ? true : false;
bool rad_attenuation = toggle_rad_attenuation == 1 ? true : false;
bool transform = toggle_transform == 1 ? true : false;

uint RANDOM_OFFSET = 25565u;

float Kernel_Gaussian_Bandwidth = 0.01;
float Kernel_Harmonic_Frequency_Magnitude_Spread = 0.0;
float Kernel_Harmonic_Frequency_Orientation_Spread = 0.3;
int Kernels_Per_Cell = 16;
int Cell_Size_Factor = 1;

int Oscillation_Profile = 1; // Change to the one in the paper

// computed constants
float Kernel_Gaussian_Radius;
float Cell_Size;
float Kernel_Harmonic_Frequency_Orientation;

void init() {
    Kernel_Gaussian_Radius = 1.0 / Kernel_Gaussian_Bandwidth;
    Cell_Size = Kernel_Gaussian_Radius * Cell_Size_Factor;
    Kernel_Harmonic_Frequency_Orientation = texture2D(orientationTex, texCoords).r;
}

// PRNG
// multiplier from doi:10.1007/bf01937326
uint _prng_x;
void prng_seed(uint s) {
    _prng_x = s;
}
uint prng_next() {
    _prng_x *= 3039177861u;
    // we get mod 2^32 for free since
    // "Operations resulting in overflow ... yield the low-order 32 bits of the result"
    return _prng_x;
}
float prng_01() {
    return float(prng_next()) / float(UINT_MAX);
}
float prng_ab(float a, float b) {
    return a + (b - a) * prng_01();
}

// 2d coordinate to integer (in Morton order) + offset =  non-periodic seed
uint morton(ivec2 xy) {
    int x = xy.x; int y = xy.y;

    // http://www-graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
    int B[4] = int[](0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF);
    int S[4] = int[](1, 2, 4, 8);

    int z; // z gets the resulting 32-bit Morton Number.
    // x and y must initially be less than 65536.

    x = (x | (x << S[3])) & B[3];
    x = (x | (x << S[2])) & B[2];
    x = (x | (x << S[1])) & B[1];
    x = (x | (x << S[0])) & B[0];

    y = (y | (y << S[3])) & B[3];
    y = (y | (y << S[2])) & B[2];
    y = (y | (y << S[1])) & B[1];
    y = (y | (y << S[0])) & B[0];

    z = x | (y << 1);
    return uint(z);
}

// phasor noise
vec2 phasor_kernel(
    float F_0,
    float a,
    float w_0,
    float phi,
    vec2 d
) {
    float sq_a = a * a;
    float sq_x = d.x * d.x;
    float sq_y = d.y * d.y;

    float gaussian = exp(-PI * sq_a * (sq_x + sq_y));

    float s = sin(2.0 * PI * F_0  * (d.x * cos(w_0) + d.y * sin(w_0)) + phi);
    float c = cos(2.0 * PI * F_0  * (d.x * cos(w_0) + d.y * sin(w_0)) + phi);
    return gaussian * vec2(s, c);
}

vec2 phasor_cell(ivec2 ci, vec2 cp) {
    uint seed = morton(ci) + RANDOM_OFFSET;
    prng_seed(seed > 0u ? seed : 1u);

    vec2 noise = vec2(0.0);
    for (int k = 0; k < Kernels_Per_Cell; k++) {
        vec2 kernel_pos = vec2(prng_01(), prng_01());
        vec2 d = (cp - kernel_pos) * Cell_Size;
        float phi = 0.0;
        noise += phasor_kernel(
            Kernel_Harmonic_Frequency_Magnitude + (prng_01() - 0.5) * Kernel_Harmonic_Frequency_Magnitude_Spread,
            Kernel_Gaussian_Bandwidth,
            Kernel_Harmonic_Frequency_Orientation + (prng_01() - 0.5) * Kernel_Harmonic_Frequency_Orientation_Spread,
            phi,
            d
        );
    }

    return noise;
}

vec2 phasor(ivec2 cell_idx, vec2 cell_pos) {
    vec2 noise = vec2(0.0);
    for (int di = -1; di <= 1; di++)
        for (int dj = -1; dj <= 1; dj++)
            noise += phasor_cell(cell_idx+ivec2(di,dj), cell_pos-vec2(di,dj));

    return noise;
}

float angular_profile(float phi, float epsilon) {
    float inner = (1.0 + 2.0 * sqrt(epsilon)) * pow(phi / PI, 2) - epsilon;
    return sqrt(inner) - sqrt(epsilon);
}

float radial_attenuation(vec2 g, float c) {
    return (2.0f / PI) * atan(c * length(g));
}

float H(vec2 g, float epsilon, float c, float p_complement) {
    float rad = radial_attenuation(g, c);
    float ang = angular_profile(atan(g.y, g.x), epsilon);

    return rad * ang + (1.0 - rad) * p_complement;
}

void main() {
    init();

    vec2 cell_uv = gl_FragCoord.xy / Cell_Size;
    ivec2 cell_idx = ivec2(cell_uv);
    vec2 cell_pos = cell_uv - vec2(cell_idx);

    vec2 noise = phasor(cell_idx, cell_pos);
    float phi = atan(noise.y, noise.x);

    float color = phi;
    if (ang_profile) color = angular_profile(phi, epsilon);
    if (rad_attenuation) color = radial_attenuation(noise, C);
    if (transform) color = H(noise, epsilon, C, p_complement);

    fragColor = vec4(vec3(color), 1.0);
}