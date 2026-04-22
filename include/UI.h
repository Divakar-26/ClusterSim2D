#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Physics;
struct Body;

// Initialize ImGui with SDL3 and OpenGL
void UI_Init(SDL_Window* window, SDL_GLContext context);

// Process SDL events for ImGui
void UI_ProcessEvent(SDL_Event* event);

// Begin ImGui frame (call at start of render)
void UI_BeginFrame();
 
// End ImGui frame and render (call at end of render)
void UI_EndFrame();

// Render the debug UI (FPS, physics stats, particle info)
void UI_RenderDebug(float deltaTime, Body* bodies, int bodyCount, int windowW, int windowH, Physics* physics);

// Shutdown ImGui
void UI_Shutdown();
