#include "GameManager.hpp"
#include "GraphicsManager.hpp"

#include "Managers.hpp"

#include <iostream>
#include <filesystem>
#include <time.h>
#include <cstring>
#include <queue>
#include <cstdlib>
#include <time.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include "TileDef.h"

#include "Platform.h"
#ifdef  IS_WIN
#include "windows.h"
#endif //  IS_WIN


#define TIMESPEED 16666666

float alpha = 1.0f;

// Singleton stuff

std::unique_ptr<GameManager> GameManager::instance = nullptr;
int64_t GameManager::accumulator = 0;
std::chrono::high_resolution_clock::time_point GameManager::lastFrame = std::chrono::high_resolution_clock::now();

bool GameManager::showingMessage = false;
char* GameManager::message = NULL;

bool GameManager::keyMap[GLFW_KEY_LAST];
GameState GameManager::state = MAIN_MENU;

int GameManager::init(GLFWwindow* window, const uint16_t width, const uint16_t height) {
    GameManager::instance = std::make_unique<GameManager>(window, width, height);

    GraphicsManager::loadTex("Resources/tv.png", GL_BGRA);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    //instance->load("Resources/e1m1.bin", false);
    instance->loadAvailableSaves();

    srand(time(NULL));
    return 0;
}

void GameManager::deleteEntity(Entity* entity) {
    instance->currentLevel->removeEntities.push_back(entity);
}

void GameManager::addEntity(Entity* entity) {
    instance->currentLevel->addEntities.push_back(entity);
}

void GameManager::update() {
    auto newTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration_cast<std::chrono::nanoseconds>(newTime - lastFrame).count();
    lastFrame = newTime;

    if(frameTime >= 250000000){
        frameTime = 250000000;
    }
    accumulator += frameTime;
    while(accumulator >= 16129000) { // 16ms
        SoundManager::update();
        GameManager::instance->_update();
        accumulator -= TIMESPEED;
    }

    
}

void GameManager::draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    GameManager::instance->_draw();
    GraphicsManager::draw();
}

void GameManager::displayMessage(char* message) {
    GameManager::showingMessage = true;
    GameManager::message = message;
}

void GameManager::processInput(GLFWwindow* window) {
    GameManager::instance->_processInput(window);
}

// Instance Stuff

GameManager::GameManager(GLFWwindow* window, const uint16_t width, const uint16_t height):
    camera(width, height),
    player(glm::vec3(2.5f, 0.5, 2.5f)),
    availableSaves()
{
    accumulator = 0;
    initEventHandlers(window);
    initFont();

    Entity::init(GraphicsManager::shaders["Entities"]);
    fontShader = GraphicsManager::shaders["Font"];
 
    std::cout << "loading images" << std::endl;
    this->endSplash = GraphicsManager::loadTex("Resources/endLevelSplash.png", GL_BGRA);
    this->mainMenuTexture = GraphicsManager::loadTex("Resources/start.png", GL_BGRA);
    this->loadMenuTexture = GraphicsManager::loadTex("Resources/loadGame.png", GL_BGRA);
    this->menuCursor = GraphicsManager::loadTex("Resources/menuCursor.png", GL_BGRA);
    std::cout << "setting level aliases" << std::endl;

    mapNames["e1m1.bin"] = "Welcome to WHO";
    mapNames["e1m2.bin"] = "Lab Complex";
    mapNames["e1m3.bin"] = "Gain of Function";
    mapNames["e1m4.bin"] = "CRISPR";
    mapNames["e1m5.bin"] = "Eschatology";
    // Audio
}

GameManager::~GameManager() {

}

