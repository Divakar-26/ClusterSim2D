#pragma once
#include <glad/glad.h>
#include <string>

GLuint LoadShader(const std::string& vertexPath, const std::string& fragmentPath);
GLuint LoadComputeShader(const std::string& computePath);  