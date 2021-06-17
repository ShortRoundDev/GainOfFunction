#pragma once
#include "Entity.hpp"

#include "glm/glm.hpp"

class Rifle : public Entity {
public:
    Rifle(glm::vec3 position);
    ~Rifle();
    
    void update();
};