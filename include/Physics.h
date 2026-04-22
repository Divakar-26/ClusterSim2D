#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

enum ShapeType {
    SHAPE_CIRCLE    = 0,
    SHAPE_RECTANGLE = 1
};

struct Body
{
    glm::vec2 pos;
    glm::vec2 vel;
    float radius;
    float height;
    int   shapeType;
    float pad;
};

class Physics
{
public:
    Physics();
    ~Physics();

    void init(int count, int windowW, int windowH);

    void addCircle(glm::vec2 pos, glm::vec2 vel, float radius);
    void addRectangle(glm::vec2 pos, glm::vec2 vel, float width, float height);

    void update(float deltaTime);

    void setSubsteps(int steps) { substeps = steps; }
    int  getSubsteps() const    { return substeps; }

    const std::vector<Body>& getBodies() const { return bodies; }
    int getBodiesCount() const { return bodiesCount; }

    Body* getGPUDataPtr();
    void  releaseGPUDataPtr();

    GLuint getSSBO() const { return ssbo; }

    int  getWindowW() const { return windowW; }
    int  getWindowH() const { return windowH; }
    void setWindowDimensions(int w, int h);

    void shutdown();

private:
    std::vector<Body> bodies;
    int bodiesCount = 0;

    // Shader programs
    GLuint integrateShader   = 0;   // gravity + walls + integrate
    GLuint broadphaseShader  = 0;   // assign bodies to grid cells
    GLuint narrowphaseShader = 0;   // resolve collisions per cell

    // GPU buffers
    GLuint ssbo            = 0;   // Body data            binding=0
    GLuint cellCountsSSBO  = 0;   // int per cell         binding=1
    GLuint cellEntriesSSBO = 0;   // body indices         binding=2

    int windowW = 0, windowH = 0;
    int substeps = 4;

    // Spatial grid
    float cellSize   = 0.0f;   // set to 2 * maxRadius
    int   maxPerCell = 16;
    int   gridWidth  = 0;
    int   gridHeight = 0;
    int   totalCells = 0;

    void initializeBodies();
    void setupGPUBuffers();
    void rebuildGridBuffers();
    void updateGPUBuffer();

    // Cached uniform locations (integrate)
    GLint loc_dt      = -1;
    GLint loc_count   = -1;
    GLint loc_windowH = -1;
    GLint loc_windowW = -1;

    // Cached uniform locations (broadphase / narrowphase)
    GLint bp_count      = -1;
    GLint bp_cellSize   = -1;
    GLint bp_gridWidth  = -1;
    GLint bp_gridHeight = -1;
    GLint bp_maxPerCell = -1;

    GLint np_count      = -1;
    GLint np_cellSize   = -1;
    GLint np_gridWidth  = -1;
    GLint np_gridHeight = -1;
    GLint np_maxPerCell = -1;
};