#pragma once

#include "Entity.hpp"
#include "glm/glm.hpp"

#include "AL/al.h"

class Ammo : public Entity {
public:
    Ammo(glm::vec3 pos);
    ~Ammo();
    
    ALuint pickupSound;
    
    void update();
};