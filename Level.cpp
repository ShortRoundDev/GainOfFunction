#include "Level.hpp"
#include "Level.hpp"

#include <cstdlib>
#include <string.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "glm/glm.hpp"
#include "GraphicsManager.hpp"
#include "GameManager.hpp"

#include "EntDef.h"
#include "TileDef.h"
#include "Managers.hpp"

#define TILE_TOP_LEFT       0.0f, 0.0f
#define TILE_TOP_RIGHT      1.0f, 0.0f
#define TILE_BOTTOM_RIGHT   1.0f, 1.0f
#define TILE_BOTTOM_LEFT    0.0f, 1.0f

//25px
#define DOOR_MAX_Y          0.78125f

#define DOOR_TOP_RIGHT      1.0f, DOOR_MAX_Y
#define DOOR_BOTTOM_RIGHT   1.0f, 1.0f
#define DOOR_BOTTOM_LEFT    0.0f, 1.0f
#define DOOR_TOP_LEFT       0.0f, DOOR_MAX_Y

#define DRAW_WIDTH          10
#define DRAW_HEIGHT         10

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))

#define COORD(x, y) (x + (y * width))

Level::Level(std::string path){
    std::cout << "New Level" << std::endl;
    fileBuffer = loadFile(path);
    if (fileBuffer == NULL) {
        loadedSuccessfully = false;
        return;
    }
    levelName = std::filesystem::path(path).filename().string();
    this->numberOfTextures = ((uint16_t*)fileBuffer)[0];
    this->width = fileBuffer[2];
    this->height = fileBuffer[4];
    this->walls = (Wall*) calloc(
        (uint64_t)width * (uint64_t)height,
        sizeof(Wall)
    );
    this->entities = std::vector<Entity*>();
    this->removeEntities = std::vector<Entity*>();
    
    auto playerX = *((uint16_t*)(fileBuffer + 6));
    auto playerY = *((uint16_t*)(fileBuffer + 8));
    
    playerPos = glm::vec3(
        ((float)playerX),
        0.5f,
        ((float)playerY)
    );
    uint16_t lookDir = *((uint16_t*)(fileBuffer + 10));
    switch(lookDir){
        case 0: {
            cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
            break;
        }
        case 1: {
            cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        }
        case 2: {
            cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
        }
        case 3: {
            cameraFront = glm::vec3(-1.0f, 0.0f, 0.0f);
            break;
        }
    }
    
    this->wallsLocation = fileBuffer + 12;
    this->entitiesLocation = wallsLocation + (sizeof(uint64_t) * 2 * width * height);
    auto eDiff = entitiesLocation - fileBuffer;
    
    std::cout << " loading walls...";
    loadWalls();
    std::cout << " loading entities...";
    loadEntities();
    std::cout << " uploading wall data to GPU...";
    uploadWall();
    std::cout << " uploading floor data to GPU...";
    uploadFloor();
    std::cout << " uploading ceiling data to GPU...";
    uploadCeiling();
    
    wallShader = GraphicsManager::shaders["Walls"];
    std::cout << " freeing level buffer data...";
    free(fileBuffer);
    loadedSuccessfully = true;

    auto loopBuffer = SoundManager::instance->sounds.find("Resources/Audio/Basement.ogg");
    if (loopBuffer != SoundManager::instance->sounds.end()) {
        SoundManager::instance->playLoop(loopBuffer->second, glm::vec3(0), 3.0f);
    }

}

Level::~Level() {

    auto loopSource = SoundManager::instance->sounds.find("Resources/Audio/Basement.ogg");
    if (this->ambience != 0) {
        alSourceStop(ambience);
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            Wall wall = walls[COORDS(i, j)];
            if (wall.message != NULL) {
                free(wall.message);
                wall.message = NULL;
            }
        }
    }
    for (Entity* e : entities) {
        delete e;
    }
    free(walls);
}


