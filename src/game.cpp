#include "game.h"
#include "shader_util.h"
#include "UI.h"
#include <iostream>
#include <vector>

Game::Game(int W_W, int W_H)
{
    WINDOW_H = W_H;
    WINDOW_W = W_W;
}

Game::~Game() 
{
    UI_Shutdown();
    physics.shutdown();
}

bool Game::init(const char *title)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(title, WINDOW_W, WINDOW_H, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        return false;
    }

    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    // Initialize UI module
    UI_Init(window, context);

    // renderer
    renderer.init(WINDOW_W, WINDOW_H);

    // Initialize physics system 
    physics.init(10, WINDOW_W, WINDOW_H);
    physics.addCircle({100, 100}, {50, 50}, 100.0f);
    physics.addRectangle({200, 200}, {30, 20}, 15.0f, 10.0f);

    isRunning = true;
    return true;
}

void Game::handleEvent()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        UI_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT)
        {
            isRunning = false;
        }
        if (e.type == SDL_EVENT_MOUSE_WHEEL)
        {
            if (e.wheel.y > 0)
                zoom *= 1.1f;
            else if (e.wheel.y < 0)
                zoom *= 0.9f;
        }
    }
}

void Game::update()
{
    float currentFrame = SDL_GetTicks() / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    const bool *state = SDL_GetKeyboardState(NULL);

    float speed = 2.0f * deltaTime / zoom;

    if (state[SDL_SCANCODE_UP])
        cameraPos.y += speed;

    if (state[SDL_SCANCODE_DOWN])
        cameraPos.y -= speed;

    if (state[SDL_SCANCODE_LEFT])
        cameraPos.x -= speed;

    if (state[SDL_SCANCODE_RIGHT])
        cameraPos.x += speed;

    // Update physics simulation
    physics.update(deltaTime);
}

void Game::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Begin UI frame
    UI_BeginFrame();
    UI_RenderDebug(deltaTime);

    // CAMERA
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(-cameraPos, 0.0f));
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));

    renderer.begin(view);

    // Get physics bodies from GPU and render them
    Body *ptr = physics.getGPUDataPtr();

    for (int i = 0; i < physics.getBodiesCount(); i++)
    {
        if (ptr[i].shapeType == SHAPE_CIRCLE)
        {
            renderer.drawCircle(
                ptr[i].pos.x,
                ptr[i].pos.y,
                ptr[i].radius,
                {1, 1, 1, 1});
        }
        else if (ptr[i].shapeType == SHAPE_RECTANGLE)
        {
            renderer.drawRect(
                ptr[i].pos.x,
                ptr[i].pos.y,
                ptr[i].radius,
                ptr[i].height,
                {1, 1, 1, 1});
        }
    }


    physics.releaseGPUDataPtr();

    // End UI frame and render
    UI_EndFrame();

    SDL_GL_SwapWindow(window);
}