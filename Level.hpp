#pragma once

#include <string>
#include <stdint.h>
#include <vector>

#include "glad/glad.h"

#include "Wall.hpp"
#include "Shader.hpp"
#include "Entity.hpp"

#include "AL/al.h"

class Level{
public:
    Level(std::string path);
    ~Level();

    std::string levelName;
    
    uint16_t numberOfTextures;
    uint8_t width;
    uint8_t height;
    Wall* walls;
    uint8_t* wallsLocation;
    uint8_t* entitiesLocation;
    
    ALuint ambience = 0;

    int totalItems = 0;
    int totalEnemies = 0;

    bool loadedSuccessfully;
    
    std::vector<Entity*> entities;
    std::vector<Entity*> removeEntities;
    std::vector<Entity*> addEntities;
    
    Wall* tryWall(int x, int y);

    GLuint wallsVao;
    GLuint ceilingVao;
    GLuint ceilingTexture;
    
    Shader* wallShader;

    glm::vec3 playerPos;
    glm::vec3 cameraFront;
    
    GLuint floorVao;
    GLuint floorTexture;
    
    uint8_t* loadFile(std::string path);
    
    void draw();
    glm::vec3 drawDoor(int x, int y, glm::vec3 offset);
    glm::vec3 drawRegular(int x, int y, glm::vec3 offset);
    glm::vec3 drawSign(int x, int y, glm::vec3 offset);
    glm::vec3 drawTv(int x, int y, glm::vec3 offset);

    void update();
private:
    void uploadWall();
    void uploadHorizontalPlane(float h, GLuint* which);
    void uploadFloor();
    void uploadCeiling();
    
    void loadWalls();
    void loadEntities();
    
    uint8_t* fileBuffer;
    
    Entity* createEntity(uint16_t entNum, int x, int y);
};