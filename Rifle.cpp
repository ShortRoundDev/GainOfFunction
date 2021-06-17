#include "Rifle.hpp"

#include <iostream>
#include "Managers.hpp"

Rifle::Rifle(glm::vec3 pos): Entity(pos, "Resources/riflepickup.png", glm::vec2(0.70f, 0.35f), glm::vec2(0.1f, 0.1f)) {
    frame = 0;
    totalFrames = 1;
}

Rifle::~Rifle() {
    
}

void Rifle::update(){
    auto player = &(GameManager::instance->player);
    if(glm::length(
        glm::vec2(position.x, position.z) -
        glm::vec2(player->pos.x, player->pos.z)
    ) < 0.5f) {
        if(!player->hasRifle)
            player->activeWeapon = 2;
        player->hasRifle = true;
        player->rifleAmmo += 10;
        player->collectedItems++;
        //SoundManager::instance->playSound("Resources/Audio/terminal_05.ogg", glm::vec3(0));
        PLAY_S("Resources/Audio/pickupRifle.ogg", glm::vec3(0.0f));
        GameManager::deleteEntity(this);
    }
}