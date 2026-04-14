// include/graphics/Material.h (Exemplo de como deve ficar)
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include "JsonHelpers.h" // Importa a nossa regra de GLM

struct Material {
    glm::vec4 baseColor = glm::vec4(1.0f);
    float specularStrength = 0.5f;
    float shininess = 32.0f;
    float reflectivity = 0.0f;
    
    // NOTA: Texturas em OpenGL (unsigned int) não podem ser salvas porque o ID muda toda vez
    // que o PC liga. Você precisará adicionar std::string diffusePath; etc., futuramente.
    unsigned int diffuseMap = 0; 
    unsigned int specularMap = 0;
    unsigned int normalMap = 0;

    std::string diffusePath = "";
    std::string specularPath = "";
    std::string normalPath = "";

    // Sempre que criar uma propriedade nova no Material, só adicione o nome dela aqui dentro:
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Material, baseColor, specularStrength, shininess, reflectivity, diffusePath, specularPath, normalPath)

    static unsigned int GetDefaultTexture();
};

#endif