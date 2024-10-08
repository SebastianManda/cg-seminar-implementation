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

public:
    TextureMap m_amplitudeMap;
    TextureMap m_drainageMap;

private:
    void fillPits();
    void computeFlowDirections();
    void computeAccumulation();
    void computeAccumulationHelper(int index);

    std::vector<int> getNeighbours(int index);

private:
    std::vector<float> m_dem;
    int m_res;
    std::vector<int> m_flows;
    std::vector<float> m_accumulation;
};
