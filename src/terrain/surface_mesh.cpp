#include "surface_mesh.h"

#include <iostream>
#include <framework/disable_all_warnings.h>

DISABLE_WARNINGS_PUSH()

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "glm/gtx/string_cast.hpp"

DISABLE_WARNINGS_POP()

SurfaceMesh::SurfaceMesh() : SurfaceMesh(400, 400, 5.0f) {}
SurfaceMesh::SurfaceMesh(const int width, const int height, const float size) {
    m_width = width;
    m_height = height;
    m_size = size;
    generate();
}

void SurfaceMesh::setResolution(const int width, const int height) {
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        generate();
    }
}

void SurfaceMesh::generate() {
    const float x_offset = (m_size * 2) / static_cast<float>(m_width - 1);
    const float z_offset = (m_size * 2) / static_cast<float>(m_height - 1);

    m_vertices.clear();
    for (int i = 0; i < m_width; i++) {
        const auto i_f = static_cast<float>(i);
        for (int j = 0; j < m_height; j++) {
            const auto j_f = static_cast<float>(j);
            m_vertices.emplace_back(-m_size + x_offset * i_f, 0.1, -m_size + z_offset * j_f);
        }
    }

    // std::cout << "New fucking one" << std::endl;
    // for (auto vec : vertices) {
    //     std::cout << glm::to_string(vec) << std::endl;
    // }

    m_faces.clear();
    for (int i = 0; i < m_width - 1; i++) {
        for (int j = 0; j < m_height - 1; j++) {
            int current = i * m_height + j;
            int above = (i+1) * m_height + j;
            m_faces.emplace_back(current, current + 1, above + 1);
            m_faces.emplace_back(above + 1, above, current);
        }
    }

    // std::cout << "Faces" << std::endl;
    // for (auto vec : faces) {
    //     std::cout << glm::to_string(vec) << std::endl;
    // }

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

void SurfaceMesh::draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_faces.size() * sizeof(glm::uvec3)), GL_UNSIGNED_INT, nullptr);
}