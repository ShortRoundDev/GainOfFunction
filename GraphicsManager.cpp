#include "GraphicsManager.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Platform.h"

// Static singleton stuff

std::unique_ptr<GraphicsManager> GraphicsManager::instance = nullptr;
std::map<std::string, GLuint> GraphicsManager::textures;
std::map<std::string, Shader*> GraphicsManager::shaders = std::map<std::string, Shader*>();
GLuint GraphicsManager::errorTex = 0;

int GraphicsManager::init(std::string& title, const uint16_t width, const uint16_t height) {
    GraphicsManager::instance = std::make_unique<GraphicsManager>(title, width, height);
    return GraphicsManager::instance->statusCode;
}

GLuint GraphicsManager::loadTex(int imageNum, GLint format) {
    return loadTex("Resources/" + std::to_string(imageNum) + ".png", format);
}

GLuint GraphicsManager::loadTex(std::string path, GLint format){
    auto check = findTex(path);
    if(check != errorTex)
        return check;
    
    int width, height, nrChannels;
    uint8_t* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    //uint8_t* data = loadBitMap(path, &width, &height, format == GL_BGR ? 3 : 4);
    if(data == NULL)
        return 0;

    GLuint texNum = 0;
    glGenTextures(1, &texNum);
    glBindTexture(GL_TEXTURE_2D, texNum);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    
    if(format == GL_BGRA)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_INT_8_8_8_8_REV, data);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(data);
    //stbi_set_flip_vertically_on_load(true);
    /*std::string path = "resources/" + std::to_string(imageNum) + ".bmp";
    unsigned char* data = stbi_load(path.c_str(), &width, &height, , 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else{
        std::cerr << "Couldn't load texture #" << imageNum << std::endl;
    }
    stbi_image_free(data);*/
    
    
    GraphicsManager::textures[path] = texNum;
    return texNum;
}

float GraphicsManager::px2scrnX(int x) {
    return (2.0f * ((float)x/(float)GraphicsManager::instance->getWidth())) - 1.0f;
}
float GraphicsManager::px2scrnY(int y) {
    return -((2.0f * ((float)y/(float)GraphicsManager::instance->getHeight())) - 1.0f);
}
void GraphicsManager::px2scrn(int x, int y, float* fx, float* fy) {
    *fx = px2scrnX(x);
    *fy = px2scrnY(y);
}

float GraphicsManager::scrnscaleX(int x) {
    return ((float)x/(float)GraphicsManager::instance->getWidth());
}

float GraphicsManager::scrnscaleY(int y) {
    return ((float)y/(float)GraphicsManager::instance->getHeight());
}

GLuint GraphicsManager::findTex(int imageNum) {
    return findTex("Resources/" + std::to_string(imageNum) + ".png");
}

GLuint GraphicsManager::findTex(std::string image) {
    auto tex = textures.find(image);
    if(tex == textures.end()){
        return errorTex;
    }
    return tex->second;
}


GLuint GraphicsManager::generateVao(float* vertices, size_t size){
    unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    return vao;
}

bool GraphicsManager::isLeft(glm::vec3 start, glm::vec3 end, glm::vec3 point, float* raw) {
    float dist = ((end.x - start.x) * (point.z - start.z) - (end.z - start.z) * (point.x - start.x));
    if (raw != NULL)
        *raw = dist;
    return dist > 0;
}

void GraphicsManager::draw() {
    glfwSwapBuffers(instance->window);
    glfwPollEvents();
}


// Instance stuff

GraphicsManager::GraphicsManager(std::string& title, const uint16_t width, const uint16_t height) {
    this->width = width;
    this->height = height;
    this->statusCode = initGlfw(title);
    if(this->statusCode){
        return;
    }
    this->statusCode = initGLAD();
    if(this->statusCode){
        return;
    }
    this->statusCode = initGL();
    
    float square[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
         
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
    };
    this->squareVao = GraphicsManager::generateVao(square, sizeof(square));
    errorTex = loadTex("Resources/error.png", GL_BGR);

}

GraphicsManager::~GraphicsManager(){
    
}

uint16_t GraphicsManager::getHeight() {
    return this->height;
}

uint16_t GraphicsManager::getWidth() {
    return this->width;
}

int GraphicsManager::initGlfw(std::string& title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    this->window = glfwCreateWindow(
        getWidth(),
        getHeight(),
        title.c_str(),
        NULL, NULL
    );
    if(this->window == NULL) {
        std::cerr << "Failed to initialize GLFW Window!" << std::endl;
        return -1;
    }
    
    glfwMakeContextCurrent(this->window);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSwapInterval(1);
    return 0;
}

int GraphicsManager::initGL() {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return initShaders();
}
int GraphicsManager::initShaders() {
    shaders["Walls"] = new Shader("WallsVertexShader.glsl", "WallsFragmentShader.glsl");
    shaders["Entities"] = new Shader("EntityVertexShader.glsl", "EntityFragmentShader.glsl");
    shaders["Font"] = new Shader("FontVertexShader.glsl", "FontFragmentShader.glsl");
    shaders["UI"] = new Shader("UIVertexShader.glsl", "UIFragmentShader.glsl");
    shaders["BrushSprite"] = new Shader("BrushSpriteVertexShader.glsl", "BrushSpriteFragmentShader.glsl");
    return 0;
}

int GraphicsManager::initGLAD() {
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -2;
    }
    return 0;
}


uint8_t* GraphicsManager::loadBitMap(std::string path, uint32_t* width, uint32_t* height, size_t size) {

#ifdef IS_WIN
    FILE* fp;
    fopen_s(&fp, path.c_str(), "rb");
#else
    FILE* fp = fopen(path.c_str(), "rb");
#endif

    if(fp == NULL) {
        //std::cerr << "Failed to load bitmap at " << path << std::endl;
        return NULL;
    }
    uint32_t dataOffset = 0;
    fseek(fp, 0xa, SEEK_SET);
    fread(&dataOffset, sizeof(uint32_t), 1, fp);
    
    fseek(fp, 0x12, SEEK_SET);
    fread(width, sizeof(uint32_t), 1, fp);
    
    fseek(fp, 0x16,  SEEK_SET);
    fread(height, sizeof(uint32_t), 1, fp);
    
    uint8_t* data = (uint8_t*) calloc((*width) * (*height) * size, sizeof(uint8_t));
    fseek(fp, dataOffset, SEEK_SET);
    fread(data, sizeof(uint8_t), (*width) * (*height) * size, fp);
    fclose(fp);
    return data;
}


// Framework stuff
void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
