#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

// Shape types for particles
enum ShapeType {
    SHAPE_CIRCLE = 0,
    SHAPE_RECTANGLE = 1 
};

// Body structure for particle physics
struct Body
{
    glm::vec2 pos;
    glm::vec2 vel;
    float radius;      // for circles: radius, for rectangles: width
    float height;      // for rectangles: height (unused for circles)
    int shapeType;     // 0 = circle, 1 = rectangle
    float pad;         // padding for alignment
};

class Physics
{
public:
    Physics();
    ~Physics();

    // Initialize physics system with given body count
    void init(int count, int windowW, int windowH);

    // Add individual circle body
    void addCircle(glm::vec2 pos, glm::vec2 vel, float radius);

    // Add individual rectangle body
    void addRectangle(glm::vec2 pos, glm::vec2 vel, float width, float height);

    // Update physics simulation
    void update(float deltaTime);

    // Set number of physics substeps (default: 4)
    void setSubsteps(int steps) { substeps = steps; }
    int getSubsteps() const { return substeps; }

    // Get access to bodies data
    const std::vector<Body>& getBodies() const { return bodies; }
    int getBodiesCount() const { return bodiesCount; }

    // Get GPU buffer pointer for rendering
    Body* getGPUDataPtr();
    void releaseGPUDataPtr();

    // Get SSBO ID
    GLuint getSSBO() const { return ssbo; }

    // Cleanup
    void shutdown();

private:
    std::vector<Body> bodies;
    int bodiesCount;
    
    GLuint computeShader;
    GLuint ssbo;

    int windowW, windowH;
    int substeps = 100;  // Number of physics substeps per frame

    void initializeBodies();
    void setupGPUBuffers();
    void updateGPUBuffer();
};
