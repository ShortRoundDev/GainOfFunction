#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#define WEP_PISTOL 1
#define WEP_RIFLE  2

#define PLAYER_ACCEL (0.01f)

class Player{
public:
    Player(glm::vec3 startPos);
    ~Player();
    
    float radius = 0.2f;
    uint8_t health = 2;
    
    glm::vec3 pos;
    glm::vec3 moveVec;
    glm::vec3 moveDir;
    float accel = 0;
    
    int collectedItems = 0;
    int killedEnemies = 0;

    float healthFrame = 0;
    GLuint greenHealth;
    GLuint yellowHealth;
    GLuint redHealth;
    
    void update(GLFWwindow* window);
    void move(GLFWwindow* window);
    
    bool hasBlueKey = false;
    bool hasRedKey = false;
    bool hasYellowKey = false;
    bool seen = false;
    
    bool noClip = false;
    
    float gunTheta = 0.0f;
    
    bool hasPistol = true;
    bool hasRifle = false;
    int activeWeapon = 0;
    
    GLuint gun;
    GLuint pistolAmmoIndicator;
    GLuint rifle;
    GLuint rifleAmmoIndicator;

    uint8_t ammo = 5;
    uint8_t rifleAmmo = 0;
    float gunFrame = 0;
    float rifleFrame = 0;
    
    GLuint popupSign;
    
    glm::vec3 pushWall(glm::vec3 newPos);
    
    void keyHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
    void collide();
    
    void draw();
    void shoot();

    void hurt(int damage);
    void die();
    int damageBoost = 0;
    int pistolCooldown = 0;
    int rifleCooldown = 0;

    float dieMessageLocation = -32.0f;
    
private:
    void drawPistol();
    void drawRifle();
    
    bool shootPistol();
    bool shootRifle();
};