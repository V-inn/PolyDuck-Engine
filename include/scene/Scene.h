#ifndef SCENE_H
#define SCENE_H

#include "SceneNode.h"

class Scene {
public:
    SceneNode* root;

    Scene();
    ~Scene();

    // Função de atalho para desenhar o mundo inteiro
    void draw(Shader& shader, const glm::mat4& viewMatrix);
};

#endif