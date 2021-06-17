#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>

#include "glad/glad.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "Shader.hpp"

class GraphicsManager {
public:
    // Singleton stuff
    static int init(std::string& title, const uint16_t width, const uint16_t height);
    static std::unique_ptr<GraphicsManager> instance;
    static std::map<std::string, GLuint> textures;
    static std::map<std::string, Shader*> shaders;
    static GLuint loadTex(int imageNum, GLint format);
    static GLuint loadTex(std::string path, GLint format);
    
    static GLuint findTex(int imageNum);
    static GLuint findTex(std::string image);
    static GLuint errorTex;
    
    static float px2scrnX(int x);
    static float px2scrnY(int y);
    static void px2scrn(int x, int y, float* fx, float* fy);
    static float scrnscaleX(int x);
    static float scrnscaleY(int y);
    static GLuint generateVao(float* vertices, size_t size);
    
    static void draw();
    
    // Instance stuff
    GraphicsManager(std::string& title, const uint16_t width, const uint16_t height);
    ~GraphicsManager();
    GLFWwindow* window;

    // methods
    uint16_t getWidth();
    uint16_t getHeight();

    GLuint squareVao;

    int statusCode;
    
private:
    uint16_t width;
    uint16_t height;
    
    int initGlfw(std::string& title);
    int initGL();
    int initShaders();
    int initGLAD();
    
    static uint8_t* loadBitMap(std::string path, uint32_t* width, uint32_t* height, size_t size);
};

void frameBufferSizeCallback(GLFWwindow* window, int width, int height);