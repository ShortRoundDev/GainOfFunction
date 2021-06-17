#pragma once

#include "Entity.hpp"
#include "glm/glm.hpp"

#include "AL/al.h"

class RifleAmmo : public Entity {
public:
    RifleAmmo(glm::vec3 pos);
    ~RifleAmmo();
    
    ALuint pickupSound;
    
    void update();
};