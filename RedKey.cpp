#include "RedKey.hpp"

#include <iostream>
#include "Managers.hpp"

RedKey::RedKey(glm::vec3 pos): Entity(pos, 1000, glm::vec2(0.25f, 0.25f), glm::vec2(0.1f, 0.1f)) {
    animations["idle"] = { 0, 1, 0 };
}

RedKey::~RedKey() {
    
}

void RedKey::update(){
    Player* player = &(GameManager::instance->player);
    if(glm::length(
        glm::vec2(position.x, position.z) -
        glm::vec2(player->pos.x, player->pos.z)
    ) < 0.5f) {
        player->hasRedKey = true;
        player->collectedItems++;
        SoundManager::instance->playSound("Resources/Audio/terminal_05.ogg", glm::vec3(0), 1.0f);
        GameManager::deleteEntity(this);
    }
}