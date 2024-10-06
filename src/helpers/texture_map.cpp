#include "texture_map.h"

#include <iostream>
#include <framework/disable_all_warnings.h>

DISABLE_WARNINGS_PUSH()

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "glm/gtx/string_cast.hpp"

DISABLE_WARNINGS_POP()

TextureMap::TextureMap() : TextureMap(glm::ivec2(526, 526)) {}
TextureMap::TextureMap(glm::ivec2 res) : m_resolution(res) {}

void TextureMap::Init() {
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the color buffer
    glGenTextures(1, &m_map);
    glBindTexture(GL_TEXTURE_2D, m_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_resolution.x, m_resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

TextureMap::~TextureMap() {
    glDeleteTextures(1, &m_map);
    glDeleteFramebuffers(1, &m_fbo);
}

void TextureMap::bindWrite() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport( 0, 0, m_resolution.x, m_resolution.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_map, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void TextureMap::bindRead(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_map);
}