#include "amplitude.h"

#include <algorithm>
#include <numeric>
#include <queue>
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <fmt/format.h>
DISABLE_WARNINGS_POP()
#include <framework/image.h>

#include <iostream>

Amplitude::Amplitude(std::vector<float> dem) {
    m_dem = dem;
    m_res = std::sqrt(m_dem.size());
    m_flows = std::vector<int>(m_dem.size());
    m_accumulation = std::vector<float>(m_dem.size(), 0.001f);
    m_amplitudeMap.Init();
    m_drainageMap.Init();
}

void Amplitude::process() {
    fillPits();
    computeFlowDirections();
    computeAccumulation();


    glBindTexture(GL_TEXTURE_2D, m_drainageMap.getMap());
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_res, m_res, 0, GL_RED, GL_FLOAT, m_accumulation.data());
    glBindTexture(GL_TEXTURE_2D, 0);


}

void Amplitude::fillPits() {
    std::priority_queue<pqValue, std::vector<pqValue>, decltype(pqCompMin)> open;
    std::queue<pqValue> pit;
    std::vector closed(m_dem.size(), false);

    for (int i = 0; i < m_res; i++) {
        open.emplace(i, m_dem[i]);
        open.emplace((m_res - 1) * m_res + i, m_dem[(m_res - 1) * m_res + i]);
        closed[i] = true;
        closed[(m_res - 1) * m_res + i] = true;
    }
    for (int i = 1; i < m_res - 1; i++) {
        open.emplace(i * m_res, m_dem[i * m_res]);
        open.emplace(i * m_res + m_res - 1, m_dem[i * m_res + m_res - 1]);
        closed[i * m_res] = true;
        closed[i * m_res + m_res - 1] = true;
    }

    while (!open.empty() || !pit.empty()) {
        pqValue c;
        if (!pit.empty()) {
            c = pit.front();
            pit.pop();
        } else {
            c = open.top();
            open.pop();
        }

        std::vector<int> neighbours = getNeighbours(c.index);
        for (int n : neighbours) {
            if (closed[n]) continue;
            closed[n] = true;

            if (m_dem[n] <= m_dem[c.index]) {
                m_dem[n] = m_dem[c.index];
                pit.emplace(n, m_dem[n]);
            } else open.emplace(n, m_dem[n]);
        }
    }
}

void Amplitude::computeFlowDirections() {
    std::priority_queue<pqValue, std::vector<pqValue>, decltype(pqCompMin)> open;
    std::vector closed(m_dem.size(), false);

    for (int i = 0; i < m_res; i++) {
        open.emplace(i, m_dem[i]);
        open.emplace((m_res - 1) * m_res + i, m_dem[(m_res - 1) * m_res + i]);
        closed[i] = true;
        closed[(m_res - 1) * m_res + i] = true;
        m_flows[i] = -1;
        m_flows[(m_res - 1) * m_res + i] = -1;
    }
    for (int i = 1; i < m_res - 1; i++) {
        open.emplace(i * m_res, m_dem[i * m_res]);
        open.emplace(i * m_res + m_res - 1, m_dem[i * m_res + m_res - 1]);
        closed[i * m_res] = true;
        closed[i * m_res + m_res - 1] = true;
        m_flows[i * m_res] = -1;
        m_flows[i * m_res + m_res - 1] = -1;
    }

    while (!open.empty()) {
        pqValue c = open.top();
        open.pop();

        std::vector<int> neighbours = getNeighbours(c.index);
        for (int n : neighbours) {
            if (closed[n]) continue;
            m_flows[n] = c.index;
            closed[n] = true;
            open.emplace(n, m_dem[n]);
        }
    }
}

void Amplitude::computeAccumulation() {
    for (int i = 0; i < m_dem.size(); i++) computeAccumulationHelper(i);
}

void Amplitude::computeAccumulationHelper(int index) {
    int target = m_flows[index];
    if (target == -1) return;
    m_accumulation[target] += 0.001f;
    computeAccumulationHelper(target);
}

std::vector<int> Amplitude::getNeighbours(int index) {
    std::vector<int> neighbours;
    if (index % m_res != 0) neighbours.push_back(index - 1);
    if (index % m_res != m_res - 1) neighbours.push_back(index + 1);
    if (index >= m_res) neighbours.push_back(index - m_res);
    if (index < m_res * (m_res - 1)) neighbours.push_back(index + m_res);
    if (index % m_res != 0 && index >= m_res) neighbours.push_back(index - m_res - 1);
    if (index % m_res != m_res - 1 && index >= m_res) neighbours.push_back(index - m_res + 1);
    if (index % m_res != 0 && index < m_res * (m_res - 1)) neighbours.push_back(index + m_res - 1);
    if (index % m_res != m_res - 1 && index < m_res * (m_res - 1)) neighbours.push_back(index + m_res + 1);
    return neighbours;
}
