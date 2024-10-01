#include "surface_mesh.h"

#include <iostream>
#include <framework/disable_all_warnings.h>

DISABLE_WARNINGS_PUSH()

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "glm/gtx/string_cast.hpp"

DISABLE_WARNINGS_POP()

SurfaceMesh::SurfaceMesh() : SurfaceMesh(400, 2.0f) {}
SurfaceMesh::SurfaceMesh(const int resolution, const float size) {
    m_resolution = previous_resolution = resolution;
    m_size = previous_size = size;
    generate();
}

void SurfaceMesh::update() {
    if (m_resolution != previous_resolution || m_size != previous_size || m_filled != previous_filled) {
        previous_resolution = m_resolution;
        previous_size = m_size;
        previous_filled = m_filled;
        generate();
    }
}

void SurfaceMesh::generate() {
    m_vertices.clear();
    m_faces.clear();
    createVertices(0.0f);
    createFaces(0);

    if (m_filled) fillSurface();

    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, static_cast<GLsizeiptr>(m_vertices.size() * sizeof(glm::uvec3)),m_vertices.data(), 0);

    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glNamedBufferStorage(ibo, static_cast<GLsizeiptr>(m_faces.size() * sizeof(glm::uvec3)),m_faces.data(), 0);

    glCreateVertexArrays(1, &m_vao);
    glVertexArrayElementBuffer(m_vao, ibo);

    glVertexArrayVertexBuffer(m_vao, 0, vbo, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);
}

void SurfaceMesh::createVertices(float y) {
    const float margin_offset = m_size * 0.005f;
    const float offset = (m_size * 2 - 2 * margin_offset) / static_cast<float>(m_resolution - 1);
    const float start_offset = margin_offset - m_size;

    for (int i = 0; i < m_resolution; i++) {
        const auto i_f = static_cast<float>(i);
        for (int j = 0; j < m_resolution; j++) {
            const auto j_f = static_cast<float>(j);
            m_vertices.emplace_back(start_offset + offset * i_f, y, start_offset + offset * j_f);
        }
    }
}

void SurfaceMesh::createFaces(int startingIndex) {
    for (int i = 0; i < m_resolution - 1; i++) {
        for (int j = 0; j < m_resolution - 1; j++) {
            int current = startingIndex + i * m_resolution + j;
            int above = current + m_resolution;
            m_faces.emplace_back(current, current + 1, above + 1);
            m_faces.emplace_back(above + 1, above, current);
        }
    }
}

void SurfaceMesh::fillSurface() {
    auto sizeVertices = m_vertices.size();
    createVertices(-0.05f);
    createFaces(static_cast<int>(sizeVertices));

    for (int i = 0; i < m_resolution - 1; i++) {
        int current = i;
        m_faces.emplace_back(current, current + 1, sizeVertices + current + 1);
        m_faces.emplace_back(sizeVertices + current + 1, sizeVertices + current, current);

        current = (m_resolution - 1) * m_resolution + i;
        m_faces.emplace_back(current, current + 1, sizeVertices + current + 1);
        m_faces.emplace_back(sizeVertices + current + 1, sizeVertices + current, current);

        current = i * m_resolution;
        m_faces.emplace_back(current, current + m_resolution, sizeVertices + current + m_resolution);
        m_faces.emplace_back(sizeVertices + current + m_resolution, sizeVertices + current, current);

        current = i * m_resolution + m_resolution - 1;
        m_faces.emplace_back(current, current + m_resolution, sizeVertices + current + m_resolution);
        m_faces.emplace_back(sizeVertices + current + m_resolution, sizeVertices + current, current);
    }
}

void SurfaceMesh::draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_faces.size() * sizeof(glm::uvec3)), GL_UNSIGNED_INT, nullptr);
}