uint8_t* Level::loadFile(std::string path){

#ifdef IS_WIN
    FILE* fp;
    fopen_s(&fp, path.c_str(), "rb");
#else
    FILE* fp = fopen(path.c_str(), "rb");
#endif
    if(fp == NULL) {

#ifdef IS_WIN
        char errmsg[1024];
        strerror_s(errmsg, errno);
        std::cerr << "Failed to open map! " << errmsg << std::endl;
#else
        std::cerr << "Failed to open map! " << strerror(errno) << std::endl;
#endif
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size_t t = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    uint8_t* buffer = (uint8_t*)calloc(t, sizeof(uint8_t));
    if(buffer != NULL)
        fread(buffer, t, sizeof(uint8_t), fp);
    fclose(fp);
    return buffer;
}

void Level::draw() {
    wallShader->use();
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    Camera* camera = &(GameManager::instance->camera);
    wallShader->setMat4("projection", GameManager::instance->projection);
    wallShader->setVec3("playerPos", camera->cameraPos);
    wallShader->setMat4("view", camera->view);
    
    wallShader->setVec3("scale", glm::vec3(1.0, 1.0, 1.0));
    wallShader->setVec3("offset", glm::vec3(0, 0, 0));

    if(GameManager::instance->bright){
        wallShader->setFloat("minBright", 0.9f);
        wallShader->setFloat("maxBright", GameManager::maxBright);
    } else {
        wallShader->setFloat("minBright", 0.0f);
        wallShader->setFloat("maxBright", GameManager::maxBright);
    }

    for(int i = MAX(0, (int)(PLAYER.pos.z - DRAW_HEIGHT)); i < MIN(height, (int)(PLAYER.pos.z + DRAW_HEIGHT)); i++) {
        for(int j = MAX(0, (int)(PLAYER.pos.x - DRAW_WIDTH)); j < MIN(width, (int)(PLAYER.pos.x + DRAW_WIDTH)); j++) {
            auto tile = COORD(j, i);
            auto w = walls + tile;
            auto tileNum = walls[tile].wallTexture;
            
            auto offset = glm::vec3((float)j, 0.0f, (float)i);
            
            wallShader->setVec4("tint", w->tint);
            wallShader->setVec3("offset", offset);
            wallShader->setVec3("scale", glm::vec3(1.0f, 1.0f, 1.0f));
            
            auto _floorTexture = GraphicsManager::instance->findTex(walls[tile].floorTexture);
            auto _ceilingTexture = GraphicsManager::instance->findTex(walls[tile].ceilingTexture);
            
            //floor
            glBindVertexArray(floorVao);
            glBindTexture(GL_TEXTURE_2D, _floorTexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
                
            //ceiling
            glBindVertexArray(ceilingVao);
            glBindTexture(GL_TEXTURE_2D, _ceilingTexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            

            auto texture = GraphicsManager::findTex(tileNum);
            if(tileNum == BLANK_WALL || tileNum == NO_DRAW)
                goto reset;
                
            offset += glm::vec3(0, walls[tile].displacement.y, 0);

            glBindVertexArray(wallsVao);
            wallShader->setVec4("tint", w->tint);
            glBindTexture(GL_TEXTURE_2D, texture);

            if(TILE_IS_DOOR(tileNum)) {
                offset = drawDoor(j, i, offset);
            } else if(tileNum == WALL_SIGN){
                offset = drawSign(j, i, offset);
            } else if (tileNum == TV_SCREEN || tileNum == SWITCH_ON || tileNum == SWITCH_OFF) {
                offset = drawTv(j, i, offset);
            }
            else {
                wallShader->setVec3("scale", glm::vec3(1.0f, 1.0f, 1.0f));
            }
            
            wallShader->setVec3("offset", offset);
            glDrawArrays(GL_TRIANGLES, 0, 30);
        reset:
            w->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
    
    Entity::shader->use();
    Entity::shader->setVec3("playerPos", camera->cameraPos);
    Entity::shader->setVec4("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    Entity::shader->setMat4("view", camera->view);
    Entity::shader->setMat4("projection", GameManager::instance->projection);
    if(GameManager::instance->bright){
        Entity::shader->setFloat("minBright", 0.9f);
        wallShader->setFloat("maxBright", GameManager::maxBright);
    } else {
        Entity::shader->setFloat("minBright", 0.0f);
        wallShader->setFloat("maxBright", GameManager::maxBright);
    }
    for(auto e : entities){
        if(DIST_2(e->position, PLAYER.pos) < 10)
            e->draw();
    }
    if (switchNotification > 0 && switchesOff != -1) {
        PRINT((std::to_string(switchesOff) + " SWITCHES LEFT").c_str(), SCREEN_X(512 - 164), SCREEN_Y(64), 0.06f);
        switchNotification--;
    }
    glClear(GL_DEPTH_BUFFER_BIT);
    if (colleen != NULL && colleen->shootable) {
        float healthPercent = ((float)colleen->health) / 40.0f;
        GameManager::drawUi(
            ((Colleen*)colleen)->healthbarTex,
            0.0f, SCREEN_Y(128),
            SCREEN_W((int)(((float)((Colleen*)colleen)->healthbarWidth) * healthPercent)), SCREEN_H(32)
        );
    }

}

glm::vec3 Level::drawDoor(int x, int y, glm::vec3 offset) {
    auto left = COORD(x - 1, y);
    auto right = COORD(x + 1, y);

    if (left >= 0 && right < width * height && (walls[left].wallTexture != 0 && walls[right].wallTexture != 0) && !IS_DOOR(walls[left]) && !IS_DOOR(walls[right])) {
        wallShader->setVec3("scale", glm::vec3(1.0f, 1.0f, 0.5f));
        offset.z += 0.25f;
    }
    else {
        wallShader->setVec3("scale", glm::vec3(0.5f, 1.0f, 1.0f));
        offset.x += 0.25f;
    }
    return offset;
}

glm::vec3 Level::drawRegular(int x, int y, glm::vec3 offset) {
    return glm::vec3(0);
}

glm::vec3 Level::drawSign(int x, int y, glm::vec3 offset) {
    Wall* left  = TRY_WALL(x - 1, y);
    Wall* right = TRY_WALL(x + 1, y);
    Wall* up    = TRY_WALL(x, y - 1);
    Wall* down  = TRY_WALL(x, y + 1);

    if (left != NULL && SOLID(*left) && IN_BOUNDS(x - 1, y) && left->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(0.05f, 0.34375f, 0.8125f));
        offset.x -= 0.048f;
        offset.z += 0.1875 / 2.0f;
        offset.y += 0.35f;
    }
    else if (right != NULL && SOLID(*right) && IN_BOUNDS(x + 1, y) && right->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(0.05f, 0.34375f, 0.8125f));
        offset.x += 0.999f;
        offset.z += 0.1875 / 2.0f;
        offset.y += 0.35f;
    }
    else if (up != NULL && SOLID(*up) && IN_BOUNDS(x, y - 1) && up->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(0.8125f, 0.34375f, 0.05f));
        offset.z -= 0.048f;
        offset.y += 0.35f;
        offset.x += 0.1875 / 2.0f;
    }
    else if (down != NULL && SOLID(*down) && IN_BOUNDS(x, y + 1) && down->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(0.8125f, 0.34375f, 0.05f));
        offset.z += 0.999f;
        offset.y += 0.35f;
        offset.x += 0.1875 / 2.0f;
    }
    return offset;
}

glm::vec3 Level::drawTv(int x, int y, glm::vec3 offset) {
    Wall* left = TRY_WALL(x - 1, y);
    Wall* right = TRY_WALL(x + 1, y);
    Wall* up = TRY_WALL(x, y - 1);
    Wall* down = TRY_WALL(x, y + 1);

    if (left != NULL && SOLID(*left) && IN_BOUNDS(x - 1, y) && left->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(0.05f, 1.0f, 1.0f));
        offset.x -= 0.048f;
        offset.z += 0.1875 / 2.0f;
    }
    else if (right != NULL && SOLID(*right) && IN_BOUNDS(x + 1, y) && right->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(0.05f, 1.0f, 1.0f));
        offset.x += 0.999f;
        offset.z += 0.1875 / 2.0f;
    }
    else if (up != NULL && SOLID(*up) && IN_BOUNDS(x, y - 1) && up->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(1.0f, 1.0f, 0.05f));
        offset.z -= 0.048f;
        offset.x += 0.1875 / 2.0f;
    }
    else if (down != NULL && SOLID(*down) && IN_BOUNDS(x, y + 1) && down->wallTexture != 3) {
        wallShader->setVec3("scale", glm::vec3(1.0f, 1.0f, 0.05f));
        offset.z += 0.999f;
        offset.x += 0.1875 / 2.0f;
    }
    return offset;
}


