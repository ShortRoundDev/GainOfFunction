#pragma once
#include "Entity.hpp"

#include "glm/glm.hpp"

class Pistol : public Entity {
public:
    Pistol(glm::vec3 position);
    ~Pistol();
    
    void update();
};