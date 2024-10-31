#include "orientation.h"

#include <algorithm>
#include <numeric>
#include <queue>
#include <framework/disable_all_warnings.h>
#include <stb/stb_image_write.h>
DISABLE_WARNINGS_PUSH()
#include <fmt/format.h>
DISABLE_WARNINGS_POP()
#include <framework/image.h>

#include <iostream>
Orientation::Orientation(std::vector<float> dem) : Orientation(dem, false) {}
Orientation::Orientation(std::vector<float> dem, bool upscale) {
    m_dem = dem;
    m_res = std::sqrt(m_dem.size());
    int new_size = upscale ? std::pow(m_res * 4, 2) : m_dem.size();
    m_gradient = std::vector(new_size, 0.0f);
    m_orientation = std::vector(new_size, 0.0f);
    m_gradientX = std::vector(new_size, 0.0f);
    m_gradientY = std::vector(new_size, 0.0f);


    m_gradientMap.Init(glm::ivec2(m_res));
    m_orientationMap.Init(glm::ivec2(m_res));
    m_smoothedDemMap.Init(glm::ivec2(m_res));
}

void Orientation::process() {
    process(false);
}

void Orientation::process(bool upscale) {
    if (upscale) {
        upscaleDem();
        smoothDem(5);
        // smoothDem(5);
        // smoothDem(5);
        // smoothDem(5);
    }

    if (!upscale) {
        computeGradient();
        computeOrientation();
    }

    if (upscale) {
        float max = *std::max_element(m_dem.begin(), m_dem.end());
        std::vector<glm::u8vec4> textureData8Bits(m_dem.size());
        for (int i = 0; i < m_dem.size(); i++) {
            float val = std::clamp(m_dem[i] / max, 0.0f, 1.0f) * 255.0f;
            textureData8Bits[i] = glm::u8vec4(glm::vec3(val), 255.0f);
        }
        if (m_res == 2048 || m_res == 512) stbi_write_bmp("../2.png", m_res, m_res, 4, textureData8Bits.data());
        else stbi_write_bmp("../1.png", m_res, m_res, 4, textureData8Bits.data());
    }

    if (!upscale) {
        glBindTexture(GL_TEXTURE_2D, m_gradientMap.getMap());
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_res, m_res, 0, GL_RED, GL_FLOAT, m_gradient.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, m_orientationMap.getMap());
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_res, m_res, 0, GL_RED, GL_FLOAT, m_orientation.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindTexture(GL_TEXTURE_2D, m_smoothedDemMap.getMap());
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_res, m_res, 0, GL_RED, GL_FLOAT, m_dem.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Orientation::computeGradient() {
    for (int i = 0; i < m_dem.size(); i++) {
        std::vector<int> neighbours = getNeighbours(i);
        if (neighbours.size() != 4) {
            m_gradient[i] = 0.0f;
            m_gradientX[i] = 0.0f;
            m_gradientY[i] = 0.0f;
            continue;
        }

        float dx = std::abs(m_dem[neighbours[1]] - m_dem[neighbours[0]]) / 2.0f;
        float dy = std::abs(m_dem[neighbours[3]] - m_dem[neighbours[2]]) / 2.0f;
        m_gradientX[i] = dx;
        m_gradientY[i] = dy;
        m_gradient[i] = std::atan2(dy, dx);
    }

    // Smooth Gradient
    std::vector<float> new_grad = std::vector(m_gradient.size(), 0.0f);
    std::vector<float> new_gradX = std::vector(m_gradientX.size(), 0.0f);
    std::vector<float> new_gradY = std::vector(m_gradientY.size(), 0.0f);
    for (int i = 0; i < m_dem.size(); i++) {
        std::vector<int> neighbours = getNeighboursNXN(i, 4);

        float sum = m_gradient[i];
        float sumX = m_gradientX[i];
        float sumY = m_gradientY[i];
        for (int neighbour : neighbours) {
            sum += m_gradient[neighbour];
            sumX += m_gradientX[neighbour];
            sumY += m_gradientY[neighbour];
        }
        new_grad[i] = sum / (static_cast<float>(neighbours.size()) + 1);
        new_gradX[i] = sumX / (static_cast<float>(neighbours.size()) + 1);
        new_gradY[i] = sumY / (static_cast<float>(neighbours.size()) + 1);
    }

    m_gradient = new_grad;
    m_gradientX = new_gradX;
    m_gradientY = new_gradY;
}

void Orientation::computeOrientation() {
    for (int i = 0; i < m_dem.size(); i++)
        m_orientation[i] = std::atan2(m_gradientY[i], m_gradientX[i]) + std::numbers::pi / 2.0f;
}

std::vector<int> Orientation::getNeighbours(int index, bool diagonal) {
    std::vector<int> neighbours;
    if (index % m_res != 0) neighbours.push_back(index - 1);
    if (index % m_res != m_res - 1) neighbours.push_back(index + 1);
    if (index >= m_res) neighbours.push_back(index - m_res);
    if (index < m_res * (m_res - 1)) neighbours.push_back(index + m_res);
    if (diagonal) {
        if (index % m_res != 0 && index >= m_res) neighbours.push_back(index - m_res - 1);
        if (index % m_res != m_res - 1 && index >= m_res) neighbours.push_back(index - m_res + 1);
        if (index % m_res != 0 && index < m_res * (m_res - 1)) neighbours.push_back(index + m_res - 1);
        if (index % m_res != m_res - 1 && index < m_res * (m_res - 1)) neighbours.push_back(index + m_res + 1);
    }
    return neighbours;
}

std::vector<int> Orientation::getNeighboursNXN(int index, int n) {
    std::vector<int> neighbours;
    for (int i = -n; i <= n; i++) {
        for (int j = -n; j <= n; j++) {
            if (std::sqrt(i * i + j * j) > n) continue;
            int current_i = index / m_res;
            if (index + i * m_res > m_dem.size()) continue;
            if (index + i * m_res < 0) continue;
            if (index + j < current_i * m_res) continue;
            if (index + j > (current_i + 1) * m_res) continue;
            neighbours.push_back(index + i * m_res + j);
        }
    }

    return neighbours;
}

void Orientation::upscaleDem() {
    float res_off = 4;
    int new_res = m_res * res_off;
    std::vector<float> new_dem = std::vector(std::pow(new_res, 2), 0.0f);

    for (int i = 0; i < new_res; i++) {
        for (int j = 0; j < new_res; j++) {
            int low_i = i / res_off;
            int low_j = j / res_off;

            int i_offset = i % int(res_off);
            int j_offset = j % int(res_off);

            float c_h = m_dem[low_i * m_res + low_j];
            float n_i_1 = low_i > 0 ? m_dem[(low_i - 1) * m_res + low_j] : c_h;
            float n_i_2 = low_i < m_res ? m_dem[(low_i + 1) * m_res + low_j] : c_h;
            float n_j_1 = low_j > 0 ? m_dem[low_i * m_res + low_j - 1] : c_h;
            float n_j_2 = low_j < m_res ? m_dem[low_i * m_res + low_j + 1] : c_h;

            float n_i = i_offset <= 2 ? n_i_1 : n_i_2;
            float n_j = j_offset <= 2 ? n_j_1 : n_j_2;
            float i_coeff = (i_offset == 0 || i_offset == 3) ? 0.875f : 0.625f;
            float j_coeff = (j_offset == 0 || j_offset == 3) ? 0.875f : 0.625f;

            new_dem[i * new_res + j] = ((i_coeff * c_h + (1.0f - i_coeff) * n_i) + (j_coeff * c_h + (1.0f - j_coeff) * n_j)) / 2.0f;
            // new_dem[i * new_res + j] = m_dem[low_i * m_res + low_j];
        }
    }
    m_dem = new_dem;
    m_res = new_res;
}

void Orientation::smoothDem(int n) {
    std::vector<float> smooth_dem = std::vector(m_dem.size(), 0.0f);
    for (int i = 0; i < m_dem.size(); i++) {
        std::vector<int> neighbours = getNeighboursNXN(i, n);
        float sum = m_dem[i];
        for (int neighbour : neighbours) sum += m_dem[neighbour];
        smooth_dem[i] = sum / (static_cast<float>(neighbours.size()) + 1);
    }

    m_dem = smooth_dem;
}