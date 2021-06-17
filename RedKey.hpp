#pragma once
#include "Entity.hpp"

#include "glm/glm.hpp"

class RedKey : public Entity {
public:
    RedKey(glm::vec3 position);
    ~RedKey();
    
    void update();
};