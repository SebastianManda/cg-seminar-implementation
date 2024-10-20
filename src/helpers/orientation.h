#pragma once
#include <cmath>
#include <vector>
#include <framework/disable_all_warnings.h>

#include "texture_map.h"
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <exception>
#include <filesystem>
#include <framework/opengl_includes.h>

class Orientation {
public:
    Orientation(std::vector<float> dem);

    void process();

public:
    TextureMap m_gradientMap;
    TextureMap m_gradientMapX;
    TextureMap m_gradientMapY;
    TextureMap m_orientationMap;

private:
    void computeGradient();
    void computeOrientation();

    std::vector<int> getNeighbours(int index);

private:
    std::vector<float> m_dem;
    std::vector<float> m_gradient;
    std::vector<float> m_gradientX;
    std::vector<float> m_gradientY;
    std::vector<float> m_orientation;
    int m_res;

};
