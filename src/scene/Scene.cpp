#include "scene/Scene.h"

Scene::Scene() {
    // Cria um nó invisível na coordenada (0,0,0) que segura tudo
    root = new SceneNode("Root", NodeType::FOLDER);

    environment = new SceneNode("Environment", NodeType::ENVIRONMENT);
    environment->ambientColor = glm::vec3(0.1f, 0.1f, 0.1f); // Luz ambiente padrão
    environment->skyboxTexture = 0; // Começa sem céu
    environment->isDraggable = false; // Não pode arrastar o ambiente
    environment->hasRightclick = false; // Não tem menu de contexto
    
    // Configuração inicial do Sol
    environment->sunDirection = glm::vec3(-0.2f, -1.0f, -0.3f); // Apontando pra baixo e pro lado
    environment->sunColor = glm::vec3(1.0f, 0.95f, 0.8f);       // Um tom levemente amarelado
    environment->sunIntensity = 1.0f;
    
    root->addChild(environment);
}

Scene::~Scene() {
    delete root;
}

void Scene::draw(Shader& shader, const glm::mat4& viewMatrix) {
    // Começa a desenhar a partir da raiz, usando a matriz Identidade (neutra) como base
    root->draw(shader, viewMatrix, glm::mat4(1.0f));
}