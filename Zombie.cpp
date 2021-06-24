#include "Zombie.hpp"

#include <iostream>
#include <cstdlib>
#include <cmath>

#include "glm/gtx/rotate_vector.hpp"
#include "Managers.hpp"

#define IS_HIT (currentAnimation == "rightHit" || currentAnimation == "leftHit" || currentAnimation == "centerHit")

Zombie::Zombie(glm::vec3 pos):
    Entity(
        pos + glm::vec3(0.0f, 0.0f, 0.0f),
        "Resources/Lank128.png",
        glm::vec2(0.85f, 0.85f),
        glm::vec2(0.25f, 0.5f),
        1003
    ),
    currentGoal(-1.0f, -1.0f, -1.0f)
{
    front = glm::vec3(0, 0, 1.0f);
    totalFrames = 24;
    totalAngles = 8;
    animations["idle"] = { 2, 3, 0 };
    animations["walk"] = { 0, 2, 0 };
    animations["attack"] = { 4, 7, 0 };
    animations["rightHit"] = { 8, 11, 0 };
    animations["leftHit"] = { 11, 14, 0 };
    animations["centerHit"] = { 14, 17, 0 };
    animations["die"] = { 17, 23, 0 };
    currentAnimation = "idle";
    shootable = true;
    health = 7;
}

void Zombie::update(){
    int x, y;

#ifdef DISABLE_ZOMBIE_AI
    return;
#endif

    if (dead)
        return;

    if (IS_HIT) {
        if (animations[currentAnimation].checkLooped())
            currentAnimation = "idle";
        else
            animations[currentAnimation].iterate(0.2f);
    }
    else if (currentAnimation == "die") {
        animations[currentAnimation].iterate(0.2f);
        if (animations[currentAnimation].checkLooped()) {
            dead = true;
            whack();
            PLAY_I(SoundManager::instance->fleshSound, position);
            animations[currentAnimation].currentFrame = 6;
            position.y = -0.1f;
        }
        return;
    }
    else if (currentAnimation == "attack") {
        animations[currentAnimation].iterate(0.3f);
        if (animations[currentAnimation].checkLooped()) {
            (&PLAYER)->hurt(2);
            whack();
            PLAY_I(SoundManager::instance->fleshSound, position);

            auto pDiff = glm::vec2(PLAYER.pos.x, PLAYER.pos.z) - glm::vec2(position.x, position.z);
            if (glm::length(pDiff) < 0.6f) {
                (&PLAYER)->moveVec += glm::normalize(glm::vec3(pDiff.x, 0, pDiff.y)) * 0.5f;
                (&PLAYER)->accel = 1.0f;
            }
            currentAnimation = "idle";
        }
        return;
    }
    else {
        animations[currentAnimation].iterate(0.07f);
    }

    if (PLAYER.health <= 0)
        return;
    bool seen = GameManager::instance->dda(
        position.x, position.z,
        PLAYER.pos.x, PLAYER.pos.z,
        &x, &y
    );
    //see player, attack
    if(seen){
        if(!goals.empty()){
            std::queue<glm::vec3> empty;
            std::swap(goals, empty);
        }
        if(!path.empty())
            path.clear();
        attackPlayer();
        seesPlayer = true;
        lastSawPlayer = glm::vec3(PLAYER.pos.x, 0, PLAYER.pos.z);

    } else {
        // just lost sight of player, hunt
        if(seesPlayer) {
            generatePath();
            seesPlayer = false;
            if(!goals.empty()){
                currentGoal = goals.front();
                goals.pop();
            }
        }
        // already hunting, follow goal
        else if(currentGoal.x != -1 && currentGoal.y != -1) {
            auto dist = glm::distance(position, currentGoal);
            //reached goal, get new
            if(dist < 0.1){
                if(!goals.empty()){
                    currentGoal = goals.front();
                    goals.pop();
                } else {
                    // out of goals
                    currentAnimation = "idle";
                    currentGoal = glm::vec3(-1, 0, -1);
                }
            }
        }
    }

    position = pushWall(position + moveVec);

    if(hurtTimer > 0) {
        hurtTimer--;
        moveVec *= 0.9f;
    }
    else if(currentGoal.x != -1 && currentGoal.z != -1)
    {
        goToGoal(currentGoal);
    } else {
    }
    auto pDiff = glm::vec2(PLAYER.pos.x, PLAYER.pos.z) - glm::vec2(position.x, position.z);
    if (glm::length(pDiff) < 0.6 && PLAYER.damageBoost <= 0) {
        moveVec = glm::vec3(0);
        whoosh();
        currentAnimation = "attack";
    }
}

void Zombie::wander() {
    return; // sit still for now
}