void GameManager::load(std::string levelName, bool loadSave, bool saving) {
    auto newLevel = new Level(levelName);
    if (newLevel->loadedSuccessfully) {
        if(currentLevel != NULL)
            delete currentLevel;
        currentLevel = newLevel;
    }
    else {
        return;
    }
    for (int i = 1; i < currentLevel->numberOfTextures + 1; i++) {
        GraphicsManager::loadTex(i, GL_BGRA);
    }
    for (int i = 0; i < 3; i++)
        GraphicsManager::loadTex(100 + i, GL_BGR);
    currentLevel->ceilingTexture = GraphicsManager::loadTex(200, GL_BGR);
    currentLevel->floorTexture = GraphicsManager::loadTex(201, GL_BGR);

    this->projection = glm::perspective(
        glm::radians(45.0f),
        (float)GraphicsManager::instance->getWidth() / (float)GraphicsManager::instance->getHeight(),
        0.1f,
        100.0f
    );
    this->camera.view = glm::translate(glm::mat4(1.0f), this->camera.cameraPos);

    player.pos = currentLevel->playerPos;
    camera.cameraFront = currentLevel->cameraFront;

    GameManager::instance->player.hasBlueKey = false;
    GameManager::instance->player.hasRedKey = false;
    GameManager::instance->player.hasYellowKey = false;
    GameManager::instance->player.colleenKillingCooldown = -1;

    if (saving)
        save();
    if (loadSave) {
        loadSaveFile(levelName);
    }
    else {
        //player.health = 6;
    }

    state = PLAYING;
}

void GameManager::loadSaveFile(std::string levelname) {
    std::string saveFileName = "Saves" + levelname.substr(levelname.find("/"), levelname.size()) + ".save";
    FILE* fp = fopen(saveFileName.c_str(), "rb");
    if (fp == NULL) {
        std::cerr << "Failed to load save file!" << std::endl;
        player.health = 6;
        return;
    }

    uint8_t playerHealth = 0;
    fread(&playerHealth, sizeof(uint8_t), 1, fp);

    uint8_t playerHasPistol = 0;
    fread(&playerHasPistol, sizeof(uint8_t), 1, fp);
    
    uint8_t pistolAmmo = 0;
    fread(&pistolAmmo, sizeof(uint8_t), 1, fp);

    uint8_t playerHasRifle = 0;
    fread(&playerHasRifle, sizeof(uint8_t), 1, fp);

    uint8_t rifleAmmo = 0;
    fread(&rifleAmmo, sizeof(uint8_t), 1, fp);
    
    player.health = playerHealth;
    
    player.hasPistol = playerHasPistol == 1;
    player.ammo = pistolAmmo;

    player.hasRifle = playerHasRifle == 1;
    player.rifleAmmo = rifleAmmo;
    fclose(fp);
}


void GameManager::_update() {
    switch (state) {
    case MAIN_MENU:
        updateMainMenu();
        break;
    case LOAD_SCREEN:
        updateLoadMenu();
        break;
    case PLAYING:
        updateGame();
        break;
    }
}

void GameManager::updateMainMenu() {

}

void GameManager::updateLoadMenu() {

}

void GameManager::updateGame() {
    if (showingMessage)
        return;
    if (levelChanging) {
        if (slideSplash > -1.0f) {
            slideSplash -= 0.02f;
            if (slideSplash < -1.0f) {
                slideSplash = -1.0f;
            }
        }
        else if (showDead > 0) {
            showDead--;
            if (showDead <= 0) {
                SoundManager::instance->playSound("Resources/Audio/pistol.ogg", glm::vec3(0.0f), 1.0f);
            }
        }
        else if (showItems > 0) {
            showItems--;
            if (showItems <= 0) {
                SoundManager::instance->playSound("Resources/Audio/pistol.ogg", glm::vec3(0.0f), 1.0f);
            }
        }
        return;
    }
    camera.update();
    player.update(GraphicsManager::instance->window);
    currentLevel->update();

}

void GameManager::_draw() {
    switch (state) {
    case MAIN_MENU:
        drawMainMenu();
        break;
    case LOAD_SCREEN:
        drawLoadMenu();
        break;
    case PLAYING:
        drawGame();
        break;
    }

    if (GameManager::showingMessage && GameManager::message != NULL) {
        drawUi(GraphicsManager::findTex("Resources/tv.png"), SCREEN_X(512), SCREEN_Y(1024), SCREEN_W(2048), SCREEN_H(2048));
        glClear(GL_DEPTH_BUFFER_BIT);
        print(message, SCREEN_X(140), SCREEN_Y(212), 0.06);
        print("Press Q to close", SCREEN_X(1024 - 512), SCREEN_Y(768 - 32), 0.07);
    }
}