void Level::update() {
    if(!GameManager::instance->levelChanging) {
        for(const auto &e : entities) {
            e->update();
        }
    }
    
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            auto wall = walls[COORD(j, i)];
            walls[COORD(j, i)].update();
        }
    }    
    for(auto e : removeEntities) {
        auto idx = std::find(entities.begin(), entities.end(), e);
        entities.erase(idx);
        delete e;
    }
    removeEntities.clear();
    
    for(auto e : addEntities) {
        entities.push_back(e);
    }
    addEntities.clear();
}

void Level::uploadWall(){
    const float X = 0.0f,
                Y = 0.0f,
                B = 0.0f,
                W = 1.0f,
                H = 1.0f,
                D = 1.0f;
    
    float cube[] = {
        // Right Face
        W, H, D, TILE_TOP_LEFT,
        W, H, Y, TILE_TOP_RIGHT,
        W, B, Y, TILE_BOTTOM_RIGHT,
        
        W, H, D, TILE_TOP_LEFT,
        W, B, D, TILE_BOTTOM_LEFT,
        W, B, Y, TILE_BOTTOM_RIGHT,
        
        // Front Face
        X, H, Y, TILE_TOP_RIGHT,
        W, H, Y, TILE_TOP_LEFT,
        W, B, Y, TILE_BOTTOM_LEFT,
        
        X, H, Y, TILE_TOP_RIGHT,
        W, B, Y, TILE_BOTTOM_LEFT,
        X, B, Y, TILE_BOTTOM_RIGHT,
        
        // Left Face
        X, H, Y, TILE_TOP_LEFT,
        X, B, D, TILE_BOTTOM_RIGHT,
        X, H, D, TILE_TOP_RIGHT,
        
        X, H, Y, TILE_TOP_LEFT,
        X, B, Y, TILE_BOTTOM_LEFT,
        X, B, D, TILE_BOTTOM_RIGHT,
        
        // Back Face
        X, H, D, TILE_TOP_LEFT,
        X, B, D, TILE_BOTTOM_LEFT,
        W, B, D, TILE_BOTTOM_RIGHT,
        
        X, H, D, TILE_TOP_LEFT,
        W, H, D, TILE_TOP_RIGHT,
        W, B, D, TILE_BOTTOM_RIGHT,
        
        // Include bottom face for slidingdoors
        W, B, D, DOOR_BOTTOM_RIGHT,//DOOR_TOP_RIGHT,    // far bottom right
        X, B, D, DOOR_TOP_RIGHT,     // far bottom left
        X, B, Y, DOOR_TOP_LEFT,  // near bottom left
        
        W, B, D, DOOR_BOTTOM_RIGHT,    // far bottom right
        W, B, Y, DOOR_BOTTOM_LEFT, // near bottom right
        X, B, Y, DOOR_TOP_LEFT  // near bottom left
    };
    
    this->wallsVao = GraphicsManager::generateVao(cube, sizeof(cube));
}

