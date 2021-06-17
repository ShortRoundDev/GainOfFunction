#include "BulletHole.hpp"

#include "Managers.hpp"

BulletHole::BulletHole(glm::vec3 pos) : Entity(pos, "Resources/BulletHole.png", glm::vec2(0.2f, 0.2f), glm::vec2(0.0f, 0.0f)){
    lifetime = 0;
    totalFrames = 5;
}

BulletHole::~BulletHole() {
    
}

void BulletHole::update() {
    lifetime++;
    if(lifetime > 20){
        GameManager::deleteEntity(this);
    }
    frame = (float)(lifetime/5);
}