void Zombie::attackPlayer(){
    currentGoal = PLAYER.pos;
    if (!(IS_HIT) && hurtTimer <= 0 && currentAnimation != "walk") {
        currentAnimation = "walk";
        PLAY_I(SoundManager::instance->ghoulMoan[rand() % 2], position);
    }
}

void Zombie::goToGoal(glm::vec3 goal) {
    
    auto diffVec = goal - position;
    
    auto diff2 = glm::vec2(diffVec.x, diffVec.z);
    auto front2 = glm::vec2(front.x, front.z);
    
    auto dp = glm::dot(diff2, front2);
    float s = (2.0f * std::signbit(glm::dot(diff2, glm::rotate(front2, (float)M_PI_2)))) - 1;
    
    float angle = s * acos(
        dp/
        (glm::length(diff2) * glm::length(front2))
    );
    
    if(fabs(angle) > 0.1)
        front = glm::rotateY(front, angle * 0.04f);
    moveVec += front * 0.01f;
    moveVec = glm::normalize(moveVec) * 0.02f;
    //position += moveVec;
}

void Zombie::seekPlayer() {
}

void Zombie::generatePath() {
    bool found = GameManager::instance->bfs((int16_t)position.x, (int16_t)position.z, (int16_t)lastSawPlayer.x, (int16_t)lastSawPlayer.z, path);
    if(goals.empty() && found){
        //Start backwards
        glm::vec3 last = position;

        float itX = (float)((int)PLAYER.pos.x) + 0.5f;
        float itY = (float)((int)PLAYER.pos.z) + 0.5f;
        uint32_t it = PACK_COORDS((int)itX, (int)itY);
        
        bool quit = (itX == (int)last.x) && (itY == (int)last.z);
        //safety valve
        int counter = 0;
        while(!quit && counter < 512){
            int x, y;
            counter++;
            if(GameManager::instance->dda(
                last.x,
                last.z,
                itX,
                itY,
                &x, &y
            )){
                last = glm::vec3(itX, 0, itY);
                goals.push(last);
                it = PACK_COORDS((int)PLAYER.pos.x, (int)PLAYER.pos.z);
                itX = (float)((int)PLAYER.pos.x) + 0.5f;
                itY = (float)((int)PLAYER.pos.z) + 0.5f;
            } else {
                it = path[it];
                itX = (float)(UNPACK_X(it)) + 0.5f;
                itY = (float)(UNPACK_Y(it)) + 0.5f;
            }
            quit = ((int)itX == (int)last.x) && ((int)itY == (int)last.z);
        }
    }
    int counter = 0;
    if(goals.empty() && found) { // raycasting failed
        uint32_t goal = path[PACK_COORDS((int16_t)lastSawPlayer.x, (int16_t)lastSawPlayer.z)];
        uint32_t start = PACK_COORDS((int16_t)position.x, (int16_t)position.z);
        while(goal != start && counter < 512) {
            auto x = UNPACK_X(goal);
            auto y = UNPACK_Y(goal);
            goals.push(glm::vec3(x, 0, y));
            goal = path[goal];
            counter++;
        }
    }
}

void Zombie::hurt(int damage, glm::vec3 hitPos) {
    float dist;
    auto side = GraphicsManager::isLeft(position, position + front, hitPos, &dist);
    if (abs(dist) < 0.04f) {
        auto flatNormal = CAMERA.cameraFront;
        moveVec += glm::normalize(glm::vec3(flatNormal.x, 0, flatNormal.z)) * 0.1f;
        currentAnimation = "centerHit";
    }
    else if (side) {
        auto flatNormal = glm::rotate(CAMERA.cameraFront, (float)(-M_PI / 2.0f), glm::vec3(0, 1, 0)) + CAMERA.cameraFront;
        moveVec += glm::normalize(glm::vec3(flatNormal.x, 0, flatNormal.z)) * 0.1f;
    }
    else {
        auto flatNormal = glm::rotate(CAMERA.cameraFront, (float)(M_PI / 2.0f), glm::vec3(0, 1, 0)) + CAMERA.cameraFront;
        moveVec += glm::normalize(glm::vec3(flatNormal.x, 0, flatNormal.z)) * 0.1f;
    }
    Entity::hurt(damage, hitPos);
    PLAY_I(SoundManager::instance->ghoulPain[rand() % 2], position);
}

void Zombie::die() {
    currentAnimation = "die";
    position.y = -0.1f;
    (&PLAYER)->killedEnemies++;
    PLAY_I(SoundManager::instance->ghoulDeath[rand() % 2], position);
}

void Zombie::whoosh() {
    PLAY_I(SoundManager::instance->whooshSound, position);
}

void Zombie::whack() {
    auto whackSound = rand() % 2;
    PLAY_I(SoundManager::instance->whackSounds[whackSound], position);
}