void Level::uploadHorizontalPlane(float h, GLuint* which) {
    float texw = 1.0f;
    
    float planeH = 1.0f;
    float planeW = 1.0f;
    float O = 0.0f;
    
    float plane[] = {
        O,      h, O,       0.0f, 0.0f,
        O,      h, planeH,  0.0f, texw,
        planeW, h, planeH,  texw, texw,
        
        planeW, h, planeH,  texw, texw,
        planeW, h, O,       texw, 0.0f,
        O,      h, O,       0.0f, 0.0f
    };
    *which = GraphicsManager::generateVao(plane, sizeof(plane));
}

void Level::uploadCeiling() {
    uploadHorizontalPlane(1.0f, &(this->ceilingVao));
}

void Level::uploadFloor() {
    uploadHorizontalPlane(0.0f, &(this->floorVao));
}

void Level::loadWalls() {
    for(int i = 0; i < width * height; i++){
        auto offset = i * sizeof(uint64_t) * 2;
        walls[i].y = (i/width);
        walls[i].x = i%width;
        auto tex = *((uint16_t*)(wallsLocation + offset));;
        walls[i].wallTexture = tex;
        walls[i].ceilingTexture = *((uint16_t*)(wallsLocation + offset + 2));
        if(walls[i].ceilingTexture == 0 && i != 0)
            walls[i].ceilingTexture = walls[0].ceilingTexture;
        walls[i].floorTexture = *((uint16_t*)(wallsLocation + offset + 4));
        if(walls[i].floorTexture == 0 && i != 0)
            walls[i].floorTexture = walls[0].floorTexture;
        uint16_t bitMask = *((uint16_t*)(wallsLocation + offset + 6));
        walls[i].zone = ((bitMask & 0xff00) >> 8);
        walls[i].isDoor = (bitMask & 1) == 1;
        walls[i].key = (bitMask >> 1) & 0b11;

        if ((tex == 1 || tex == 2) && walls[i].isDoor) { // secret door
            totalSecrets++;
        }
        
        if(TILE_HAS_MESSAGE(tex)){
            walls[i].isSolid = tex == ELEVATOR;
            uint64_t strOff = *((uint64_t*)(wallsLocation + offset + 8));
            if(strOff != 0) {
                auto strptr = (fileBuffer + strOff);
                auto len = strnlen((const char*)strptr, 1024);
                walls[i].message = (char*)calloc(len + 1, sizeof(char));
                if(walls[i].message != NULL)
                    strncpy(walls[i].message, (const char*)strptr, len);
            }
        }else{
            walls[i].isSolid = (tex != SWITCH_ON && tex != SWITCH_OFF);
            walls[i].message = NULL;
            if (tex == SWITCH_OFF) {
                if (switchesOff == -1)
                    switchesOff = 0;
                switchesOff++;
            }
        }
        walls[i].tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void Level::loadEntities() {
    uint16_t totalEnts = *entitiesLocation;
    entitiesLocation += 2;
    for(int i = 0; i < totalEnts; i++){
        auto offset = i * sizeof(uint16_t) * 3;
        auto x = *((uint16_t*)(entitiesLocation + offset) + 1);
        auto y = *((uint16_t*)(entitiesLocation + offset) + 2);
        auto entNum = *((uint16_t*)(entitiesLocation + offset));
        entities.push_back(createEntity(entNum, x, y));
    }
}

Entity* Level::createEntity(uint16_t entNum, int x, int y) {
    auto start = glm::vec3((float)x + 0.5f, 0.0f, (float)y + 0.5f);
    switch(entNum){
        case RED_KEY:
            totalItems++;
            return new RedKey(start);
        case BLUE_KEY:
            totalItems++;
            return new BlueKey(start);
        case YELLOW_KEY:
            totalItems++;
            return new YellowKey(start);
        case ZOMBIE:
            totalEnemies++;
            return new Zombie(start);
        case AMMO:
            totalItems++;
            return new Ammo(start);
        case RIFLEAMMO:
            totalItems++;
            return new RifleAmmo(start);
        case PISTOL:
            totalItems++;
            return new Pistol(start);
        case RIFLE:
            totalItems++;
            return new Rifle(start);
        case SYRINGE:
        case BIGSYRINGE:
            return new Syringe(start, entNum & 1);
        case COLLEEN:
            colleen = new Colleen(start);
            return colleen;
        case BEACON:
            return new Beacon(start);
        case THROWER:
            totalEnemies++;
            return new Thrower(start);
    }
    return new Entity(
        glm::vec3((float)x + 0.5f, 0, (float)y + 0.5f),
        entNum,
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.05f, 0.05f)
    );
}

Wall* Level::tryWall(int x, int y) {
    if (NOT_IN_BOUNDS(x, y)) {
        return NULL;
    }
    return &(walls[COORDS(x, y)]);
}