#pragma once

#include "Entity.hpp"

#include "glm/glm.hpp"

class ZombieGib : public Entity {
public:
    ZombieGib(glm::vec3 pos, glm::vec2 scale);
    ~ZombieGib();
    
    int lifetime = 0;
    
    void update();

};