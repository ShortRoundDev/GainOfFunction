#include "Thrower.hpp"

#include <iostream>

#include "Managers.hpp"
#include "Fireball.hpp"
#include "glm/gtx/rotate_vector.hpp"

#define IS_HIT (currentAnimation == "rightHit" || currentAnimation == "leftHit" || currentAnimation == "centerHit")


Thrower::Thrower(glm::vec3 position) : Entity(
	position,
	"Resources/LankThrow128.png",
	glm::vec2(0.85f, 0.85f),
	glm::vec2(0.25f, 0.5f),
	1050
) {
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
    dead = false;
}

Thrower::~Thrower() {

}

void Thrower::update() {
    if (dead)
        return;
    if (currentAnimation == "die") {
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
    else if (IS_HIT) {
        if (animations[currentAnimation].checkLooped())
            currentAnimation = "idle";
        else
            animations[currentAnimation].iterate(0.4f);
    }
    else {
        if (PLAYER.health <= 0)
            return;

        switch (state) {
        case ThrowerState::IDLE:
            idleCheckTransitions();
            break;
        case ThrowerState::ATTACKING:
            attackingCheckTransitions();
            break;
        case ThrowerState::FOLLOW:
            followCheckTransitions();
            break;
        case ThrowerState::HUNT:
            huntCheckTransitions();
            break;
        }

        if (currentAnimation == "walk") {
            animations[currentAnimation].iterate(0.07f);
        }
        else if (currentAnimation == "attack") {
            animations[currentAnimation].iterate(0.2f);
        }

        switch (state) {
        case ThrowerState::IDLE:
            idleUpdate();
            break;
        case ThrowerState::ATTACKING:
            attackingUpdate();
            break;
        case ThrowerState::FOLLOW:
            followUpdate();
            break;
        case ThrowerState::HUNT:
            huntUpdate();
            break;
        }
    }

    move();
}

void Thrower::idleCheckTransitions() {
    if (canSeePlayer()) {
        state = ThrowerState::FOLLOW;
        currentAnimation = "walk";
    }
}

void Thrower::followCheckTransitions() {
    if (!canSeePlayer()) {
        lastSeenPlayer = PLAYER.pos;
        if (!path.empty()) {
            path.clear();
        }
        if (!goals.empty()) {
            std::queue<glm::vec3> empty;
            std::swap(goals, empty);
        }
        if (findPathToSpot((int)(PLAYER.pos.x), (int)(PLAYER.pos.z), &path, &goals)) {
            state = ThrowerState::HUNT;
            currentAnimation = "walk";
        }
        else {
            state = ThrowerState::IDLE;
            currentAnimation = "idle";
        }
    }
    else if (DIST_2(PLAYER.pos, position) <= ATTACK_RANGE) {
        state = ThrowerState::ATTACKING;
        currentAnimation = "idle";
    }
}

void Thrower::attackingCheckTransitions() {
    if (currentAnimation == "attack" && !animations[currentAnimation].checkLooped())
        return;
    if (!canSeePlayer()) {
        lastSeenPlayer = PLAYER.pos;
        if (!path.empty()) {
            path.clear();
        }
        if (!goals.empty()) {
            std::queue<glm::vec3> empty;
            std::swap(goals, empty);
        }
        if (findPathToSpot((int)(PLAYER.pos.x), (int)(PLAYER.pos.z), &path, &goals)) {
            state = ThrowerState::HUNT;
        }
        else {
            state = ThrowerState::IDLE;
        }
    }
    else if (DIST_2(PLAYER.pos, position) > ATTACK_RANGE) {
        state = ThrowerState::FOLLOW;
        currentAnimation = "walk";
    }
}

void Thrower::huntCheckTransitions() {
    if (canSeePlayer()) {
        state = ThrowerState::FOLLOW;
        currentAnimation = "walk";
    }
    else if (goals.empty() && currentGoal.x == -1 && currentGoal.z == -1) {
        state = ThrowerState::IDLE;
        currentAnimation = "idle";
    }
}

void Thrower::idleUpdate() {
    currentGoal = glm::vec3(-1);
}

void Thrower::followUpdate() {
    currentGoal = PLAYER.pos;
}

void Thrower::huntUpdate() {
    if (currentGoal.x == -1 && currentGoal.z == -1) {
        // grab new goals
        if (goals.empty()) {
            return;
        }
        currentGoal = goals.front();
        goals.pop();
        return;
    }
    else if (glm::length(glm::vec2(position.x, position.z) - glm::vec2(currentGoal.x, currentGoal.z)) <= GOAL_RANGE) {
        if (goals.empty()) {
            currentGoal = glm::vec3(-1);
            return;
        }
        currentGoal = goals.front();
        goals.pop();
        return;
    }
}

void Thrower::attackingUpdate() {
    currentGoal = glm::vec3(-1);
    front = glm::normalize(glm::vec3(PLAYER.pos.x, 0, PLAYER.pos.z) - glm::vec3(position.x, 0, position.z));
    if (currentAnimation == "attack" && animations[currentAnimation].checkLooped()) {
        whoosh();
        if(rand() % 3 == 0)
            PLAY_I(SoundManager::instance->ghoulMoan[rand() % 2], position);
        GameManager::addEntity(new Fireball(position + glm::vec3(0, 0.4f, 0), front));
        currentAnimation = "idle";
    }

    if (currentAnimation == "idle") {
        if (attackCooldown <= 0) {
            currentAnimation = "attack";
            attackCooldown = 50;
        }
        else {
            attackCooldown--;
        }
    }
}

void Thrower::move() {
    if (currentGoal.x != -1 && currentGoal.z != -1) {
        auto diffVec = currentGoal - position;

        auto diff2 = glm::vec2(diffVec.x, diffVec.z);
        auto front2 = glm::vec2(front.x, front.z);

        auto dp = glm::dot(diff2, front2);
        float s = (2.0f * std::signbit(glm::dot(diff2, glm::rotate(front2, (float)M_PI_2)))) - 1;

        float angle = s * acos(
            dp /
            (glm::length(diff2) * glm::length(front2))
        );

        if (fabs(angle) > 0.1)
            front = glm::rotateY(front, angle * 0.04f);
        moveVec += front * 0.01f;
        moveVec = glm::normalize(moveVec) * 0.02f;
    }
    else {
        moveVec *= 0.9f;
    }
    position = pushWall(position + moveVec);
}

bool Thrower::canSeePlayer() {
    int x, y;
    return GameManager::instance->dda(
        position.x, position.z,
        PLAYER.pos.x, PLAYER.pos.z,
        &x, &y
    );
}

void Thrower::die() {
    currentAnimation = "die";
    position.y = -0.1f;
    (&PLAYER)->killedEnemies++;
    PLAY_I(SoundManager::instance->ghoulDeath[rand() % 2], position);
}

void Thrower::hurt(int damage, glm::vec3 hitPos) {
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
    PLAY_I(SoundManager::instance->ghoulPain[rand() % 3], position);
    attackCooldown = 20;
    animations["attack"].currentFrame = 0;

}

void Thrower::whoosh() {
    auto whooshSound = rand() % 3;
    PLAY_I(SoundManager::instance->whooshSounds[whooshSound], position);
}

void Thrower::whack() {
    auto whackSound = rand() % 2;
    PLAY_I(SoundManager::instance->whackSounds[whackSound], position);
}