#pragma once
#include <memory>
#include <chrono>
#include <map>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "AL/al.h"

#include "Level.hpp"
#include "Camera.hpp"
#include "glm/glm.hpp"
#include "Player.hpp"

#include "Platform.h"

enum GameState {
    MAIN_MENU,
    LOAD_SCREEN,
    PLAYING
};

class GameManager {
public:
    // Singleton stuff
    static int init(GLFWwindow* window, const uint16_t width, const uint16_t height);
    static std::unique_ptr<GameManager> instance;
    static bool shouldQuit;

    std::vector<std::string> availableSaves;

    static GameState state;
    static bool showingMessage;
    static char* message;
    
    static bool keyMap[GLFW_KEY_LAST];
    
    static void update();
    static void draw();

    static void displayMessage(char* message);
    
    static void processInput(GLFWwindow* window);
    
    static void deleteEntity(Entity* entity);
    static void addEntity(Entity* entity);
    static void printPath(std::map<uint32_t, uint32_t> &path, uint32_t start, uint32_t end);
    
    static void drawUi(GLuint texture, float x, float y, float w, float h);
    static void drawAnimatedUi(GLuint texture, float x, float y, float w, float h, float frame, float maxFrame);

    static void save();

#ifdef IS_WIN
    static void saveWin();
#else
    static void saveLinux();
#endif
    
    static std::chrono::high_resolution_clock::time_point lastFrame;

    bool levelChanging = false;
    char* nextLevel = NULL;
    float slideSplash = 1.0f;
    int showDead = 10;
    int showItems = 10;
    int showSecrets = 10;
    GLuint endSplash;
    
    // Instance stuff
    GameManager(GLFWwindow* window, const uint16_t width, const uint16_t height);
    ~GameManager();

    void load(std::string levelName, bool loadSave, bool saving);
    void loadSaveFile(std::string levelname);
    
    Level* currentLevel;
    std::map<std::string, std::string> mapNames;
    
    glm::mat4 projection;
    Camera camera;
    Player player;
    
    bool bright = false;

    bool wireframe = false;
    
    GLuint fontTex;
    GLuint fontVao;
    Shader* fontShader;
    
    //timing
    static int64_t accumulator;
    
    bool dda(float startX, float startY, float endX, float endY, int* x, int* y);
    bool castRayToWall(glm::vec3 start, int wallX, int wallY, glm::vec3* out);
    bool castRayToEntities(glm::vec3 start, glm::vec3 dir, glm::vec3* worldPos, Entity** e);
    bool castRayToSphere(glm::vec3 start, glm::vec3 dir, glm::vec3 sphereStart, glm::vec2 radius, glm::vec3* hit);
    bool bfs(float startX, float startY, float endX, float endY, std::map<uint32_t, uint32_t> &cameFrom);
    
    void print(const char* message, float xPos, float yPos, float size);
    
    //update
    void _update();
    void updateMainMenu();
    void updateLoadMenu();
    void updateGame();
    
    //draw
    void _draw();
    void drawMainMenu();
    void drawLoadMenu();
    void drawGame();

    void _processInput(GLFWwindow* window);

    // Main Menu stuff
    int mainMenuOption = 0;
    int loadMenuOption = 0;

    GLuint mainMenuTexture;
    GLuint loadMenuTexture;
    GLuint menuCursor;
    // Load Menu stuff
    int totalSaves = 0;

    void loadAvailableSaves();
#ifdef IS_WIN
    void loadAvailableSavesWin();
#else
    void loadAvailableSaveLinux();
#endif
    
private:
    void initEventHandlers(GLFWwindow* window);
    void initFont();
};

// Event handlers

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);