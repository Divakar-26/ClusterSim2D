#include "game.h"
#include "shader_util.h"
#include "UI.h"
#include <imgui.h>
#include <iostream>
#include <vector>

Game::Game(int W_W, int W_H)
{
    WINDOW_W = W_W;
    WINDOW_H = W_H;
}

Game::~Game()
{
    UI_Shutdown();
    physics.shutdown();
}

bool Game::init(const char* title)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(title, WINDOW_W, WINDOW_H,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
        return false;

    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    UI_Init(window, context);
    renderer.init(WINDOW_W, WINDOW_H);
    physics.init(500, WINDOW_W, WINDOW_H);

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
            isRunning = false;

        if (e.type == SDL_EVENT_MOUSE_WHEEL)
        {
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureMouse)
            {
                if (e.wheel.y > 0)
                    zoom *= 1.1f;
                else if (e.wheel.y < 0)
                    zoom *= 0.9f;
            }
        }

        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_MIDDLE)
        {
            middleMouseDown = true;
            lastMouseX = e.button.x;
            lastMouseY = e.button.y;
        }
        if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_MIDDLE)
            middleMouseDown = false;

        if (e.type == SDL_EVENT_MOUSE_MOTION && middleMouseDown)
        {
            // Pan in world space: divide screen delta by zoom so panning
            // feels consistent at any zoom level.
            float deltaX = (e.motion.x - lastMouseX) / zoom;
            float deltaY = (e.motion.y - lastMouseY) / zoom;
            cameraPos.x -= deltaX;
            cameraPos.y -= deltaY;
            lastMouseX = e.motion.x;
            lastMouseY = e.motion.y;
        }
    }
}

void Game::update()
{
    float currentFrame = SDL_GetTicks() / 1000.0f;
    deltaTime   = currentFrame - lastFrame;
    lastFrame   = currentFrame;

    physics.update(deltaTime);
}

void Game::reinitPhysics(int count)
{
    physics.shutdown();
    physics.init(count, physics.getWindowW(), physics.getWindowH());
}

void Game::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ---- UI frame ----
    UI_BeginFrame();

    // Don't map GPU data during UI rendering — sliders may modify physics state
    Body* uiPtr = nullptr;  // UI doesn't actually need live body data
    UI_RenderDebug(deltaTime, uiPtr, physics.getBodiesCount(),
                   WINDOW_W, WINDOW_H, &physics, &zoom, this);

    // ---- Camera matrix ----
    // Build: first translate by -cameraPos (world units), then scale by zoom.
    // This means cameraPos is always in world-space units — panning and
    // borders live in the same coordinate system.
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
    view = glm::translate(view, glm::vec3(-cameraPos, 0.0f));

    renderer.begin(view);

    // ---- World border ---- drawn in world space, same view as particles
    int   bW  = physics.getWindowW();
    int   bH  = physics.getWindowH();
    float bt  = 3.0f;   // border thickness in world pixels
    glm::vec4 borderColor = {0.5f, 0.5f, 0.5f, 1.0f};

    renderer.drawRect(0,          0,          bW,  bt,  borderColor); // top
    renderer.drawRect(0,          bH - bt,    bW,  bt,  borderColor); // bottom
    renderer.drawRect(0,          0,          bt,  bH,  borderColor); // left
    renderer.drawRect(bW - bt,    0,          bt,  bH,  borderColor); // right

    // ---- Particles ----
    // NOW it's safe to map the GPU buffer, after UI has finished
    Body* ptr = physics.getGPUDataPtr();
    for (int i = 0; i < physics.getBodiesCount(); i++)
    {
        renderer.drawCircle(ptr[i].pos.x, ptr[i].pos.y,
                            ptr[i].radius, {1.0f, 1.0f, 1.0f, 1.0f});
    }
    physics.releaseGPUDataPtr();

    // ---- ImGui ----
    UI_EndFrame();

    SDL_GL_SwapWindow(window);
}