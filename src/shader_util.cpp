#include "shader_util.h"
#include <fstream>
#include <sstream>
#include <iostream>

GLuint LoadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vFile(vertexPath), fFile(fragmentPath);
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();
    std::string vCode = vStream.str(), fCode = fStream.str();
    const char* vSource = vCode.c_str(), *fSource = fCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint LoadComputeShader(const std::string& computePath) { 
    std::ifstream cFile(computePath);
    if (!cFile.is_open()) {
        std::cerr << "Failed to open compute shader: " << computePath << std::endl;
        return 0;
    }
    std::stringstream cStream;
    cStream << cFile.rdbuf();
    std::string cCode = cStream.str();
    const char* cSource = cCode.c_str();

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &cSource, nullptr); 
    glCompileShader(computeShader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);

    // Check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(computeShader);

    std::cout << "Compute shader loaded successfully: " << computePath << std::endl;
    return program;
}