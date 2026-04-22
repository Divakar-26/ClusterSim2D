#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer2d.h"
#include "UI.h"
#include "Physics.h"

class Game
{

public:
    Game(int W_W, int W_H);
    ~Game();

    bool init(const char *title);
    void handleEvent();
    void update();
    void render();
    bool running()
    {
        return isRunning;
    } 

private:
    SDL_Window *window;
    SDL_GLContext context;
    int WINDOW_W, WINDOW_H;

    bool isRunning;

    glm::vec2 cameraPos = {0.0f, 0.0f};
    float zoom = 1.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Renderer2D renderer;
    Physics physics;
};

#endif