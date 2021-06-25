#include "Player.hpp"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <tuple>

#include "GameManager.hpp"
#include "Camera.hpp"
#include "GLFW/glfw3.h"

#include "Managers.hpp"
#include "BulletHole.hpp"
#include "ZombieGib.hpp"

#include "TileDef.h"

Player::Player(glm::vec3 startPos):
    pos(startPos),
    moveVec(0, 0, 0),
    moveDir(0, 0, 0){
    
    gun = GraphicsManager::loadTex("Resources/gun.png", GL_BGRA);
    rifle = GraphicsManager::loadTex("Resources/rifle.png", GL_BGRA);
    GraphicsManager::loadTex("Resources/crosshair.png", GL_BGRA);
    pistolAmmoIndicator = GraphicsManager::loadTex("Resources/ammo.png", GL_BGRA);
    rifleAmmoIndicator = GraphicsManager::loadTex("Resources/rifleammo.png", GL_BGRA);
    popupSign = GraphicsManager::loadTex("Resources/popupsign.png", GL_BGRA);

    greenHealth = GraphicsManager::loadTex("Resources/GreenHealth.png", GL_BGRA);
    yellowHealth = GraphicsManager::loadTex("Resources/YellowHealth.png", GL_BGRA);
    redHealth = GraphicsManager::loadTex("Resources/RedHealth.png", GL_BGRA);

    zoneHistory.push_back(-1);
    zoneHistory.push_back(-1);
}

Player::~Player(){
    
}

void Player::update(GLFWwindow* window){
    Camera* camera = &(GameManager::instance->camera);
    camera->cameraPos = this->pos;

    if (damageBoost > 0)
        damageBoost--;
    move(window);
    collide();

    if (colleenKillingCooldown > 0) {
        pos.y = 0.5f + ((((float)(100 - colleenKillingCooldown)) / 100.0f) * 0.3f);
        colleenKillingCooldown--;
    }
    else if (colleenKillingCooldown == 0) {
        health = 0;
        colleenKillingCooldown = -2;
        auto whackSound = rand() % 2;
        PLAY_I(SoundManager::instance->whackSounds[whackSound], pos);
        PLAY_I(SoundManager::instance->fleshSound, pos);
    }

    Wall* currentWall = TRY_WALL((int)pos.x, (int)pos.z);
    if (currentWall != NULL) {
        actualZone = currentWall->zone;
        if (currentWall->zone != currentZone && std::find(zoneHistory.begin(), zoneHistory.end(), currentWall->zone) == zoneHistory.end()) {
            currentZone = currentWall->zone;
            if (currentZone != -1) { // being chased
                zonesCrossed++;
            }
            zoneHistory.push_back(currentZone);
            zoneHistory.pop_front();
        }
    }

    if (health <= 0) {
        if (this->pos.y > 0.1f) {
            if (colleenKillingCooldown == -1) {
                this->pos.y -= 0.01f;
            }
            else if (colleenKillingCooldown == -2) {
                this->pos.y -= 0.04f;
            }
        }
        else {
            this->pos.y = 0.1f;
        }
        if (this->dieMessageLocation < 64.0f) {
            this->dieMessageLocation += 0.5f;
        }
        return;
    }

    gunTheta += 0.1f;
    
    if(gunFrame > 0.0f){
        gunFrame += 0.3f;
    }
    if(gunFrame >= 5.0f){
        gunFrame = 0.0f;
        pistolCooldown = 10;
    }
    
    if(rifleFrame > 0.0f){
        rifleFrame += 0.3f;
    }
    if(rifleFrame >= 25.0f){
        rifleFrame = 0.0f;
        rifleCooldown = 15;
    }
    if (health >= 5) {
        healthFrame += 0.5f;
    }
    else if (health >= 3) {
        healthFrame += 0.75f;
    }
    else if (health >= 1) {
        healthFrame += 1.0f;
    }
    if (healthFrame >= 66.0f) {
        healthFrame = 0.0f;
    }
    if (rifleCooldown > 0)
        rifleCooldown--;
    if (pistolCooldown > 0)
        pistolCooldown--;
}

