#include "scene/Scene.h"

Scene::Scene() {
    // Cria um nó invisível na coordenada (0,0,0) que segura tudo
    root = new SceneNode("Root"); 
}

Scene::~Scene() {
    delete root;
}

void Scene::draw(Shader& shader, const glm::mat4& viewMatrix) {
    // Começa a desenhar a partir da raiz, usando a matriz Identidade (neutra) como base
    root->draw(shader, viewMatrix, glm::mat4(1.0f));
}