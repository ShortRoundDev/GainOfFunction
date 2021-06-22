#include <stdio.h>
#include <iostream>

#include "GraphicsManager.hpp"
#include "GameManager.hpp"
#include "SoundManager.hpp"

#include <GLFW/glfw3.h>

#include "windows.h"

int main(int argc, char **argv)
{
    std::string title = "Game";
    
    char curdir[1024];
    GetCurrentDirectoryA(1024, curdir);

    std::cout << "initing graphics...";

    int error = GraphicsManager::init(
        title,
        1024,
        768
    );
    
    if (error) {
        std::cerr << "Failed to launch" << std::endl;
        return error;
    }

    std::cout << " done!" << std::endl;
    
    std::cout << "initing game...";
    GameManager::init(GraphicsManager::instance->window, 1024, 768);
    std::cout << " done!" << std::endl;
    std::cout << "initing sound...";
    SoundManager::init();
    std::cout << " done!" << std::endl;
    
    auto window = GraphicsManager::instance->window;
    std::cout << "starting game..." << std::endl;
    if (argc == 2) {
        GameManager::instance->load(argv[1], false, false);
    }
    while(!glfwWindowShouldClose(window)) {
        GameManager::processInput(window);
        GameManager::update();
        SoundManager::update();
        GameManager::draw();
    }

    std::cout << "Exiting game. Good bye " << std::endl;
    
    return 0;
}
