#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

#include "Platform.h"

Shader::Shader(std::string vertex, std::string fragment){
    std::string vertexShaderCode;
    std::string fragmentShaderCode;
    
    std::ifstream vertexShaderFile;
    std::ifstream fragmentShaderFile;
    
    vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        vertexShaderFile.open(vertex);
        fragmentShaderFile.open(fragment);
        
        std::stringstream vertexStream, fragmentStream;
        
        vertexStream << vertexShaderFile.rdbuf();
        fragmentStream << fragmentShaderFile.rdbuf();
        
        vertexShaderFile.close();
        fragmentShaderFile.close();
        
        vertexShaderCode = vertexStream.str();
        fragmentShaderCode = fragmentStream.str();
    }
    catch(std::ifstream::failure& e) {
        std::cerr << "Couldn't read shader files!" << std::endl;
        std::cerr << e.what() << std::endl;;

#ifdef IS_WIN
        char errmsg[1024];
        strerror_s(errmsg, errno);
        std::cerr << errmsg << std::endl;
#else
        std::cerr << strerror(errno) << std::endl;
#endif
        return;
    }
    const char* vertexCodeCStr = vertexShaderCode.c_str();
    const char* fragmentCodeCStr = fragmentShaderCode.c_str();
    
    unsigned int vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(vertexShader, 1, &vertexCodeCStr, NULL);
    glShaderSource(fragmentShader, 1, &fragmentCodeCStr, NULL);
    
    glCompileShader(vertexShader);
    int error = this->checkCompilationErrors(vertexShader, "Vertex");
    if(error){
        std::cerr << error << std::endl;
        return;
    }
    glCompileShader(fragmentShader);
    error = this->checkCompilationErrors(fragmentShader, "Fragment");
    if(error){
        std::cerr << error << std::endl;
        return;
    }
    
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    error = this->checkCompilationErrors(program, "Program");
    if(error){
        std::cerr << error << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() {
    glUseProgram(program);
}

void Shader::setInt(const std::string &name, int value) {
    glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value);
}

void Shader::setFloat(const std::string &name, float value) {
    glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}


void Shader::setMat4(const std::string &name, const glm::mat4 &matrix) {
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) {
    glUniform3f(glGetUniformLocation(program, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &vec) {
    glUniform4f(glGetUniformLocation(program, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &vec) {
    glUniform2f(glGetUniformLocation(program, name.c_str()), vec.x, vec.y);
}


int Shader::checkCompilationErrors(GLuint shader, std::string type){
    GLint success;
    GLchar infoLog[1024];
    if(type != "Program"){
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << type << " shader compilation failed!" << std::endl << infoLog << std::endl;
            return -4;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "Failed to link program!" << std::endl << infoLog << std::endl;
            return -5;
        }
    }
    return 0;
}
