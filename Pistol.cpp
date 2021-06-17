#include "Pistol.hpp"

#include <iostream>
#include "Managers.hpp"

Pistol::Pistol(glm::vec3 pos): Entity(pos, "Resources/pistolpickup.png", glm::vec2(0.35f, 0.35f), glm::vec2(0.1f, 0.1f)) {
    frame = 0;
    totalFrames = 1;
}

Pistol::~Pistol() {
    
}

void Pistol::update(){
    Player* player = &(GameManager::instance->player);
    if(glm::length(
        glm::vec2(position.x, position.z) -
        glm::vec2(player->pos.x, player->pos.z)
    ) < 0.5f) {
        if(!player->hasPistol)
            player->activeWeapon = 1;
        player->hasPistol = true;
        player->ammo += 10;
        player->collectedItems++;
        //SoundManager::instance->playSound("Resources/Audio/terminal_05.ogg", glm::vec3(0));
        PLAY_S("Resources/Audio/pickupPistol.ogg", glm::vec3(0.0f));
        GameManager::deleteEntity(this);
    }
}