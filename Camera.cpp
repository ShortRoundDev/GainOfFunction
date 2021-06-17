#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const uint16_t width, const uint16_t height):
    cameraPos(2.5f, 0.5f, 2.5f),
    cameraFront(0.0f, 0.0f, -1.0f),
    cameraUp(0.0f, 1.0f, 0.0f),
    yaw(-90.f),
    pitch(0.0f),
    lastX(width/2.0f),
    lastY(height/2.0f),
    fov(45.0f){
    
}

Camera::~Camera(){
    
}

void Camera::update() {
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
