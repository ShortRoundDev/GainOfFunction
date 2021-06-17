#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

#include <string>

class Shader {
public:
    Shader(std::string vertex, std::string fragment);
    ~Shader();
    
    void use();
    
    void setInt(const std::string &name, int texture);
    void setFloat(const std::string &name, float value);
    void setVec3(const std::string &name, const glm::vec3 &vec);
    void setVec4(const std::string &name, const glm::vec4 &vec);
    void setMat4(const std::string &name, const glm::mat4 &matrix);
    void setVec2(const std::string &name, const glm::vec2 &vec);

    
private:
    GLuint program;
    int checkCompilationErrors(GLuint shader, std::string type);
};