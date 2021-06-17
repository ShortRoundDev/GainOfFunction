#pragma once

#include <map>
#include <queue>

#include "Entity.hpp"

class Zombie : public Entity{
public:
    Zombie(glm::vec3 pos);
    
    std::map<uint32_t, uint32_t> path;
    
    bool seesPlayer = false;
    glm::vec3 lastSawPlayer;
    glm::vec3 currentGoal;
    
    std::queue<glm::vec3> goals;
    
    glm::vec3 frontVec;
    
    void update();
    
private:
    void wander();
    void attackPlayer();
    void seekPlayer();
    void goToGoal(glm::vec3 goal);
    void generatePath();
};