#include "Ammo.hpp"

#include "Managers.hpp"

Ammo::Ammo(glm::vec3 pos) : Entity(pos, "Resources/ammo.png", glm::vec2(0.2, 0.2), glm::vec2(0.7, 0.2)){
}

Ammo::~Ammo(){
    
}

void Ammo::update(){
    if(glm::length(glm::vec2(position.x, position.z) - glm::vec2(PLAYER.pos.x, PLAYER.pos.z)) < radiusX){
        SoundManager::instance->playSound("Resources/Audio/ammo_pickup.ogg", PLAYER.pos + CAMERA.cameraFront, 1.0f);
        (&PLAYER)->ammo += 10;
        (&PLAYER)->collectedItems++;
        GameManager::instance->deleteEntity(this);
    }
}