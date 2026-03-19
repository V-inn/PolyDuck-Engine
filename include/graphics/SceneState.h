#ifndef SCENESTATE_H
#define SCENESTATE_H

#include <glm/glm.hpp>

struct SceneState {
    // Visualização
    bool wireframeMode = false;

    // Transformações do Objeto Selecionado
    glm::vec3 objPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 objRotation = glm::vec3(0.0f, 0.0f, 0.0f); // Em graus
    glm::vec3 objScale    = glm::vec3(1.0f, 1.0f, 1.0f);

    // Iluminação
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos   = glm::vec3(1.2f, 1.0f, 2.0f);
};

#endif