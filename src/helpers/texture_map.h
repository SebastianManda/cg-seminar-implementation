#include <vector>
#include <framework/disable_all_warnings.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()

class TextureMap {
public:
    TextureMap();
    TextureMap(glm::ivec2 res);
    ~TextureMap();

    void Init();

    void bindWrite();
    void bindRead(GLenum TextureUnit);
    std::vector<float> getData();
    GLuint getMap() const { return m_map; }

public:
    glm::ivec2 m_resolution{526};

private:
    GLuint m_fbo{0};
    GLuint m_map{0};
};
