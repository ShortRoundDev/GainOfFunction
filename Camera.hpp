#pragma once

#include "glm/glm.hpp"
#include <stdint.h>

class Camera{
public:
    Camera(const uint16_t width, const uint16_t height);
    ~Camera();
    
    void update();
    
    glm::mat4 view;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    
    float yaw;
    float pitch;
    float lastX;
    float lastY;
    float fov;
};