#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // O GLAD precisa vir antes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int ID; // O ID do programa de shader na GPU

    // O construtor lê e constrói o shader
    Shader(const char* vertexPath, const char* fragmentPath);
    
    // Ativa o shader para uso
    void use();
    
    // Funções utilitárias para enviar dados (uniforms) para a GPU no futuro
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
};

#endif