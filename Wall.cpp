#include "Wall.hpp"

#include <iostream>
#include "GameManager.hpp"

#include "Managers.hpp"

Wall::Wall() {
    this->displacement = glm::vec3(0.0f, 0.0f, 0.0f);
    openProgress = 0.0f;
    isOpen = false;
    isOpening = false;
    tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

Wall::~Wall() {
    
}

void Wall::update(){
    if (isDoor) {
        if (isOpening) {
            if (openProgress < 1.0f) {
                openProgress += 0.03f;
            }
            if (openProgress >= 1.0f) {
                openProgress = 1.0f;
                isOpen = true;
                isOpening = false;
            }
            this->displacement = glm::vec3(0.0f, openProgress * 0.9f, 0.0f);
        }
        else if (isOpen) {
            this->displacement = glm::vec3(0.0f, 0.9, 0.0f);
        }
    }
}

void Wall::open(){
    if(isOpen || isOpening)
        return;
    if(
        (key == 1 && !PLAYER.hasRedKey) ||
        (key == 2 && !PLAYER.hasBlueKey) ||
        (key == 3 && !PLAYER.hasYellowKey)
    ){
        return;
    }
    PLAY_S("Resources/Audio/doorOpen.ogg", glm::vec3((float)x + 0.5f, 0.5f, (float)y + 0.5f));


    this->isOpening = true;
    openProgress = 0.0f;
}

void Wall::draw() {
    
}