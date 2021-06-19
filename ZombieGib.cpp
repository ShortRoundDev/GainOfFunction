#include "ZombieGib.hpp"

#include "Managers.hpp"

ZombieGib::ZombieGib(glm::vec3 pos, glm::vec2 scale): Entity(
    pos,
    "Resources/pop.png",
    scale,
    glm::vec2(0, 0),
    1300
) {
    totalFrames = 8;
    currentAnimation = "pop";
    animations["pop"] = { 0, 8, 0 };
}

ZombieGib::~ZombieGib(){
    
}

void ZombieGib::update(){

    if (animations["pop"].checkLooped()) {
        GameManager::deleteEntity(this);
        return;
    }
    animations["pop"].iterate(0.3f);
}