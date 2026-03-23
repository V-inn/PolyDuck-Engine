#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

struct Material {
    // 1. Cores e Propriedades Físicas Globais
    glm::vec4 baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float specularStrength = 0.5f; // Força do reflexo (0.0 a 1.0)
    float shininess = 32.0f;       // Nível de polimento (2 a 256)
    float reflectivity = 0.0f;      // Nível de refletividade (0.0 a 1.0)

    // 2. Mapas de Textura (0 significa que não tem mapa aplicado)
    unsigned int diffuseMap = 0;   // A textura principal (cor)
    unsigned int specularMap = 0;  // Mapa de onde brilha mais
    unsigned int normalMap = 0;    // Mapa de relevo falso para a luz
    unsigned int heightMap = 0;    // Mapa de profundidade (Parallax)

    static unsigned int GetDefaultTexture();
};

#endif