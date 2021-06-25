#include "Entity.hpp"

#include "GraphicsManager.hpp"
#include "GameManager.hpp"
#include "Managers.hpp"

#include "glm/gtx/rotate_vector.hpp"

#include <iostream>

//static stuff

#define TILE_TOP_LEFT       0.0f, 0.0f
#define TILE_TOP_RIGHT      1.0f, 0.0f
#define TILE_BOTTOM_RIGHT   1.0f, 1.0f
#define TILE_BOTTOM_LEFT    0.0f, 1.0f

Shader* Entity::shader = NULL;
GLuint Entity::vao = 0;

void Entity::init(Shader* shader) {
    Entity::shader = shader;
    uploadVertices();
}

void Entity::uploadVertices(){
    float plane[] = {
        -0.5f,  1.0f, 0.0f, TILE_TOP_LEFT,
         0.5f,  1.0f, 0.0f, TILE_TOP_RIGHT,
        -0.5f,  0.0f, 0.0f, TILE_BOTTOM_LEFT,
        
         0.5f,  1.0f, 0.0f, TILE_TOP_RIGHT,
         0.5f,  0.0f, 0.0f, TILE_BOTTOM_RIGHT,
        -0.5f,  0.0f, 0.0f, TILE_BOTTOM_LEFT
    };
    Entity::vao = GraphicsManager::generateVao(plane, sizeof(plane));
}

Entity::Entity(glm::vec3 position, std::string texture, glm::vec2 scale, glm::vec2 radius, int entityType): animations() {
    this->position = position;
    this->texture = GraphicsManager::loadTex(texture, GL_BGRA);
    this->front = glm::vec3(0.0f, 0.0f, 1.0f);
    this->scale = scale;
    this->radiusX = radius.x;
    this->radiusY = radius.y;
    totalFrames = 1;
    this->entityType = entityType;
    animations["idle"] = { 0, 1, 0 };
}

Entity::Entity(glm::vec3 position, GLuint texture, glm::vec2 scale, glm::vec2 radius) {
    this->position = position;
    this->texture = GraphicsManager::loadTex(texture, GL_BGRA);
    this->front = glm::vec3(0.0f, 0.0f, 1.0f);
    this->scale = scale;
    this->radiusX = radius.x;
    this->radiusY = radius.y;
    entityType = texture;
    totalFrames = 1;
    animations["idle"] = { 0, 1, 0 };
}

Entity::~Entity() {
    
}

