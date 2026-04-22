#include "UI.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include "Physics.h"
#include <glm/glm.hpp>
#include <cmath>

void UI_Init(SDL_Window* window, SDL_GLContext context)
{
    // IMGUI SETUP
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    // SDL3 + OpenGL backend init
    ImGui_ImplSDL3_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI_ProcessEvent(SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
}

void UI_BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void UI_EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI_RenderDebug(float deltaTime, Body* bodies, int bodyCount, int windowW, int windowH, Physics* physics)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 550), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Physics Debug");
    
    // FPS
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "FPS: %.2f (%.2f ms)", 1.0f / deltaTime, deltaTime * 1000.0f);
    ImGui::Separator();
    
    // Physics Settings
    if(physics)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Physics Settings");
        int substeps = physics->getSubsteps();
        if(ImGui::SliderInt("Substeps", &substeps, 1, 10))
        {
            physics->setSubsteps(substeps);
        }
        ImGui::SameLine();
        ImGui::HelpMarker("More substeps = more stable but slower");
        ImGui::Separator();
    }
    
    // Physics Stats
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Physics Statistics");
    ImGui::Text("Particle Count: %d", bodyCount);
    ImGui::Text("Window Size: %d x %d", windowW, windowH);
    ImGui::Separator();
    
    // Global physics stats
    float totalKineticEnergy = 0.0f;
    float maxVelocity = 0.0f;
    int circleCount = 0;
    int rectCount = 0;
    
    if(bodies && bodyCount > 0)
    {
        for(int i = 0; i < bodyCount; i++)
        {
            float velMagnitude = glm::length(bodies[i].vel);
            maxVelocity = glm::max(maxVelocity, velMagnitude);
            totalKineticEnergy += 0.5f * velMagnitude * velMagnitude;
            
            if(bodies[i].shapeType == 0) circleCount++;
            else rectCount++;
        }
    }
    
    ImGui::Text("Circles: %d | Rectangles: %d", circleCount, rectCount);
    ImGui::Text("Total Kinetic Energy: %.2f", totalKineticEnergy);
    ImGui::Text("Max Velocity: %.2f", maxVelocity);
    ImGui::Separator();
    
    // Individual particle details
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 1.0f, 1.0f), "Particle Details");
    
    if(ImGui::BeginTable("ParticleTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, 30);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 50);
        ImGui::TableSetupColumn("Pos X", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Pos Y", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Vel", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 60);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        
        if(bodies && bodyCount > 0)
        {
            for(int i = 0; i < bodyCount; i++)
            {
                ImGui::TableNextRow();
                
                ImGui::TableNextColumn();
                ImGui::Text("%d", i);
                
                ImGui::TableNextColumn();
                ImGui::Text("%s", bodies[i].shapeType == 0 ? "Circle" : "Rect");
                
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", bodies[i].pos.x);
                
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", bodies[i].pos.y);
                
                ImGui::TableNextColumn();
                float vel = glm::length(bodies[i].vel);
                ImGui::Text("%.1f", vel);
                
                ImGui::TableNextColumn();
                if(bodies[i].shapeType == 0)
                    ImGui::Text("r:%.1f", bodies[i].radius);
                else
                    ImGui::Text("%.1fx%.1f", bodies[i].radius, bodies[i].height);
            }
        }
        ImGui::EndTable();
    }
    
    ImGui::End();
}

void UI_Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
