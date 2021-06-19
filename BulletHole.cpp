#include "BulletHole.hpp"

#include "Managers.hpp"

BulletHole::BulletHole(glm::vec3 pos) : Entity(
    pos,
    "Resources/BulletHole.png",
    glm::vec2(0.2f, 0.2f),
    glm::vec2(0.0f, 0.0f),
    1301
){
    totalFrames = 5;
    currentAnimation = "poof";
    animations["poof"] = { 0, 5, 0 };
}

BulletHole::~BulletHole() {
    
}

void BulletHole::update() {
    if (animations["poof"].checkLooped()) {
        GameManager::deleteEntity(this);
        return;
    }
    animations["poof"].iterate(0.2f);

}