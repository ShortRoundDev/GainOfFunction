#pragma once
#include "Entity.hpp"

#include "glm/glm.hpp"

class YellowKey : public Entity {
public:
    YellowKey(glm::vec3 position);
    ~YellowKey();
    
    void update();
};