void Entity::draw() {
    Shader* _shader = nullptr;
    if(shaderOverride == nullptr){
        _shader = shader;
        _shader->use();
        if (GameManager::instance->bright) {
            _shader->setFloat("minBright", 0.9f);
            _shader->setFloat("maxBright", GameManager::maxBright);
        }
        else {
            _shader->setFloat("minBright", 0.0f);
            _shader->setFloat("maxBright", GameManager::maxBright);
        }
    } else {
        _shader = shaderOverride;
        _shader->use();
        _shader->setVec3("playerPos",   CAMERA.cameraPos);
        _shader->setVec4("tint",        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        _shader->setMat4("view",        CAMERA.view);
        _shader->setMat4("projection",  GameManager::instance->projection);
        if(GameManager::instance->bright){
            _shader->setFloat("minBright", 0.9f);
            _shader->setFloat("maxBright", GameManager::maxBright);
        } else {
            _shader->setFloat("minBright", 0.0f);
            _shader->setFloat("maxBright", GameManager::maxBright);
        }
    }

    AnimationDesc* desc = NULL;
    getCurrentAnimation(&desc);
    int currentFrame = 0;
    if (desc != NULL)
        currentFrame = desc->frameOffset();
    
    _shader->setVec3("offset", position);
    _shader->setVec2("scale", scale);
    _shader->setFloat("frame", currentFrame);
    _shader->setFloat("maxFrame", totalFrames);

    // calculate angle
    int angle = 0;
    if (totalAngles > 1) {
        auto diff = glm::normalize(position - PLAYER.pos);
        auto rotFront = glm::rotate(front, (float)(-M_PI / 3.0f), glm::vec3(0, 1.0f, 0));

        auto perspective = acosf(glm::dot(diff, rotFront)) * (180.0 / 3.141592653589793238463);
        if (GraphicsManager::isLeft(position, position + rotFront, PLAYER.pos, NULL)) {
            perspective *= -1;
        }
        perspective += 180;
        angle = (int)(perspective / 360.0f * totalAngles);
    }
    _shader->setFloat("angle", angle);
    _shader->setFloat("maxAngles", totalAngles);

    
    glBindVertexArray(Entity::vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Entity::update() {
    
}

void Entity::getCurrentAnimation(AnimationDesc** desc) {
    auto animationDesc = animations.find(currentAnimation);
    if (animationDesc == animations.end())
        *desc = NULL;
    *desc = &(animationDesc->second);
}

glm::vec3 Entity::pushWall(glm::vec3 newPos) {
    
    //check if inside wall after newPos
    
    int x = (int)newPos.x,
        y = (int)newPos.z;
        
    float xDiff = 0.0f,
          yDiff = 0.0f;
    
    auto w = TRY_WALL(x, y);
    if (w == NULL)
        return newPos;
    
    if(IN_BOUNDS(x + 1, y)) {
        // Check right wall
        auto nextWall = WALLS[COORDS(x + 1, y)];
        auto distX = (((float)x + 1.0f) - newPos.x);
        if(distX < radiusX && SOLID(nextWall)){
            xDiff = -(radiusX - distX);
        }
    }if(IN_BOUNDS(x - 1, y)){
        auto nextWall = WALLS[COORDS(x - 1, y)];
        auto distX = (newPos.x - (float)x);
        if(distX < radiusX && SOLID(nextWall)){
            xDiff = radiusX - distX;
        }
    }
    
    if(IN_BOUNDS(x, y + 1)) {
        auto nextWall = WALLS[COORDS(x, y + 1)];
        auto distY = ((float)y + 1.0f) - newPos.z;
        if(distY < radiusX && SOLID(nextWall)){
            yDiff = -(radiusX - distY);
        }
    } if(IN_BOUNDS(x, y - 1)){
        auto nextWall = WALLS[COORDS(x, y - 1)];
        auto distY = (newPos.z - (float)y);
        if(distY < radiusX && SOLID(nextWall)){
            yDiff = radiusX - distY;
        }
    }
    
    if(xDiff == 0.0f && yDiff == 0.0f){
        //time to check the corners...
        //lets just push X for simplicity. Always lean left/right I guess
        
        //top right
        if(IN_BOUNDS(x + 1, y + 1)){
            auto nextWall = WALLS[COORDS(x + 1, y + 1)];
            
            auto dist = glm::length(glm::vec3(((float)x + 1), newPos.y, ((float)y + 1)) - newPos);
            if(dist < radiusX && SOLID(nextWall)){
                auto pushBack = glm::normalize(newPos - glm::vec3(x + 1, 0, y + 1)) * ((radiusX) - dist);
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
            if(dist < radiusX && SOLID(nextWall)){
                auto pushBack = glm::normalize(glm::vec3(newPos - glm::vec3(x + 1, 0, y))) * ((radiusX) - dist);
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

            if(dist < radiusX && SOLID(nextWall)){
                auto pushBack = glm::normalize(glm::vec3(newPos - glm::vec3(x, 0, y))) * ((radiusX) - dist);
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

            
            if(dist < radiusX && SOLID(nextWall)){
                auto pushBack = glm::normalize(glm::vec3(newPos - glm::vec3(x, 0, y + 1))) * ((radiusX) - dist);
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

void Entity::hurt(int damage, glm::vec3 hitPos){
    hurtTimer = 20;
    this->health -= damage;
    if(this->health <= 0){
        this->health = 0;
        die();
    }
}

void Entity::die(){
    (&PLAYER)->killedEnemies++;
    GameManager::deleteEntity(this);
}

bool Entity::findPathToEntity(Entity* entity, std::map<uint32_t, uint32_t>* path, std::queue<glm::vec3>* goals) {
    if (entity == NULL) {
        return false;
    }
    return findPathToSpot(
        (int16_t)entity->position.x, (int16_t)entity->position.z,
        path,
        goals
    );
}

bool Entity::findPathToSpot(int x, int y, std::map<uint32_t, uint32_t>* path, std::queue<glm::vec3>* goals) {
    bool found = GameManager::instance->bfs(
        (int16_t)position.x, (int16_t)position.z,
        x, y,
        *path
    );
    if (found) {
        found = optimizePath(x, y, path, goals);
    }
    return found;
}
bool Entity::optimizePath(int fx, int fy, std::map<uint32_t, uint32_t>* path, std::queue<glm::vec3>* goals) {
    if (goals->empty()) {
        //Start backwards
        glm::vec3 last = position;

        float itX = ((float)fx) + 0.5f;
        float itY = ((float)fy) + 0.5f;
        uint32_t it = PACK_COORDS((int)itX, (int)itY);

        bool quit = (itX == (int)last.x) && (itY == (int)last.z);
        //safety valve
        int counter = 0;
        while (!quit && counter < 512) {
            int _x, _y;
            counter++;
            if (GameManager::instance->dda(
                last.x,
                last.z,
                itX,
                itY,
                &_x, &_y
            )) {
                last = glm::vec3(itX, 0, itY);
                goals->push(last);
                it = PACK_COORDS(fx, fy);
                itX = ((float)fx) + 0.5f;
                itY = ((float)fy) + 0.5f;
            }
            else {
                it = (*path)[it];
                itX = (float)(UNPACK_X(it)) + 0.5f;
                itY = (float)(UNPACK_Y(it)) + 0.5f;
            }
            quit = ((int)itX == (int)last.x) && ((int)itY == (int)last.z);
        }
    }
    int counter = 0;
    if (goals->empty()) { // raycasting failed, just use a regular orthogonal path
        uint32_t goal = (*path)[PACK_COORDS((int16_t)fx, (int16_t)fy)];
        uint32_t start = PACK_COORDS((int16_t)position.x, (int16_t)position.z);
        while (goal != start && counter < 512) {
            auto x = UNPACK_X(goal);
            auto y = UNPACK_Y(goal);
            goals->push(glm::vec3(x, 0, y));
            goal = (*path)[goal];
            counter++;
        }
    }
    if (goals->empty()) { // something really fucked up here. This should never happen. Treat it as a path not existing
        return false;
        std::cerr << "Catastrophic failure" << std::endl;
    }
    return true;
}