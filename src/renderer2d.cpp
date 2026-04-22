#include "renderer2d.h"
#include "shader_util.h"
#include <string>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Renderer2D::init(int screenWidth, int screenHeight)
{
    shader = LoadShader(std::string(PROJECT_ROOT) + "shaders/rect.vert", std::string(PROJECT_ROOT) + "shaders/rect.frag");

    float vertices[] =
    {
        -0.5f, 0.5f,
        0.5f, 0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f
    };

    unsigned int indices[] =
    {
        0,1,2, 
        2,3,0
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    modelLoc = glGetUniformLocation(shader, "model");
    viewLoc  = glGetUniformLocation(shader, "view");
    projectionLoc = glGetUniformLocation(shader, "projection");
    colorLoc = glGetUniformLocation(shader, "color");
    isCircleLoc = glGetUniformLocation(shader, "isCircle");

    projection = glm::ortho(
        0.0f, (float)screenWidth,
        (float)screenHeight, 0.0f,
        -1.0f, 1.0f
    );

    glUseProgram(shader);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer2D::begin(const glm::mat4& view)
{
    viewMatrix = view;

    glUseProgram(shader);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glBindVertexArray(vao);
}

void Renderer2D::drawRect(float x, float y, float w, float h, const glm::vec4& color)
{
    glUniform1i(isCircleLoc, 0);

    glm::mat4 model = glm::mat4(1.0f);


    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(colorLoc, 1, glm::value_ptr(color));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer2D::drawCircle(float x, float y, float r, const glm::vec4& color)
{
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(r * 2.0f, r * 2.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(colorLoc, 1, glm::value_ptr(color));

    glUniform1i(isCircleLoc, 1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}