void GameManager::drawGame() {
    currentLevel->draw();
    auto playerStr = std::string("P ") + std::to_string((int)player.pos.x) + " " + std::to_string((int)player.pos.z);
    glClear(GL_DEPTH_BUFFER_BIT);

    player.draw();

    if (levelChanging) {
        glClear(GL_DEPTH_BUFFER_BIT);
        GameManager::drawUi(endSplash, 0.0f, slideSplash, 2.0f, 2.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        if (showDead <= 0) {
            print((std::string("KILLED ") + std::to_string(player.killedEnemies + 0)).c_str(), SCREEN_X(600), SCREEN_Y(768 / 2), 0.05f);
            print((std::string("OF  ") + std::to_string(currentLevel->totalEnemies)).c_str(), SCREEN_X(854), SCREEN_Y(768 / 2), 0.05f);
        }
        if (showItems <= 0) {
            print((std::string("ITEMS  ") + std::to_string(player.collectedItems + 0)).c_str(), SCREEN_X(600), SCREEN_Y(768 / 2 + 64), 0.05f);
            print((std::string("OF  ") + std::to_string(currentLevel->totalItems)).c_str(), SCREEN_X(854), SCREEN_Y(768 / 2 + 64), 0.05f);
        }
        if (showDead <= 0 && showItems <= 0) {
            print("PRESS SPACE TO CONTINUE", SCREEN_X(128), SCREEN_Y(768 - 64), 0.08f);
        }
    }

    //print(playerStr.c_str(), SCREEN_X(64), SCREEN_Y(64), 0.05f);
}

void GameManager::drawMainMenu() {
    GameManager::drawUi(mainMenuTexture, 0.0f, -1.0f, 2.0f, 2.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    GameManager::drawUi(menuCursor, SCREEN_X(424), SCREEN_Y(512 + (mainMenuOption * 108)), SCREEN_W(256), SCREEN_H(256));
}

void GameManager::drawLoadMenu() {
    GameManager::drawUi(loadMenuTexture, 0.0f, -1.0f, 2.0f, 2.0f);
    int i = 0;
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto available : availableSaves) {
        auto mapName = available.find(".");
        print(mapNames[available].c_str(), SCREEN_X(624), SCREEN_Y(232 + (i + 1) * 80), 0.07);
        //print(available.substr(0, mapName).c_str(), SCREEN_X(624), SCREEN_Y(232 + (i + 1) * 80), 0.1);
        i++;
    }
    print("GO BACK", SCREEN_X(624), SCREEN_Y(232), 0.1);
    GameManager::drawUi(menuCursor, SCREEN_X(524), SCREEN_Y(316 + (loadMenuOption * 80)), SCREEN_W(256), SCREEN_H(256));

}

void GameManager::print(const char* message, float xPos, float yPos, float size) {
    fontShader->use();
    glBindVertexArray(fontVao);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    
    float x = 0;
    float y = 0;
    for(uint64_t i = 0; i < strlen(message); i++) {
        char c = message[i];
        if(c >= 'a' && c <= 'z'){
            c -= 32;
        }
        auto cDiff = c - 'A';
        x += size * 0.65;
        if(!(cDiff >= 0 && cDiff <= 25)){
            cDiff = c - '0';
            if(cDiff >= 0 && cDiff <= 9){
                cDiff += 26;
            }
            else if (c == '.') {
                cDiff = 36;
            }
            else {
                if(c == '\n') {
                    x = 0;
                    y -= size;
                }
                continue;
            }
        }
        fontShader->setInt("c", cDiff);
        fontShader->setVec3("trans", glm::vec3(xPos + x, yPos + y, size));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void GameManager::_processInput(GLFWwindow* window){
}

void GameManager::initEventHandlers(GLFWwindow* window) {
    /*if(glfwRawMouseMotionSupported()){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }*/
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        GameManager::keyMap[key] = true;
    else if (action == GLFW_RELEASE)
        GameManager::keyMap[key] = false;

    switch (GameManager::instance->state){
    case MAIN_MENU:
        if (action == GLFW_PRESS) {
            switch (key) {
            case GLFW_KEY_UP:
                GameManager::instance->mainMenuOption--;
                if (GameManager::instance->mainMenuOption < 0)
                    GameManager::instance->mainMenuOption = 2;
                PLAY_S("Resources/Audio/pickupRifle.ogg", glm::vec3(0));
                break;
            case GLFW_KEY_DOWN:
                GameManager::instance->mainMenuOption++;
                if (GameManager::instance->mainMenuOption > 2)
                    GameManager::instance->mainMenuOption = 0;
                PLAY_S("Resources/Audio/pickupRifle.ogg", glm::vec3(0));
                break;
            case GLFW_KEY_ENTER:
                PLAY_S("Resources/Audio/pistol.ogg", glm::vec3(0));
                switch(GameManager::instance->mainMenuOption){
                case 0:
                    GameManager::instance->player.health = 6;
                    GameManager::instance->player.activeWeapon = 0;
                    GameManager::instance->player.hasPistol = false;
                    GameManager::instance->player.hasRifle = false;
                    GameManager::instance->player.hasBlueKey = false;
                    GameManager::instance->player.hasRedKey = false;
                    GameManager::instance->player.hasYellowKey = false;
                    GameManager::instance->player.ammo = 0;
                    GameManager::instance->player.rifleAmmo = 0;
                    GameManager::instance->load("Resources/e1m1.bin", false, false);
                    break;
                case 1: // Load
                    GameManager::instance->state = LOAD_SCREEN;
                    break;
                case 2: // Quit
                    glfwSetWindowShouldClose(window, 1);
                    break;
                }
                break;
            }
        }
        break;
    case LOAD_SCREEN:
        if (action == GLFW_PRESS) {

            switch (key) {
            case GLFW_KEY_UP:
                GameManager::instance->loadMenuOption--;
                if (GameManager::instance->loadMenuOption < 0)
                    GameManager::instance->loadMenuOption = GameManager::instance->availableSaves.size();
                PLAY_S("Resources/Audio/pickupRifle.ogg", glm::vec3(0));
                break;
            case GLFW_KEY_DOWN:
                GameManager::instance->loadMenuOption++;
                if (GameManager::instance->loadMenuOption > GameManager::instance->availableSaves.size())
                    GameManager::instance->loadMenuOption = 0;
                PLAY_S("Resources/Audio/pickupRifle.ogg", glm::vec3(0));
                break;
            case GLFW_KEY_ENTER:
                PLAY_S("Resources/Audio/pistol.ogg", glm::vec3(0));
                if (GameManager::instance->loadMenuOption == 0) {
                    GameManager::instance->mainMenuOption = 0;
                    GameManager::instance->loadMenuOption = 0;
                    GameManager::instance->state = MAIN_MENU;
                }
                else {
                    std::string loadFileName = GameManager::instance->availableSaves[((uint64_t)GameManager::instance->loadMenuOption) - 1];
                    loadFileName = "Resources/" + loadFileName.substr(0, loadFileName.find(".save"));
                    GameManager::instance->load(loadFileName, true, false);
                }
                break;
                return;
            }
        }
        break;
    }

    if (GameManager::instance->levelChanging && key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        GameManager::instance->slideSplash = 1.0f;
        GameManager::instance->showDead = 10;
        GameManager::instance->showItems = 10;
        GameManager::instance->levelChanging = false;
        GameManager::instance->load("Resources/" + std::string(GameManager::instance->nextLevel), false, true);
    }

    if (GameManager::showingMessage && key == GLFW_KEY_Q && action == GLFW_PRESS) {
        GameManager::showingMessage = false;
        GameManager::message = NULL;
    }

    if(key == GLFW_KEY_M && action == GLFW_PRESS) {
        if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED){
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }
    /*if(key == GLFW_KEY_1 && action == GLFW_PRESS) {
        (&CAMERA)->cameraFront = glm::vec3(0.0f, 0.0f, 1.0f); // Looking down
    }
    if(key == GLFW_KEY_2 && action == GLFW_PRESS) {
        (&CAMERA)->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Looking up
    }
    if(key == GLFW_KEY_3 && action == GLFW_PRESS) {
        (&CAMERA)->cameraFront = glm::vec3(1.0f, 0.0f, 0.0f); // Looking to the right
    }
    if(key == GLFW_KEY_4 && action == GLFW_PRESS) {
        (&CAMERA)->cameraFront = glm::vec3(-1.0f, 0.0f, 0.0f); // Looking to the left
    }*/
    if(key == GLFW_KEY_B && action == GLFW_PRESS){
        GameManager::instance->bright ^= true;
    }
    if(key == GLFW_KEY_T && action == GLFW_PRESS) {
        if(GameManager::instance->wireframe){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        GameManager::instance->wireframe ^= true;
    }
    if(!GameManager::instance->levelChanging)
        GameManager::instance->player.keyHandler(window, key, scancode, action, mods);
}

void GameManager::initFont() {
    fontTex = GraphicsManager::loadTex(2000, GL_BGRA);
    
    float square[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,       0.0f, // Top left
         0.5f, -0.5f, 0.0f, 1.0f/37.0f, 0.0f, // Top Right
        -0.5f,  0.5f, 0.0f, 0.0f,       1.0f,
        
         0.5f, -0.5f, 0.0f, 1.0f/37.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f/37.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f,       1.0f
    };
    
    unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), square, GL_STATIC_DRAW);
    
    // Vertex Data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // UV Mapping
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    this->fontVao = vao;
}


bool GameManager::dda(float startX, float startY, float endX, float endY, int* x, int* y){
    *x = -1;
    *y = -1;
    
    Wall* startWall = TRY_WALL((int)startX, ((int)startY));

    if(startWall == NULL || SOLID(*startWall)){
        return false;
    }
    
    float x1 = startX;
    float y1 = startY;
    
    auto x2 = endX;
    auto y2 = endY;
    
    int gridPosX = (int)x1;
    int gridPosY = (int)y1;
     
    float dirX = x2 - x1;
    float dirY = y2 - y1;
    float distSqr = dirX * dirX + dirY * dirY;
    if (distSqr < 0.00000001)
        return false;
     
    float nf = 1 / sqrt(distSqr);
    dirX *= nf;
    dirY *= nf;
     
    float deltaX = 1.0f / std::fabs(dirX);
    float deltaY = 1.0f / std::fabs(dirY);
     
    float maxX = gridPosX - x1;
    float maxY = gridPosY - y1;
    if (dirX >= 0) maxX += 1.0f;
    if (dirY >= 0) maxY += 1.0f;
    maxX /= dirX;
    maxY /= dirY;
     
    int stepX = dirX < 0 ? -1 : 1;
    int stepY = dirY < 0 ? -1 : 1;
    int gridGoalX = (int)x2;
    int gridGoalY = (int)y2;
    int currentDirX = gridGoalX - gridPosX;
    int currentDirY = gridGoalY - gridPosY;
        
    while ((currentDirX * stepX > 0 || currentDirY * stepY > 0) && gridPosX >= 0 && gridPosY >= 0)
    {
        if (maxX < maxY)
        {
            maxX += deltaX;
            gridPosX += stepX;
            currentDirX = gridGoalX - gridPosX;
        }
        else
        {
            maxY += deltaY;
            gridPosY += stepY;
            currentDirY = gridGoalY - gridPosY;
        }
                      
        Wall* wall = TRY_WALL(gridPosX, gridPosY);
        if (wall == NULL)
            break;
        if(COLLIDEABLE_ALL(wall)) {
            *x = gridPosX;
            *y = gridPosY;
            return false;
        }
    }
    *x = (int)endY;
    *y = (int)endX;
    return true;
}

bool GameManager::castRayToWall(glm::vec3 start, int wallX, int wallY, glm::vec3* out) {
    float fWallX = (float)wallX,
          fWallY = (float)wallY;
    
    const float X = (float)wallX,
                B = 0.0f,
                Y = (float)wallY,
                
                W = X + 1.0f,
                H =     1.0f,
                D = Y + 1.0f;
    
    /*float cube[] = {
        // Right Face
        W, H, D,
        W, B, Y,
        W, H, Y,
        
        W, H, D,
        W, B, D,
        W, B, Y,
        
        // Front Face
        X, H, Y,
        W, H, Y,
        W, B, Y,
        
        X, H, Y,
        W, B, Y,
        X, B, Y,
        
        // Left Face
        X, H, Y,
        X, B, D,
        X, H, D,
        
        X, H, Y,
        X, B, Y,
        X, B, D,
        
        // Back Face
        X, H, D,
        X, B, D,
        W, B, D,
        
        X, H, D,
        W, B, D,
        W, H, D
    };*/
    
    glm::vec3 cube[] = {
        glm::vec3(W, H, D), glm::vec3( 1.0f, 0.0f,  0.0f), glm::vec3(W, B, Y), // right  side, top left corner  | i % 2 -> 0
        glm::vec3(W, H, Y), glm::vec3( 0.0f, 0.0f, -1.0f), glm::vec3(X, B, Y), // front  face, top left corner  | i % 2 -> 1
        glm::vec3(X, H, D), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(X, B, Y), // left   side, top right corner | i % 2 -> 0
        glm::vec3(W, H, D), glm::vec3( 0.0f, 0.0f,  1.0f), glm::vec3(X, B, D)  // bottom face, top right corner | i % 2 -> 1
    };
    bool hit = false;
    float finalDist = 999999.9f;
    int _i = 0;
    for(size_t i = 0; i < sizeof(cube)/sizeof(glm::vec3); i += 3){
        auto plane = cube[i];
        float dist = 0.0f;
        if(glm::intersectRayPlane(
            start,
            CAMERA.cameraFront,
            plane,
            cube[i + 1],
            dist
        )){
            dist = fabs(dist);
            auto worldPos = start + (CAMERA.cameraFront * dist);
            auto bounds = cube[i + 2];
            if(i % 2 == 0) { // check left and right, z and y
                if(!(
                    worldPos.z >= bounds.z && worldPos.z <= plane.z &&
                    worldPos.y >= bounds.y && worldPos.y <= plane.y
                )){
                    continue;
                }
            } else { // check north and south, x and y
                if(!(
                    worldPos.x >= bounds.x && worldPos.x <= plane.x &&
                    worldPos.y >= bounds.y && worldPos.y <= plane.y
                )){
                    continue;
                }
            }
            hit = true;
            if(finalDist > dist && dist != -INFINITY){
                finalDist = dist;
                _i = (int)i;
            }
        }
    }
    *out = start + (CAMERA.cameraFront * finalDist);
    return hit;
}

bool GameManager::castRayToEntities(glm::vec3 start, glm::vec3 dir, glm::vec3* worldPos, Entity** e) {
    float shortestDist = INFINITY;
    glm::vec3 closestPos(0.0f, 0.0f, 0.0f);
    Entity* closestEnt = nullptr;
    bool hit = false;
    for(auto e : currentLevel->entities){
        if(!e->shootable || e->dead)
            continue;
        glm::vec3 hitPos;
        if(castRayToSphere(
            start,
            dir,
            e->position + glm::vec3(0, 0.5, 0),
            glm::vec2(e->radiusX, e->radiusY),
            &hitPos
        )){
            if(glm::length(closestPos - start) > glm::length(hitPos - start)){
                closestPos = hitPos;
                closestEnt = e;
                hit = true;
            }
        }
    }
    if(hit){
        *worldPos = closestPos;
        *e = closestEnt;
    }
    return hit;
}

bool GameManager::castRayToSphere(glm::vec3 start, glm::vec3 dir, glm::vec3 sphereStart, glm::vec2 radius, glm::vec3* hit) {
    auto affine = glm::vec3(radius.x, radius.y, radius.x);
    auto _start = start / affine;
    auto _sphereStart = sphereStart / affine;
    auto _dir = glm::normalize(dir /affine);
    glm::vec3 m = _start - _sphereStart;
    
    float b = glm::dot(m, _dir);
    float c = glm::dot(m, m) - 1.0f;
    if(c > 0.0f && b > 0.0f)
        return false;
    float discr = b * b - c;
    if(discr < 0.0f) {
        return false;
    }
    float t = -b - sqrtf(discr);
    if(t < 0.0f)
        t = 0.0f;
    *hit = (_start + t * _dir) * affine;
    return true;
}

bool GameManager::bfs(float startX, float startY, float endX, float endY, std::map<uint32_t, uint32_t> &cameFrom)
{
    int16_t x1 = (int16_t)startX,
            y1 = (int16_t)startY,
            x2 = (int16_t)endX,
            y2 = (int16_t)endY;
    
    std::queue<uint32_t> frontier;
    
    auto end = PACK_COORDS(x2, y2);
    auto start = PACK_COORDS(x1, y1);
    cameFrom[start] = PACK_COORDS(-1, -1);
    
    frontier.push(start);
    
    bool foundPath = false;
    
    while(!frontier.empty()){
        uint32_t current = frontier.front();
        frontier.pop();
        
        int16_t cX = UNPACK_X(current),
                cY = UNPACK_Y(current);

        if(current == end) {
            foundPath = true;
            break;
        }
        
        //top
        int16_t neighbors[] = {
            cX, cY - 1, // top
            cX + 1, cY, // right
            cX, cY + 1, // bottom
            cX - 1, cY  // left
        };
        
        for(size_t i = 0; i < sizeof(neighbors)/sizeof(int16_t); i += 2) {
            auto x = neighbors[i],
                 y = neighbors[i + 1];
            
            auto next = PACK_COORDS(x, y);
            bool notVisited = cameFrom.find(next) == cameFrom.end();
            if(notVisited && IN_BOUNDS(x, y) && NOT_SOLID(WALLS[COORDS(x, y)])){
                frontier.push(next);
                cameFrom[next] = current;
            }
        }
    }
    
    return foundPath;
}

void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {
    
    auto width = GraphicsManager::instance->getWidth();
    auto height = GraphicsManager::instance->getHeight();
    
    Camera* camera = &(GameManager::instance->camera);
    
    float sensitivity = 0.1f;
    float x = ((float)xPos - width/2) * sensitivity;
    float y = ((float)yPos - height/2) * sensitivity;
    
    camera->yaw += x;
    camera->pitch -= y;
    
    if(camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if(camera->pitch < -89.0f)
        camera->pitch = -89.0f;
        
    glm::vec3 front;
    front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    front.y = sin(glm::radians(camera->pitch));
    front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
    camera->cameraFront = glm::normalize(front);
    if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        glfwSetCursorPos(window, width/2, height/2);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (GameManager::instance->levelChanging)
        return;
    if(button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        (&PLAYER)->shoot();
    }
}

void GameManager::printPath(std::map<uint32_t, uint32_t> &path, uint32_t start, uint32_t end) {
    auto node = start;
    std::cout << "==========" << std::endl;
    int x = UNPACK_X(node),
        y = UNPACK_Y(node);
    while(node != end && node != 0) {
        std::cout << x << ", " << y << std::endl;
        node = path[node];
        x = UNPACK_X(node);
        y = UNPACK_Y(node);
    }
    std::cout << x << ", " << y << std::endl;
}

void GameManager::drawUi(GLuint texture, float x, float y, float w, float h) {
    drawAnimatedUi(texture, x, y, w, h, 0.0f, 1.0f);
}

void GameManager::drawAnimatedUi(GLuint texture, float x, float y, float w, float h, float frame, float maxFrame) {
    Shader* shader = GraphicsManager::shaders["UI"];
    shader->use();
    glBindVertexArray(Entity::vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader->setFloat("frame", frame);
    shader->setFloat("maxFrame", maxFrame);
    shader->setVec3("offset", glm::vec3(x, y, 0.0f));
    shader->setVec3("scale", glm::vec3(w, h, 0.0f));
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GameManager::save() {
#ifdef IS_WIN
    saveWin();
#else
    saveLinux();
#endif
}

#ifdef IS_WIN
void GameManager::saveWin() {
    DWORD attribs = ::GetFileAttributesW(L"Saves");
    if (attribs == INVALID_FILE_ATTRIBUTES) { // file not found, create
        auto success = ::CreateDirectoryW(L"Saves", NULL);
        if (!success) {
            return;
        }
    }

    FILETIME time;
    ::GetSystemTimeAsFileTime(&time);

    std::string saveFile = ".\\Saves\\" + instance->currentLevel->levelName + ".save";

    FILE* fp = fopen(saveFile.c_str(), "wb");
    auto playerHealth = instance->player.health;
    
    auto playerHasPistol = instance->player.hasPistol;
    auto playerPistolAmmo = instance->player.ammo;

    auto playerHasRifle = instance->player.hasRifle;
    auto playerRifleAmmo = instance->player.rifleAmmo;

    uint8_t saveData[] = {
        playerHealth,
        playerHasPistol ? 1 : 0,
        playerPistolAmmo,
        playerHasRifle ? 1 : 0,
        playerRifleAmmo
    };

    fwrite(saveData, sizeof(uint8_t), sizeof(saveData)/sizeof(uint8_t), fp);
    fflush(fp);
    fclose(fp);
};
#else
void GameManager::saveLinux() {

}
#endif

void GameManager::loadAvailableSaves() {
    for (const auto &iter : std::filesystem::directory_iterator("Saves")) {
        std::string saveFileName = iter.path().filename().string();
        auto middle = saveFileName.find(".save");
        this->availableSaves.push_back(saveFileName.substr(0, middle));
    }

#ifdef IS_WIN
    //loadAvailableSavesWin();
#else
    loadAvailableSavesLinux();
#endif
}

#ifdef IS_WIN
void GameManager::loadAvailableSavesWin() {

}
#else
void GameManager::loadAvailableSavesLinux() {

}
#endif