void Player::move(GLFWwindow* window) {
    bool moving = false;
    Camera* camera = &(GameManager::instance->camera);
    moveDir = glm::vec3(0, 0, 0);
    if (health > 0 && colleenKillingCooldown == -1 && GameManager::instance->currentLevel->colleen == NULL || !GameManager::instance->currentLevel->colleen->dead) {
        if (GameManager::keyMap[GLFW_KEY_W]) {
            moving = true;
            moveDir += glm::normalize(glm::vec3(camera->cameraFront.x, 0.0f, camera->cameraFront.z));
            //camera.cameraPos += cameraSpeed * camera.cameraFront;
        }
        if (GameManager::keyMap[GLFW_KEY_S]) {
            //camera.cameraPos -= cameraSpeed * camera.cameraFront;
            moving = true;
            moveDir -= glm::normalize(glm::vec3(camera->cameraFront.x, 0.0f, camera->cameraFront.z));
        }
        if (GameManager::keyMap[GLFW_KEY_A]) {
            moving = true;
            moveDir -= glm::normalize(glm::cross(camera->cameraFront, camera->cameraUp));
            //camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
        }
        if (GameManager::keyMap[GLFW_KEY_D]) {
            moving = true;
            moveDir += glm::normalize(glm::cross(camera->cameraFront, camera->cameraUp));
            //camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
        }
    }
    if(!moving){
        this->accel *= 0.7f;
    } else {
        accel += PLAYER_ACCEL;
        accel = glm::clamp(accel, -0.03f, 0.03f);
    }
    
    if(moving && (abs(moveDir.x) > 0 || abs(moveDir.z) > 0)){
        moveDir = glm::normalize(moveDir);
        moveVec = moveDir * accel;
    } else {
        moveVec *= 0.7;
    }
}
    
void Player::collide() {
    auto nextStep = (pos + moveVec);
    if(!noClip)
        pos = pushWall(nextStep);
    else
        pos = nextStep;
}

