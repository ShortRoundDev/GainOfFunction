#pragma once
#include "Entity.hpp"

#include "glm/glm.hpp"

class BlueKey : public Entity {
public:
    BlueKey(glm::vec3 position);
    ~BlueKey();
    
    void update();
};