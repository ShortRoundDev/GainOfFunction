#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "Shader.hpp"

class Entity {
public:
    static void init(Shader* shader);

    Entity(glm::vec3, std::string texture, glm::vec2 scale, glm::vec2 radius);
    Entity(glm::vec3, GLuint texture, glm::vec2 scale, glm::vec2 radius);
    virtual ~Entity();
    float radiusX;
    float radiusY;
    
    int hurtTimer = 0;
    int health;
    bool shootable = false;
    
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 moveVec;
    float accel;
    
    void draw();
    virtual void update();
    glm::vec2 scale;
    
    static GLuint vao;
    static Shader* shader;
    float frame = 0;
    float totalFrames = 1;
    
    void hurt(int damage);
    void die();
protected:
    glm::vec3 pushWall(glm::vec3 newPos);
    Shader* shaderOverride = nullptr;
private:
    GLuint texture;
    
    static void uploadVertices();
};