glm::vec3 Player::pushWall(glm::vec3 newPos) {
    
    //check if inside wall after newPos
    
    int x = (int)pos.x,
        y = (int)pos.z;
        
    float xDiff = 0.0f,
          yDiff = 0.0f;
    
    if(IN_BOUNDS(x + 1, y)) {
        // Check right wall
        auto nextWall = WALLS[COORDS(x + 1, y)];
        auto distX = (((float)x + 1.0f) - newPos.x);
        if(distX < radius && SOLID(nextWall)){
            xDiff = -(radius - distX);
        }
    }if(IN_BOUNDS(x - 1, y)){
        auto nextWall = WALLS[COORDS(x - 1, y)];
        auto distX = (newPos.x - (float)x);
        if(distX < radius && SOLID(nextWall)){
            xDiff = radius - distX;
        }
    }
    
    if(IN_BOUNDS(x, y + 1)) {
        auto nextWall = WALLS[COORDS(x, y + 1)];
        auto distY = ((float)y + 1.0f) - newPos.z;
        if(distY < radius && SOLID(nextWall)){
            yDiff = -(radius - distY);
        }
    } if(IN_BOUNDS(x, y - 1)){
        auto nextWall = WALLS[COORDS(x, y - 1)];
        auto distY = (newPos.z - (float)y);
        if(distY < radius && SOLID(nextWall)){
            yDiff = radius - distY;
        }
    }
    
    if(xDiff == 0.0f && yDiff == 0.0f){
        //time to check the corners...
        //lets just push X for simplicity. Always lean left/right I guess
        
        //top right
        if(IN_BOUNDS(x + 1, y + 1)){
            auto nextWall = WALLS[COORDS(x + 1, y + 1)];
            
            auto dist = glm::length(glm::vec3(((float)x + 1), newPos.y, ((float)y + 1)) - newPos);
            if(dist < radius && SOLID(nextWall)){
                auto pushBack = glm::normalize(newPos - glm::vec3(x + 1, 0, y + 1)) * ((radius) - dist);
                xDiff = pushBack.x;
                yDiff = pushBack.z;
                accel = 0;
                moveVec.x = 0;
                moveVec.z = 0;
            }
        }
        //bottom right
        if(IN_BOUNDS(x + 1, y - 1)){
            auto nextWall = WALLS[COORDS(x + 1, y - 1)];
            
            auto dist = glm::length(glm::vec3(((float)x + 1), newPos.y, ((float)y)) - newPos);
            if(dist < radius && SOLID(nextWall)){
                auto pushBack = glm::normalize(glm::vec3(newPos - glm::vec3(x + 1, 0, y))) * ((radius) - dist);
                xDiff = pushBack.x;
                yDiff = pushBack.z;
                accel = 0;
                moveVec.x = 0;
                moveVec.z = 0;
            }
        }
        // Bottom left
        if(IN_BOUNDS(x - 1, y - 1)) {
            auto nextWall = WALLS[COORDS(x - 1, y - 1)];
            
            auto dist = glm::length(glm::vec3(((float)x), newPos.y, ((float)y)) - newPos);

            if(dist < radius && SOLID(nextWall)){
                auto pushBack = glm::normalize(glm::vec3(newPos - glm::vec3(x, 0, y))) * ((radius) - dist);
                xDiff = pushBack.x;
                yDiff = pushBack.z;
                accel = 0;
                moveVec.x = 0;
                moveVec.z = 0;
            }
        }
        // top left
        if(IN_BOUNDS(x - 1, y + 1)){
            auto nextWall = WALLS[COORDS(x - 1, y + 1)];
            auto dist = glm::length(glm::vec3(((float)x), newPos.y, ((float)y + 1)) - newPos);

            
            if(dist < radius && SOLID(nextWall)){
                auto pushBack = glm::normalize(glm::vec3(newPos - glm::vec3(x, 0, y + 1))) * ((radius) - dist);
                xDiff = pushBack.x;
                yDiff = pushBack.z;
                accel = 0;
                moveVec.x = 0;
                moveVec.z = 0;
            }
        }
    }
    
    return newPos + glm::vec3(xDiff, 0.0f, yDiff);
}

void Player::keyHandler(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (colleenKillingCooldown > 0) {
        return;
    }
    if (health <= 0) {
        if (key == GLFW_KEY_E && action == GLFW_PRESS) {
            dieMessageLocation = -32.0f;
            colleenKillingCooldown = -1;
            GameManager::instance->load("Resources/" + GameManager::instance->currentLevel->levelName, true, false);
        }
        else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
            delete GameManager::instance->currentLevel;
            GameManager::instance->currentLevel = NULL;
            GameManager::instance->state = MAIN_MENU;
        }
        return;
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        GameManager::instance->currentLevel->switchesOff = 0;
    }
    if(key == GLFW_KEY_E && action == GLFW_PRESS) {
        Wall* occupied = TRY_WALL(((int)pos.x), ((int)pos.z));
        if (occupied != NULL && !GameManager::showingMessage && WALL_IS_KIND(occupied, TV_SCREEN) && occupied->message != NULL) {
            GameManager::displayMessage(occupied->message);
        }
        else if (WALL_IS_KIND(occupied, SWITCH_OFF)) {
            occupied->wallTexture = SWITCH_ON;
            PLAY_S("Resources/Audio/click.ogg", pos);
            LEVEL->switchesOff--;
            LEVEL->switchNotification = 200;
        }

        auto look = glm::normalize(glm::vec3(CAMERA.cameraFront.x, 0.0f, CAMERA.cameraFront.z));
        auto lookX = look.x;
        auto lookY = look.z;
        for(int i = -1; i < 2; i++){
            int x = (int)pos.x;
            int y = (int)pos.z;
            
            if(lookX >= 0.5){
                y += i;
                x += 1;
            } else if(lookX <= -0.5){
                y += i;
                x -= 1;
            } else if(lookY >= 0.5) {
                x += i;
                y += 1;
            } else {
                x += i;
                y -= 1;
            }
            if(x >= 0 && y >= 0 && x < LEVEL->width && y < LEVEL->height){
                Wall* wall = TRY_WALL(x, y);
                if (wall == NULL)
                    break;
                if(WALL_IS_KIND(wall, ELEVATOR) && wall->message != NULL && wall->message[0] != 0) {
                    GameManager::instance->levelChanging = true;
                    GameManager::instance->nextLevel = wall->message;
                    break;
                }
                if(IS_DOOR((*wall)) && !wall->isOpen) {
                    if (wall->wallTexture == 1 || wall->wallTexture == 2) {
                        // secret door
                        foundSecrets++;
                    }
                    wall->open();
                    break;
                }
                
            }
        }
    }
    if(key == GLFW_KEY_N && action == GLFW_PRESS) {
        noClip ^= true;
    }
    
    if(key == GLFW_KEY_1 && action == GLFW_PRESS && hasPistol){
        activeWeapon = WEP_PISTOL;
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS && hasRifle){
        activeWeapon = WEP_RIFLE;   
    }
    
}

