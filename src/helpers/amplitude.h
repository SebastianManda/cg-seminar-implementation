#pragma once
#include <vector>
#include <framework/disable_all_warnings.h>

#include "texture_map.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <exception>
#include <filesystem>
#include <framework/opengl_includes.h>

struct pqValue {
    int index;
    float value;
};

struct {
    bool operator()(const pqValue l, const pqValue r) const { return l.value > r.value; }
} pqCompMin;

struct {
    bool operator()(const pqValue l, const pqValue r) const { return l.value < r.value; }
} pqCompMax;


class Amplitude {
public:
    Amplitude(std::vector<float> dem);

    void process();
    void compute(float threshold, float increment);


public:
    TextureMap m_amplitudeMap;
    TextureMap m_drainageMap;

private:
    void fillPits();
    void computeFlowDirections();
    void computeAccumulation();
    void computeAccumulationHelper(int index);

    std::vector<int> getNeighbours(int index, bool diagonal);

private:
    std::vector<float> m_dem;
    std::vector<float> m_accumulation;
    std::vector<float> m_amplitudes;
    std::vector<int> m_flows;
    int m_res;
};
