#include "Physics.h"
#include "shader_util.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <iostream>

Physics::Physics() {}
Physics::~Physics() { shutdown(); }

// ---------------------------------------------------------------------------
void Physics::init(int count, int wW, int wH)
{
    bodiesCount = count;
    bodies.resize(bodiesCount);
    windowW = wW;
    windowH = wH;

    // cellSize = diameter of the largest possible body.
    // With radii 8-20, max diameter is 40.  Use 40.
    cellSize   = 40.0f;
    gridWidth  = static_cast<int>(std::ceil(windowW / cellSize)) + 1;
    gridHeight = static_cast<int>(std::ceil(windowH / cellSize)) + 1;
    totalCells = gridWidth * gridHeight;

    initializeBodies();
    setupGPUBuffers();

    // Load shaders
    std::string root = std::string(PROJECT_ROOT);
    integrateShader   = LoadComputeShader(root + "shaders/physics.comp");
    broadphaseShader  = LoadComputeShader(root + "shaders/broadphase.comp");
    narrowphaseShader = LoadComputeShader(root + "shaders/narrowphase.comp");

    // Cache integrate uniform locations
    loc_dt      = glGetUniformLocation(integrateShader, "dt");
    loc_count   = glGetUniformLocation(integrateShader, "count");
    loc_windowH = glGetUniformLocation(integrateShader, "windowH");
    loc_windowW = glGetUniformLocation(integrateShader, "windowW");

    // Cache broadphase uniform locations
    bp_count      = glGetUniformLocation(broadphaseShader, "count");
    bp_cellSize   = glGetUniformLocation(broadphaseShader, "cellSize");
    bp_gridWidth  = glGetUniformLocation(broadphaseShader, "gridWidth");
    bp_gridHeight = glGetUniformLocation(broadphaseShader, "gridHeight");
    bp_maxPerCell = glGetUniformLocation(broadphaseShader, "maxPerCell");

    // Cache narrowphase uniform locations
    np_count      = glGetUniformLocation(narrowphaseShader, "count");
    np_cellSize   = glGetUniformLocation(narrowphaseShader, "cellSize");
    np_gridWidth  = glGetUniformLocation(narrowphaseShader, "gridWidth");
    np_gridHeight = glGetUniformLocation(narrowphaseShader, "gridHeight");
    np_maxPerCell = glGetUniformLocation(narrowphaseShader, "maxPerCell");
}

// ---------------------------------------------------------------------------
void Physics::initializeBodies()
{
    for (int i = 0; i < bodiesCount; i++)
    {
        bodies[i].vel.x    = (rand() % 200) - 100;
        bodies[i].vel.y    = (rand() % 100) - 50;
        bodies[i].pos      = { 50.0f + (rand() % (windowW - 100)),
                               50.0f + (rand() % (windowH - 100)) };
        bodies[i].shapeType = SHAPE_CIRCLE;
        bodies[i].radius   = 8.0f + (rand() % 12);
        bodies[i].height   = 0.0f;
        bodies[i].pad      = 0.0f;
    }
}

// ---------------------------------------------------------------------------
void Physics::setupGPUBuffers()
{
    // Body SSBO  (binding = 0)
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 bodiesCount * sizeof(Body),
                 bodies.data(),
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    rebuildGridBuffers();
}

// ---------------------------------------------------------------------------
void Physics::rebuildGridBuffers()
{
    // Delete old grid buffers if they exist
    if (cellCountsSSBO)  { glDeleteBuffers(1, &cellCountsSSBO);  cellCountsSSBO  = 0; }
    if (cellEntriesSSBO) { glDeleteBuffers(1, &cellEntriesSSBO); cellEntriesSSBO = 0; }

    // Cell counts  (binding = 1)  — one int per cell
    glGenBuffers(1, &cellCountsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellCountsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 totalCells * sizeof(int),
                 nullptr,
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellCountsSSBO);

    // Cell entries (binding = 2)  — maxPerCell ints per cell
    glGenBuffers(1, &cellEntriesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellEntriesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 (size_t)totalCells * maxPerCell * sizeof(int),
                 nullptr,
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cellEntriesSSBO);
}

