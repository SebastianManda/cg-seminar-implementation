#include <vector>
#include <framework/disable_all_warnings.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
DISABLE_WARNINGS_POP()

class AmplitudeMap {
public:
    AmplitudeMap();
    AmplitudeMap(glm::ivec2 res);
    ~AmplitudeMap();

    void Init();

    void bindWrite();
    void bindRead(GLenum TextureUnit);

public:
    glm::ivec2 m_resolution{526};

private:
    GLuint m_fbo{0};
    GLuint m_amplitudeMap{0};
};
