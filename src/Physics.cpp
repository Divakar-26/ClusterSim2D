#include "Physics.h"
#include "shader_util.h"
#include <cstdlib>
#include <string>

Physics::Physics() : bodiesCount(0), computeShader(0), ssbo(0), windowW(0), windowH(0)
{
}

Physics::~Physics()
{
    shutdown();
}

void Physics::init(int count, int windowW, int windowH)
{
    bodiesCount = count;
    bodies.resize(bodiesCount);
    this->windowW = windowW;
    this->windowH = windowH;

    initializeBodies();
    setupGPUBuffers();

    computeShader = LoadComputeShader(std::string(PROJECT_ROOT) + "shaders/physics.comp");
}

void Physics::initializeBodies()
{
    for (int i = 0; i < bodiesCount; i++)
    {
        bodies[i].vel.x = (rand() % 200) - 100;  // -100 to 100
        bodies[i].vel.y = (rand() % 100) - 50;   // -50 to 50
        bodies[i].pos = {50 + (rand() % (windowW - 100)), 50 + (rand() % (windowH - 100))};
        
        // All circles
        bodies[i].shapeType = SHAPE_CIRCLE;
        bodies[i].radius = 8 + (rand() % 12);  // radius 8-20
        bodies[i].height = 0.0f;
        bodies[i].pad = 0.0f;
    }
}

void Physics::setupGPUBuffers()
{
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 bodies.size() * sizeof(Body),
                 bodies.data(),
                 GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

void Physics::updateGPUBuffer()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 bodies.size() * sizeof(Body),
                 bodies.data(),
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

void Physics::addCircle(glm::vec2 pos, glm::vec2 vel, float radius)
{
    Body newBody;
    newBody.pos = pos;
    newBody.vel = vel;
    newBody.radius = radius;
    newBody.height = 0.0f;
    newBody.shapeType = SHAPE_CIRCLE;
    newBody.pad = 0.0f;

    bodies.push_back(newBody);
    bodiesCount++;
    updateGPUBuffer();
}

void Physics::addRectangle(glm::vec2 pos, glm::vec2 vel, float width, float height)
{
    Body newBody;
    newBody.pos = pos;
    newBody.vel = vel;
    newBody.radius = width;
    newBody.height = height;
    newBody.shapeType = SHAPE_RECTANGLE;
    newBody.pad = 0.0f;

    bodies.push_back(newBody);
    bodiesCount++;
    updateGPUBuffer();
}

void Physics::update(float deltaTime)
{
    // Run multiple substeps for better stability
    float subDeltaTime = deltaTime / substeps;
    
    for(int step = 0; step < substeps; step++)
    {
        glUseProgram(computeShader);

        GLuint dtLoc = glGetUniformLocation(computeShader, "dt");
        glUniform1f(dtLoc, subDeltaTime);

        GLuint countLoc = glGetUniformLocation(computeShader, "count");
        glUniform1i(countLoc, bodiesCount);

        GLuint windowHLoc = glGetUniformLocation(computeShader, "windowH");
        glUniform1i(windowHLoc, windowH);

        GLuint windowWLoc = glGetUniformLocation(computeShader, "windowW");
        glUniform1i(windowWLoc, windowW);

        int groups = (bodiesCount + 255) / 256;
        glDispatchCompute(groups, 1, 1);
        
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ALL_BARRIER_BITS);
    }
}

Body* Physics::getGPUDataPtr()
{
    glBindBuffer(GL_COPY_READ_BUFFER, ssbo);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    return (Body*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
}

void Physics::releaseGPUDataPtr()
{
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void Physics::shutdown()
{
    if (ssbo != 0)
    {
        glDeleteBuffers(1, &ssbo);
        ssbo = 0;
    }
    if (computeShader != 0)
    {
        glDeleteProgram(computeShader);
        computeShader = 0;
    }
}