void Player::draw() {

    if (GameManager::instance->currentLevel->colleen != NULL && GameManager::instance->currentLevel->colleen->dead)
        return;

    if (health >= 5) {
        GameManager::drawAnimatedUi(greenHealth, SCREEN_X(74), SCREEN_Y(768 - 10), SCREEN_W(256), SCREEN_H(128), floor(healthFrame), 66.0f);
    }
    else if (health >= 3) {
        GameManager::drawAnimatedUi(yellowHealth, SCREEN_X(74), SCREEN_Y(768 - 10), SCREEN_W(256), SCREEN_H(128), floor(healthFrame), 66.0f);
    }
    else if (health >= 1) {
        GameManager::drawAnimatedUi(redHealth, SCREEN_X(74), SCREEN_Y(768 - 10), SCREEN_W(256), SCREEN_H(128), floor(healthFrame), 66.0f);
    }
    else{
        // Draw flat line
        PRINT("YOU ARE DEAD", SCREEN_X(128), SCREEN_Y(dieMessageLocation), 0.08);
        PRINT("PRESS E TO LOAD LAST SAVE\nPRESS Q TO QUIT", SCREEN_X(128), SCREEN_Y(798.0f - (dieMessageLocation * 2)), 0.08f);
        return;
    }

    auto shader = SHADERS["UI"];
    shader->use();
    shader->setFloat("frame", 0);
    shader->setFloat("maxFrame", 1);
    glBindVertexArray(Entity::vao);
    if(hasRedKey) {
        glBindTexture(GL_TEXTURE_2D, GraphicsManager::findTex(1000));
        shader->setVec3("scale", glm::vec3(
            SCREEN_W(64),
            SCREEN_H(64),
            1.0f
        ));
        shader->setVec3("offset", glm::vec3(
            SCREEN_X(1024 - 48),
            SCREEN_Y(768 - 48),
            0.0f
        ));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    if(hasBlueKey) {
        glBindTexture(GL_TEXTURE_2D, GraphicsManager::findTex(1001));
        shader->setVec3("scale", glm::vec3(
            SCREEN_W(64),
            SCREEN_H(64),
            1.0f
        ));
        shader->setVec3("offset", glm::vec3(
            SCREEN_X(1024 - 48),
            SCREEN_Y(768 - 96),
            0.0f
        ));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    if(hasYellowKey) {
        glBindTexture(GL_TEXTURE_2D, GraphicsManager::findTex(1002));
        shader->setVec3("scale", glm::vec3(
            SCREEN_W(64),
            SCREEN_H(64),
            1.0f
        ));
        shader->setVec3("offset", glm::vec3(
            SCREEN_X(1024 - 48),
            SCREEN_Y(768 - 144),
            0.0f
        ));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    

    if(activeWeapon == WEP_PISTOL){
        drawPistol();
    } else if(activeWeapon == WEP_RIFLE){
        drawRifle();
    }    
    
    if(GameManager::instance->bright){
        glBindTexture(GL_TEXTURE_2D, GraphicsManager::findTex("Resources/crosshair.png"));
        shader->setFloat("frame", 0);
        shader->setFloat("maxFrame", 1.0f);
        shader->setVec3("scale", glm::vec3(
            SCREEN_W(32),
            SCREEN_H(32),
            1.0f
        ));
        shader->setVec3("offset", glm::vec3(
            SCREEN_X(512 - 16),
            SCREEN_Y(768 / 2 - 16),
            0.0f
        ));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glClear(GL_DEPTH_BUFFER_BIT);
    Wall* occupied = TRY_WALL(((int)pos.x), ((int)pos.z));
    if (occupied == NULL)
        return;
    if(WALL_IS_KIND(occupied, WALL_SIGN)){
        if(occupied->message != NULL){
            shader->use();
            glBindTexture(GL_TEXTURE_2D, popupSign);
            shader->setFloat("frame", 0);
            shader->setFloat("maxFrame", 1.0f);
            shader->setVec3("scale", glm::vec3(
                SCREEN_W(2048),
                SCREEN_H(2048),
                1.0f
            ));
            shader->setVec3("offset", glm::vec3(SCREEN_X(512), SCREEN_Y(768 + 768/6), 0.01f));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            PRINT(occupied->message, SCREEN_X(150), SCREEN_Y(768/2), 0.08f);
        }
    }
    if (WALL_IS_KIND(occupied, TV_SCREEN) && !GameManager::showingMessage) {
        PRINT("Press E to read", SCREEN_X(1024/2 - 128), SCREEN_Y(768/2), 0.05);
    }
    else if (WALL_IS_KIND(occupied, SWITCH_OFF)) {
        PRINT("Press E to Active", SCREEN_X(1024 / 2 - 128), SCREEN_Y(768 / 2), 0.05);
    }
}

void Player::shoot() {
    bool outOfAmmo = true;
    if (health <= 0)
        return;
    if(activeWeapon == WEP_PISTOL) {
        outOfAmmo = shootPistol();
    }
    else if(activeWeapon == WEP_RIFLE){
        outOfAmmo = shootRifle();
    }
    
    if(outOfAmmo)
        return;

    auto c = cosf(CAMERA.cameraFront.z);
    auto s = sinf(CAMERA.cameraFront.y);
    auto angle = atanf(s/c);
    
    auto end = pos + (CAMERA.cameraFront * 50.0f);
    int hitType = 0; // 0 = none, 1 = wall, 2 = ent;
    
    Entity* hitEnt;
    glm::vec3 hitPos(0.0f, 0.0f, 0.0f);
        
    int x, y;
    bool pass = GameManager::instance->dda(
        pos.x,
        pos.z,
        end.x,
        end.z,
        &x, &y
    );
    if(!pass) {
        if(GameManager::instance->castRayToWall(pos, x, y, &hitPos)){
            hitType = 1;
        }
    }

    glm::vec3 entHitPos(0.0f, 0.0f, 0.0f);
    if(GameManager::instance->castRayToEntities(pos, CAMERA.cameraFront, &entHitPos, &hitEnt)){
        if(hitType != 1)
            hitType = 2;
        else if(glm::length(pos - entHitPos) < glm::length(pos - hitPos))
            hitType = 2;
    }
    
    if(hitType == 1) {
        GameManager::addEntity(new BulletHole((pos + (0.99f * (hitPos - pos)))));
    }
    else if(hitType == 2) {
        //GameManager::addEntity(new ZombieGib(entHitPos, glm::vec2(0.2, 0.2)));
        auto hitSound = rand() % 2;
        PLAY_I(SoundManager::instance->bulletHitSound[hitSound], pos);
        PLAY_I(SoundManager::instance->fleshSound, pos);
        if(activeWeapon == WEP_PISTOL){
            hitEnt->hurt(1, entHitPos);
        } else if(activeWeapon == WEP_RIFLE){
            hitEnt->hurt(4, entHitPos);
        }        
    }
}

void Player::drawPistol() {
    auto shader = SHADERS["UI"];

    glBindTexture(GL_TEXTURE_2D, pistolAmmoIndicator);
    shader->setVec3("scale", glm::vec3(
        SCREEN_W(128),
        SCREEN_H(128),
        1.0f
    ));
    shader->setVec3("offset", glm::vec3(
        SCREEN_X(1024 - 128),
        SCREEN_Y(768 - 32),
        0.0f
    ));
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader->setFloat("frame", floor(gunFrame));
    shader->setFloat("maxFrame", 5.0f);
    glBindTexture(GL_TEXTURE_2D, gun);
    shader->setVec3("scale", glm::vec3(
        SCREEN_W(512),
        SCREEN_H(1024),
        1.0f
    ));

    auto gunBob = fmin(glm::length(moveVec), 0.1f);

    shader->setVec3("offset", glm::vec3(
        cos(gunTheta) * gunBob,
        SCREEN_Y(768) - (1 + (sin(gunTheta * 2))) * gunBob,
        0.0f
    ));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    PRINT(std::to_string(ammo).c_str(), SCREEN_X(1024 - 156), SCREEN_Y(768 - 82), 0.05f);
    
    shader->use();
    glBindVertexArray(Entity::vao);
}

void Player::drawRifle() {
    auto shader = SHADERS["UI"];

    glBindTexture(GL_TEXTURE_2D, rifleAmmoIndicator);
    shader->setVec3("scale", glm::vec3(
        SCREEN_W(128),
        SCREEN_H(128),
        1.0f
    ));
    shader->setVec3("offset", glm::vec3(
        SCREEN_X(1024 - 128),
        SCREEN_Y(768 - 32),
        0.0f
    ));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    shader->setFloat("frame", floor(rifleFrame));
    shader->setFloat("maxFrame", 25.0f);
    glBindTexture(GL_TEXTURE_2D, rifle);
    shader->setVec3("scale", glm::vec3(
        SCREEN_W((int)(768.0f * 2.53125f)),
        SCREEN_H(768),
        1.0f
    ));

    auto gunBob = fmin(glm::length(moveVec), 0.1f);

    shader->setVec3("offset", glm::vec3(
        cos(gunTheta) * gunBob,
        SCREEN_Y(768) - (1 + (sin(gunTheta * 2))) * gunBob,
        0.0f
    ));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    PRINT(std::to_string(rifleAmmo).c_str(), SCREEN_X(1024 - 156), SCREEN_Y(768 - 82), 0.05f);
    shader->use();
    glBindVertexArray(Entity::vao);
}

bool Player::shootPistol() {
    if(ammo <= 0) {
        SoundManager::instance->playSound("Resources/Audio/click57.ogg", glm::vec3(0), 1.0f);
        return true;
    }
    if (gunFrame != 0 || pistolCooldown != 0)
        return true;
    ammo--;
    gunFrame = 1.0f;
    SoundManager::instance->playSound("Resources/Audio/pistol.ogg", glm::vec3(0), 1.0f);
    return false;
}

bool Player::shootRifle() {
    if(rifleAmmo <= 0) {
        SoundManager::instance->playSound("Resources/Audio/click57.ogg", glm::vec3(0), 1.0f);
        return true;
    }
    if (rifleFrame != 0 || rifleCooldown != 0)
        return true;
    rifleAmmo--;
    rifleFrame = 1.0f;
    SoundManager::instance->playSound("Resources/Audio/rifle.ogg", glm::vec3(0), 1.0f);
    return false;
}

void Player::hurt(int damage) {
    if (health > 0 && damageBoost == 0) {
        health -= damage;
        damageBoost = 50;
        if (health <= 0) {
            die();
        }
    }
}
void Player::die() {
    //pos.y = 0.1f;
}