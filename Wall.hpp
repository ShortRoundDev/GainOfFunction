#pragma once
#include <stdint.h>

#include "glm/glm.hpp"

// Format
// address  desc        size
// 0        wallTex     1
// 1        ceilingTex  1
// 2        floorTex    1
// 3        zone        1
// 4        bitarea*    1 
//
//* bitarea:
//  bit #s  desc
//  0       isDoor
//  1,2     keyNum (0 = none)
//  3-8     reserved

class Wall {
public:
    Wall();
    ~Wall();
    uint8_t y;
    uint8_t x;
    uint16_t wallTexture;
    uint16_t ceilingTexture;
    uint16_t floorTexture;
    uint8_t zone;
    char* message;

    glm::vec4 tint;
    
    glm::vec3 displacement;
    bool isOpen;
    bool isOpening;
    bool isDoor;
    bool isSolid = true;
    int key;
    
    void open();
    
    void update();
    void draw();

    float openProgress;
    
};