#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

#include <map>
#include <queue>

#include "Shader.hpp"
#include "AnimationDesc.h"

class Entity {
public:
    static void init(Shader* shader);

    Entity(glm::vec3, std::string texture, glm::vec2 scale, glm::vec2 radius, int entityType);
    Entity(glm::vec3, GLuint texture, glm::vec2 scale, glm::vec2 radius);
    virtual ~Entity();
    float radiusX;
    float radiusY;
    
    int hurtTimer = 0;
    int health;
    bool shootable = false;
    
    std::map<std::string, AnimationDesc> animations;
    int totalAngles = 1;
    std::string currentAnimation = "idle";
    int totalFrames; // max frames of ALL animations

    void getCurrentAnimation(AnimationDesc** desc);

    int entityType = 0;
    bool dead = false;

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 moveVec;
    float accel;
    
    virtual void draw();
    virtual void update();
    glm::vec2 scale;
    
    static GLuint vao;
    static Shader* shader;
    
    virtual void hurt(int damage, glm::vec3 pos);
    virtual void die();
protected:
    glm::vec3 pushWall(glm::vec3 newPos);
    Shader* shaderOverride = nullptr;

    bool findPathToEntity(Entity* entity, std::map<uint32_t, uint32_t>* path, std::queue<glm::vec3>* goals);
    bool findPathToSpot(int x, int y, std::map<uint32_t, uint32_t>* path, std::queue<glm::vec3>* goals);
    bool optimizePath(int x, int y, std::map<uint32_t, uint32_t>* path, std::queue<glm::vec3>* goals);
private:
    GLuint texture;
    
    static void uploadVertices();
};