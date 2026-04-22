#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Renderer2D
{
public:
    void init(int screenWidth, int screenHeight);
    void begin(const glm::mat4& view); 
    void drawRect(float x, float y, float w, float h, const glm::vec4& color);
    void drawCircle(float x, float y, float r, const glm::vec4& color);

private:
    GLuint vao, vbo, ebo;
    GLuint shader;

    glm::mat4 projection;
    glm::mat4 viewMatrix;
 
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint colorLoc;

    GLint isCircleLoc;
};