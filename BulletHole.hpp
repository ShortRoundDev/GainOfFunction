#pragma once

#include "Entity.hpp"
#include "glm/glm.hpp"

class BulletHole : public Entity {
public:
    BulletHole(glm::vec3 pos);
    ~BulletHole();
    
    int lifetime = 0;
    
    void update();
};