// ---------------------------------------------------------------------------
void Physics::setWindowDimensions(int w, int h)
{
    windowW = w;
    windowH = h;

    // Recompute grid dimensions and reallocate grid buffers
    gridWidth  = static_cast<int>(std::ceil(windowW / cellSize)) + 1;
    gridHeight = static_cast<int>(std::ceil(windowH / cellSize)) + 1;
    totalCells = gridWidth * gridHeight;
    rebuildGridBuffers();
}

// ---------------------------------------------------------------------------
void Physics::updateGPUBuffer()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 bodies.size() * sizeof(Body),
                 bodies.data(),
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
}

// ---------------------------------------------------------------------------
void Physics::addCircle(glm::vec2 pos, glm::vec2 vel, float radius)
{
    if (bodiesCount > 0)
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        Body* gpuData = (Body*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        if (gpuData)
        {
            for (int i = 0; i < bodiesCount; i++)
                bodies[i] = gpuData[i];
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
    }

    Body b;
    b.pos       = pos;
    b.vel       = vel;
    b.radius    = radius;
    b.height    = 0.0f;
    b.shapeType = SHAPE_CIRCLE;
    b.pad       = 0.0f;
    bodies.push_back(b);
    bodiesCount++;
    updateGPUBuffer();
}

// ---------------------------------------------------------------------------
void Physics::addRectangle(glm::vec2 pos, glm::vec2 vel, float width, float height)
{
    Body b;
    b.pos       = pos;
    b.vel       = vel;
    b.radius    = width;
    b.height    = height;
    b.shapeType = SHAPE_RECTANGLE;
    b.pad       = 0.0f;
    bodies.push_back(b);
    bodiesCount++;
    updateGPUBuffer();
}

// ---------------------------------------------------------------------------
void Physics::update(float deltaTime)
{
    if (bodiesCount == 0) return;

    const float subDt  = deltaTime / static_cast<float>(substeps);
    const int   groups = (bodiesCount + 255) / 256;

    // Pre-bind all three SSBOs — they don't change between substeps
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellCountsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cellEntriesSSBO);

    for (int step = 0; step < substeps; step++)
    {
        // ---- 1. Integrate: gravity + walls ----
        glUseProgram(integrateShader);
        glUniform1f(loc_dt,      subDt);
        glUniform1i(loc_count,   bodiesCount);
        glUniform1i(loc_windowH, windowH);
        glUniform1i(loc_windowW, windowW);
        glDispatchCompute(groups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // ---- 2. Clear cell counts ----
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellCountsSSBO);
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, nullptr);

        // ---- 3. Broadphase: assign bodies to cells ----
        glUseProgram(broadphaseShader);
        glUniform1i(bp_count,      bodiesCount);
        glUniform1f(bp_cellSize,   cellSize);
        glUniform1i(bp_gridWidth,  gridWidth);
        glUniform1i(bp_gridHeight, gridHeight);
        glUniform1i(bp_maxPerCell, maxPerCell);
        glDispatchCompute(groups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // ---- 4. Narrowphase: resolve collisions ----
        glUseProgram(narrowphaseShader);
        glUniform1i(np_count,      bodiesCount);
        glUniform1f(np_cellSize,   cellSize);
        glUniform1i(np_gridWidth,  gridWidth);
        glUniform1i(np_gridHeight, gridHeight);
        glUniform1i(np_maxPerCell, maxPerCell);
        glDispatchCompute(groups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
}

// ---------------------------------------------------------------------------
Body* Physics::getGPUDataPtr()
{
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    return (Body*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
}

void Physics::releaseGPUDataPtr()
{
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// ---------------------------------------------------------------------------
void Physics::shutdown()
{
    if (ssbo)            { glDeleteBuffers(1, &ssbo);            ssbo            = 0; }
    if (cellCountsSSBO)  { glDeleteBuffers(1, &cellCountsSSBO);  cellCountsSSBO  = 0; }
    if (cellEntriesSSBO) { glDeleteBuffers(1, &cellEntriesSSBO); cellEntriesSSBO = 0; }
    if (integrateShader)   { glDeleteProgram(integrateShader);   integrateShader   = 0; }
    if (broadphaseShader)  { glDeleteProgram(broadphaseShader);  broadphaseShader  = 0; }
    if (narrowphaseShader) { glDeleteProgram(narrowphaseShader); narrowphaseShader = 0; }
    bodies.clear();
    bodiesCount = 0;
}